#include "web/web_server.h"

#include <thread>

int main(int argc, char *argv[])
{
    std::thread web_server([]() {
        Pss::Web::Server server("127.0.0.1", 80);
        server.start();
    });

    web_server.join();
}