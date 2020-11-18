//
// Created by alessandro on 02/11/20.
//

#include "RequestHandler.h"
#include "../models/FileChunk.h"
#include "../utils/Logger.h"
#include "../utils/Constants.h"
#include <sstream>
#include <string>
#include <filesystem>
#include "../models/FileConfig.h"
#include <boost/iostreams/device/mapped_file.hpp>
#include "../models/FileChunk.h"
#include <boost/thread/thread.hpp>
#include "../models/FileWriter.h"

namespace fs = std::filesystem;
namespace asio = boost::asio;

RequestHandler::RequestHandler(){}

void RequestHandler::handleRequest(Request& request, Response& response)
{
    LOG.info("RequestHandler::handleRequest - Request = " + request.to_string());

    if(!authenticateClient(request, response))
        return;

    if(boost::equals(request.getUri(), Services::CHECKSUM_FILE))
    {
        checksumFile(request, response);
    }
    else if(boost::equals(request.getUri(), Services::TRANSFER_DIRECTORY))
    {
        transferDirectory(request, response);
    }
    else if(boost::equals(request.getUri(), Services::TRANSFER_FILE))
    {
        transferFile(request, response);
    }
    else if(boost::equals(request.getUri(), Services::DELETE_RESOURCE))
    {
        deleteResource(request, response);
    }
    else
    {
        response = Response::stockResponse(StockResponse::not_found);
        LOG.error("RequestHandler::handleRequest - Response = < " + std::to_string(response.status) + " - Not valid Uri >");
    }
}

void RequestHandler::transferDirectory(Request& request, Response& response)
{
    auto fileChunk = Deserializer::deserialize<FileChunk>(request.getBody());
    if(fileChunk.isDirectory())
    {
        LOG.debug("RequestHandler::transferDirectory - Directory = " + fileChunk.getRelativePath() + " - Chunk = " +
                 std::to_string(fileChunk.getChunkNumber()));
        try
        {
            fs::path path(outputDirPath + fileChunk.getRelativePath());

            if (!fs::exists(path))
                fs::create_directory(path);

            response = Response::stockResponse(StockResponse::ok);
            LOG.info("RequestHandler::transferDirectory - Response = < " + std::to_string(response.status) + " >");
        }
        catch(std::exception& e)
        {
            response = Response::stockResponse(StockResponse::internal_server_error);
            LOG.error("RequestHandler::transferDirectory - Response = < " + std::to_string(response.status) + " - " + e.what() + " >");
        }
    }
    else
    {
        response = Response::stockResponse(StockResponse::bad_request);
        LOG.error("RequestHandler::transferDirectory - Response = < " + std::to_string(response.status) + " - Not a directory >");
    }
}

void RequestHandler::transferFile(Request &request, Response &response)
{
    auto fileChunk = Deserializer::deserialize<FileChunk>(request.getBody());

    if(!fileChunk.isDirectory())
    {
        LOG.debug("RequestHandler::transferFile - File = " + fileChunk.getRelativePath() + " - Chunk = " +
                 std::to_string(fileChunk.getChunkNumber()));

        try
        {
            fileWriter.write(outputDirPath + fileChunk.getRelativePath(), fileChunk);

            response = Response::stockResponse(StockResponse::ok);
            LOG.info("RequestHandler::transferFile - Response = < " + std::to_string(response.status) + " >");
        }
        catch(std::exception& e)
        {
            response = Response::stockResponse(StockResponse::internal_server_error);
            LOG.error("RequestHandler::transferFile - Response = < " + std::to_string(response.status) + " - " + e.what() + " >");
        }
    }
    else
    {
        response = Response::stockResponse(StockResponse::bad_request);
        LOG.error("RequestHandler::transferFile - Response = < " + std::to_string(response.status) + " - Not a File >");
    }
}

