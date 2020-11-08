//
// Created by alessandro on 02/11/20.
//

#include "utils/Logger.h"
#include "communication/Server.h"

int main() {

    try
    {
        Server server("0.0.0.0", "9563", 1);
        server.run();
    }
    catch (std::exception& e)
    {
        LOG.error("main - Server - Error < " + std::string(e.what()) + " >");
        return 1;
    }

    return 0;
}