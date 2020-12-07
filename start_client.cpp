#include "./utils/Logger.h"
#include "./communication/Client.h"
#include "./models/FileWriter.h"

int main()
{
    try
    {
        Client client;
        client.run();
    }
    catch (std::exception& e)
    {
        LOG.fatal("main - Client - Error < " + std::string(e.what()) + " >");
        return 1;
    }

    return 0;
}