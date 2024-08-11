#include "web_server.h"

#include "res/database.h"

#include <web/http/http_header_parser.h>
#include <web/log.h>

#include <fstream>

using namespace ::Web;

namespace Pss::Web {
constexpr auto allowed_versions =
    Http::Version::Http_1_0 | Http::Version::Http_1_1;

constexpr auto allowed_methods = Http::Method::Get | Http::Method::Head;

Server::Server(
    std::string_view ip,
    Nat16 port,
    Res::Database &static_content_db)
    : ::Web::Http::Server(ip, port, {allowed_versions, allowed_methods}),
      static_content_db(static_content_db)
{}

void Server::on_request_received(const Http::Request &request)
{
    auto accept_header = request.get_header<Http::AcceptHeader>();
    if (!accept_header) {
        accept_header = {
            .types = {{Media::Type::Any, 1000}},
        };
    }
    auto record = this->static_content_db.get(request.target, *accept_header);
    if (record) {
        Http::Response response(Http::Version::Http_1_1, Http::Status::Ok);

        std::ifstream file(record->path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            send_response(Http::Status::InternalServerError);
            return;
        }
        response.body = std::vector<Byte>(
            std::istreambuf_iterator<Byte>(file),
            std::istreambuf_iterator<Byte>());

        response.headers["content-type"] = Media::to_string(record->type);
        response.headers["content-length"] =
            std::to_string(response.body.size());

        send_response(response);
    } else {
        send_response(Http::Status::NotFound);
    }
}
}  // namespace Pss::Web
