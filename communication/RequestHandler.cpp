//
// Created by alessandro on 02/11/20.
//

#include "RequestHandler.h"
#include "../models/FileChunk.h"
#include "../utils/Logger.h"
#include <sstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;
namespace asio = boost::asio;

RequestHandler::RequestHandler(){}

//TODO da implementare l'arrivo di request in diverso ordine e scrivere file in ordine spostandosi col puntatore al file

void RequestHandler::handleRequest(Request& request, Response& response)
{
    LOG.info("RequestHandler::handleRequest - Request = " + request.to_string());

    if(boost::equals(request.getUri(), "/transfer/directory"))
    {
        transferDirectory(request, response);
    }
    else if(boost::equals(request.getUri(), "/transfer/file"))
    {
        transferFile(request, response);
    }
    else
    {
        response = Response::stockResponse(Response::not_found);
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
            fs::path path("/home/alessandro/CLionProjects/remote-backup/outputDirectory" + fileChunk.getRelativePath());

            if (!fs::exists(path))
                fs::create_directory(path);

            response = Response::stockResponse(Response::ok);
            LOG.info("RequestHandler::handleRequest - Response = < " + std::to_string(response.status) + " >");
        }
        catch(std::exception& e)
        {
            response = Response::stockResponse(Response::internal_server_error);
            LOG.error("RequestHandler::handleRequest - Response = < " + std::to_string(response.status) + " - " + e.what() + " >");
        }
    }
    else
    {
        response = Response::stockResponse(Response::bad_request);
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
                ofs3 = std::ofstream("/home/alessandro/CLionProjects/remote-backup/outputDirectory" + fileChunk.getRelativePath(), std::ios::out | std::ios::binary);
            }
            else
            {
                ofs3 = std::ofstream("/home/alessandro/CLionProjects/remote-backup/outputDirectory" + fileChunk.getRelativePath(), std::ios::out | std::ios::binary | std::ios::app);
            }

            if(!ofs3.is_open())
                LOG.error("ServerSocket.receiveDirectory - Cannot open/create received file on server");

            ofs3.write(fileChunk.getContent().data(), fileChunk.getContent().size()*sizeof(char));

            response = Response::stockResponse(Response::ok);
            LOG.info("RequestHandler::handleRequest - Response = < " + std::to_string(response.status) + " >");
        }
        catch(std::exception& e)
        {
            response = Response::stockResponse(Response::internal_server_error);
            LOG.error("RequestHandler::handleRequest - Response = < " + std::to_string(response.status) + " - " + e.what() + " >");
        }
    }
    else
    {
        response = Response::stockResponse(Response::bad_request);
        LOG.error("RequestHandler::handleRequest - Response = < " + std::to_string(response.status) + " - Not a File >");
    }
}
