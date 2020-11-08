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


namespace fs = std::filesystem;
namespace asio = boost::asio;

RequestHandler::RequestHandler(){}

//TODO da implementare l'arrivo di request in diverso ordine e scrivere file in ordine spostandosi col puntatore al file

void RequestHandler::handleRequest(Request& request, Response& response)
{
    LOG.info("RequestHandler::handleRequest - Request = " + request.to_string());

    if(!authenticateClient(request, response))
        return;

    if(boost::equals(request.getUri(), Services::TRANSFER_DIRECTORY))
    {
        transferDirectory(request, response);
    }
    else if(boost::equals(request.getUri(), Services::TRANSFER_FILE))
    {
        transferFile(request, response);
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
        LOG.info("RequestHandler::handleRequest - Directory = " + fileChunk.getRelativePath() + " - Chunk = " +
                 std::to_string(fileChunk.getChunkNumber()));
        try
        {
            fs::path path(outputDirPath + fileChunk.getRelativePath());

            if (!fs::exists(path))
                fs::create_directory(path);

            response = Response::stockResponse(StockResponse::ok);
            LOG.info("RequestHandler::handleRequest - Response = < " + std::to_string(response.status) + " >");
        }
        catch(std::exception& e)
        {
            response = Response::stockResponse(StockResponse::internal_server_error);
            LOG.error("RequestHandler::handleRequest - Response = < " + std::to_string(response.status) + " - " + e.what() + " >");
        }
    }
    else
    {
        response = Response::stockResponse(StockResponse::bad_request);
        LOG.error("RequestHandler::handleRequest - Response = < " + std::to_string(response.status) + " - Not a directory >");
    }
}

void RequestHandler::transferFile(Request &request, Response &response)
{
    auto fileChunk = Deserializer::deserialize<FileChunk>(request.getBody());
    if(!fileChunk.isDirectory())
    {
        LOG.info("RequestHandler::handleRequest - File = " + fileChunk.getRelativePath() + " - Chunk = " +
                 std::to_string(fileChunk.getChunkNumber()));

        try
        {
            std::ofstream ofs3;
            if(fileChunk.getChunkNumber() == 1)
            {
                ofs3 = std::ofstream(outputDirPath + fileChunk.getRelativePath(), std::ios::out | std::ios::binary);
            }
            else
            {
                ofs3 = std::ofstream(outputDirPath + fileChunk.getRelativePath(), std::ios::out | std::ios::binary | std::ios::app);
            }

            if(!ofs3.is_open())
                LOG.error("ServerSocket.receiveDirectory - Cannot open/create received file on server");

            ofs3.write(fileChunk.getContent().data(), fileChunk.getContent().size()*sizeof(char));

            response = Response::stockResponse(StockResponse::ok);
            LOG.info("RequestHandler::handleRequest - Response = < " + std::to_string(response.status) + " >");
        }
        catch(std::exception& e)
        {
            response = Response::stockResponse(StockResponse::internal_server_error);
            LOG.error("RequestHandler::handleRequest - Response = < " + std::to_string(response.status) + " - " + e.what() + " >");
        }
    }
    else
    {
        response = Response::stockResponse(StockResponse::bad_request);
        LOG.error("RequestHandler::handleRequest - Response = < " + std::to_string(response.status) + " - Not a File >");
    }
}

bool RequestHandler::authenticateClient(Request& request, Response& response){
    LOG.info("RequestHandler::authenticateClient");
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
            LOG.info("RequestHandler::authenticateClient - Client NOT found -> creation of client folder");
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
