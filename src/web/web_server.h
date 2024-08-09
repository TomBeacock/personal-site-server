#pragma once

#include <web/http/http_server.h>

namespace Pss::Web {
class Server : public ::Web::Http::Server {
  public:
    Server(std::string_view ip, Nat16 port);

  protected:
    virtual void on_request_received(
        const ::Web::Http::Request &request) override;

  private:
};
}  // namespace Pss::Web