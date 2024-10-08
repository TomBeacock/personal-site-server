#pragma once

#include <web/http/http_server.h>

namespace Web::Media {
enum class Type;
}

namespace Pss::Res {
class Database;
struct Record;
}  // namespace Pss::Res

namespace Pss::Content {
class Server : public ::Web::Http::Server {
  public:
    Server(std::string_view ip, Nat16 port, Res::Database &static_content_db);

  protected:
    virtual void on_request_received(::Web::Http::Request &request);

  private:
    std::optional<Res::Record> json_body_to_record(
        const std::vector<Byte> &body);
    std::optional<Web::Media::Type> json_body_to_type(
        const std::vector<Byte> &body);

  private:
    Res::Database &static_content_db;
};
}  // namespace Pss::Content