#include "utils/Logger.h"
#include "communication/Server.h"

int main() {

    try
    {
        Server server(2);
        server.run();
    }
    catch (std::exception& e)
    {
        LOG.fatal("main - Server - Error < " + std::string(e.what()) + " >");
        return 1;
    }

    return 0;
}
