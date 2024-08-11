#pragma once

#include <web/http/http_server.h>

namespace Pss::Res {
class Database;
}

namespace Pss::Web {
class Server : public ::Web::Http::Server {
  public:
    Server(std::string_view ip, Nat16 port, Res::Database &static_content_db);

  protected:
    virtual void on_request_received(
        const ::Web::Http::Request &request) override;

  private:
    Res::Database &static_content_db;
};
}  // namespace Pss::Web