bool RequestHandler::authenticateClient(Request& request, Response& response){
    LOG.debug("RequestHandler::authenticateClient");
    try
    {
        auto headers = request.getHeaders();
        std::string username;
        std::string inputDirPath;

        for(auto & h: headers){
            if(boost::equals(h.getName(), Config::USERNAME))
            {
                username = h.getValue();
            }
            else if(boost::equals(h.getName(), Config::INPUT_DIR_PATH))
            {
                inputDirPath = h.getValue();
                std::replace(inputDirPath.begin(), inputDirPath.end(), '/', '_');
            }
        }

        if(username.empty() || inputDirPath.empty())
        {
            LOG.error("RequestHandler::authenticateClient - Client unauthorized");
            response = Response::stockResponse(StockResponse::unauthorized);
            return false;
        }

        outputDirPath = fileConfig.getOutputDirPath() + "/" + username + "_" + inputDirPath;

        if(fs::exists(outputDirPath))
        {
            LOG.info("RequestHandler::authenticateClient - Found Client and Path");
            /* credentials present */
        }
        else
        {
            /* credentials not present */
            LOG.debug("RequestHandler::authenticateClient - Client NOT found -> creation of client folder");
            fs::create_directory(outputDirPath);
        }
        return true;
    }
    catch(std::exception& exception)
    {
        LOG.error("RequestHandler::authenticateClient - Unexpected Error < " + std::string(exception.what()) + " >");
        response = Response::stockResponse(StockResponse::internal_server_error);
    }
    return false;
}

void RequestHandler::createDirectories(const std::string& realativePath)
{
    auto directories = StringUtils::split(realativePath, "/");
    std::string newPath(outputDirPath);

    for(int i = 1; i < static_cast<int>(directories.size()) - 1; i++){ // il primo è stringa vuota, l'ultimo è il file
        newPath += "/" + directories[i];

        if (!fs::exists(newPath))
            fs::create_directory(newPath);
    }
}

void RequestHandler::checksumFile(Request& request, Response& response)
{
    auto fileChunk = Deserializer::deserialize<FileChunk>(request.getBody());
    LOG.debug("RequestHandler::checksumFile - File = " + fileChunk.getRelativePath());

    try
    {
        if(!fileChunk.isDirectory() && fileChunk.getChunkNumber() == 0)
        {
            std::string md5File = fs::exists(outputDirPath + fileChunk.getRelativePath()) ?
                    StringUtils::md5FromFile(outputDirPath + fileChunk.getRelativePath()) : "";
            if(boost::equals(md5File, fileChunk.getContent()))
            {
                response = Response::stockResponse(StockResponse::ok);
                LOG.info("RequestHandler::checksumFile - Response = < " + std::to_string(response.status) + " - File < " + fileChunk.getRelativePath() + " > already exists >");
            }
            else
            {
                response = Response::stockResponse(StockResponse::continue_);
                if(!md5File.empty())
                    fs::remove(outputDirPath + fileChunk.getRelativePath());
                LOG.trace("RequestHandler::checksumFile - Response = < " + std::to_string(response.status) + " - File < " + fileChunk.getRelativePath() + " > doesn't exists or has changed >");
            }
        }
        else
        {
            response = Response::stockResponse(StockResponse::bad_request);
            LOG.error("RequestHandler::checksumFile - Response = < " + std::to_string(response.status) + " - Not a File >");
        }
    }
    catch(std::exception& e)
    {
        response = Response::stockResponse(StockResponse::internal_server_error);
        LOG.error("RequestHandler::checksumFile - Response = < " + std::to_string(response.status) + " - " + e.what() + " >");
    }
}

void RequestHandler::deleteResource(Request& request, Response& response){
    auto fileChunk = Deserializer::deserialize<FileChunk>(request.getBody());

    LOG.debug("RequestHandler::deleteResource - Delete Resource = " + fileChunk.getRelativePath() + " - Chunk = " +
              std::to_string(fileChunk.getChunkNumber()));
    try
    {
        fs::path path(outputDirPath + fileChunk.getRelativePath());

        if (fs::exists(path) && fs::is_directory(path))
        {
            fs::remove_all(path); //rimozione ricorsiva della cartella e del suo contenuto
            response = Response::stockResponse(StockResponse::ok);
            LOG.info("RequestHandler::deleteResource - Response = < " + std::to_string(response.status) + " >");
        }
        else if(fs::exists(path))
        {
            fs::remove(path); //rimozione file
            response = Response::stockResponse(StockResponse::ok);
            LOG.info("RequestHandler::deleteResource - Response = < " + std::to_string(response.status) + " >");
        }
        else
        {
            response = Response::stockResponse(StockResponse::bad_request);
            LOG.error("RequestHandler::deleteResource - Response = < " + std::to_string(response.status) + " - not existing file/directory >");
        }
    }
    catch(std::exception& e)
    {
        response = Response::stockResponse(StockResponse::internal_server_error);
        LOG.error("RequestHandler::deleteResource - Response = < " + std::to_string(response.status) + " - " + e.what() + " >");
    }
}

