#include "res/database.h"
#include "web/web_server.h"

#include <thread>

using namespace Pss;

int main(int argc, char *argv[])
{
    Res::Database static_content_db("db/database.db");

    std::thread web_server([&]() {
        Pss::Web::Server server("127.0.0.1", 80, static_content_db);
        server.start();
    });

    web_server.join();
}