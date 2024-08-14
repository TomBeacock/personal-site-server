#include "content_server.h"

#include "res/database.h"

#include <json/json_parser.h>

using namespace Web;

namespace Pss::Content {
constexpr auto allowed_versions =
    Http::Version::Http_1_0 | Http::Version::Http_1_1;

constexpr auto allowed_methods = Http::Method::Get | Http::Method::Head |
                                 Http::Method::Post | Http::Method::Put |
                                 Http::Method::Delete;

Server::Server(
    std::string_view ip,
    Nat16 port,
    Res::Database &static_content_db)
    : ::Web::Http::Server(ip, port, {allowed_versions, allowed_methods}),
      static_content_db(static_content_db)
{}

void Server::on_request_received(Http::Request &request)
{
    switch (request.method) {
        case Http::Method::Get: {
            Http::Accept &accept_header =
                request.get_or_create_header<Http::Accept>();
            if (accept_header.types.size() == 0) {
                accept_header.types.push_back({Media::Type::Any, 1000});
            }
            auto record =
                this->static_content_db.get(request.target, accept_header);
            if (record) {
                Http::Response response(
                    Http::Version::Http_1_1, Http::Status::Ok);

                Json::Object body_obj;
                body_obj.emplace("mediaType", to_string(record->type));
                body_obj.emplace("path", record->path.string());
                Json::Value json_body(std::move(body_obj));

                std::string body = json_body.to_string();
                response.body = std::vector<Byte>(body.begin(), body.end());

                Http::ContentType &content_type =
                    response.get_or_create_header<Http::ContentType>();
                content_type.media_type = Media::Type::Application_Json;
                Http::ContentLength &content_length =
                    response.get_or_create_header<Http::ContentLength>();
                content_length.length = static_cast<Nat>(response.body.size());

                send_response(response);
            } else {
                send_response(Http::Status::NotFound);
            }
            break;
        }
        case Http::Method::Head: {
            Http::Accept &accept_header =
                request.get_or_create_header<Http::Accept>();
            if (accept_header.types.size() == 0) {
                accept_header.types.push_back({Media::Type::Any, 1000});
            }
            auto record =
                this->static_content_db.get(request.target, accept_header);
            if (record) {
                Http::Response response(
                    Http::Version::Http_1_1, Http::Status::Ok);

                Json::Object body_obj;
                body_obj.emplace("mediaType", to_string(record->type));
                body_obj.emplace("path", record->path.string());
                Json::Value json_body(std::move(body_obj));

                std::string body = json_body.to_string();

                Http::ContentType &content_type =
                    response.get_or_create_header<Http::ContentType>();
                content_type.media_type = Media::Type::Application_Json;
                Http::ContentLength &content_length =
                    response.get_or_create_header<Http::ContentLength>();
                content_length.length = static_cast<Nat>(body.size());

                send_response(response);
            } else {
                send_response(Http::Status::NotFound);
            }
            break;
        }
        case Http::Method::Post: {
            break;
        }
        case Http::Method::Put: {
            break;
        }
        case Http::Method::Delete: {
            break;
        }
    }
}
}  // namespace Pss::Content