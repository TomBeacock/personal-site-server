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

void Server::on_request_received(Http::Request &request)
{
    Http::Accept &accept_header = request.get_or_create_header<Http::Accept>();
    if (accept_header.types.size() == 0) {
        accept_header.types.push_back({Media::Type::Any, 1000});
    }
    auto record = this->static_content_db.get(request.target, accept_header);
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

        Http::ContentType &content_type =
            response.get_or_create_header<Http::ContentType>();
        content_type.media_type = record->media_type;
        Http::ContentLength &content_length =
            response.get_or_create_header<Http::ContentLength>();
        content_length.length = static_cast<Nat>(response.body.size());

        send_response(response);
    } else {
        send_response(Http::Status::NotFound);
    }
}
}  // namespace Pss::Web
