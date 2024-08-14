#include "content/content_server.h"
#include "res/database.h"
#include "web/web_server.h"

#include <thread>

using namespace Pss;

int main(int argc, char *argv[])
{
    Res::Database static_content_db("db/database.db");

    std::thread web_server_thread([&]() {
        Pss::Web::Server server("127.0.0.1", 80, static_content_db);
        server.start();
    });

    std::thread content_server_thread([&]() {
        Pss::Content::Server server("127.0.0.1", 1969, static_content_db);
        server.start();
    });

    web_server_thread.join();
    content_server_thread.join();
}