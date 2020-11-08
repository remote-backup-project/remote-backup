//
// Created by alessandro on 07/11/20.
//

#include "./utils/Logger.h"
#include "./communication/Client.h"

int main()
{
    try
    {
        std::string server = "AR-PC";
        std::string port = "9563";

        Client client(server, port);
        client.run();
    }
    catch (std::exception& e)
    {
        LOG.error("main - Client - Error < " + std::string(e.what()) + " >");
        return 1;
    }

    return 0;
}