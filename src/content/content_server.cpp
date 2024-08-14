#include "content_server.h"

#include "res/database.h"
#include "res/record.h"

#include <json/json_parser.h>

using namespace Web;

namespace Pss::Content {
constexpr auto allowed_versions =
    Http::Version::Http_1_0 | Http::Version::Http_1_1;

constexpr auto allowed_methods = Http::Method::Get | Http::Method::Head |
                                 Http::Method::Put | Http::Method::Delete;

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
                body_obj.emplace("mediaType", to_string(record->media_type));
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
                body_obj.emplace("mediaType", to_string(record->media_type));
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
        case Http::Method::Put: {
            auto record = json_body_to_record(request.body);
            if (record) {
                this->static_content_db.put(request.target, *record);
                send_response(Http::Status::Ok);
            } else {
                send_response(Http::Status::BadRequest);
            }
            break;
        }
        case Http::Method::Delete: {
            bool did_remove = false;

            // Check for optional specific type
            Http::ContentType *const content_type =
                request.get_header<Http::ContentType>();
            if (content_type == nullptr) {
                // Remove all at target
                did_remove = this->static_content_db.remove(request.target);
            } else {
                if (content_type->media_type != Media::Type::Application_Json) {
                    send_response(Http::Status::BadRequest);
                    break;
                }
                auto media_type = json_body_to_type(request.body);
                if (media_type) {
                    // Remove specific type at target
                    did_remove = this->static_content_db.remove(
                        request.target, *media_type);
                } else {
                    send_response(Http::Status::BadRequest);
                    break;
                }
            }
            if (did_remove) {
                send_response(Http::Status::Ok);
            } else {
                send_response(Http::Status::NotFound);
            }
            break;
        }
    }
}

std::optional<Res::Record> Server::json_body_to_record(
    const std::vector<Byte> &body)
{
    std::string_view string_body(body.begin(), body.end());
    Json::Parser parser(string_body);
    Json::Value json;
    if (!parser.parse(json)) {
        return std::nullopt;
    }
    Json::Object *json_obj = json.get_if<Json::Object>();
    if (json_obj == nullptr) {
        return std::nullopt;
    }

    // Media type
    auto type_it = json_obj->find("mediaType");
    if (type_it == json_obj->end()) {
        return std::nullopt;
    }
    Json::String *media_type_str = type_it->second.get_if<Json::String>();
    if (media_type_str == nullptr) {
        return std::nullopt;
    }
    auto media_type = Media::to_type(*media_type_str);
    if (!media_type) {
        return std::nullopt;
    }

    // Path
    auto path_it = json_obj->find("path");
    if (path_it == json_obj->end()) {
        return std::nullopt;
    }
    Json::String *path_str = path_it->second.get_if<Json::String>();
    if (path_str == nullptr) {
        return std::nullopt;
    }
    auto path = std::filesystem::path(*path_str);

    return Res::Record{
        .media_type = *media_type,
        .path = path,
    };
}

std::optional<Media::Type> Server::json_body_to_type(
    const std::vector<Byte> &body)
{
    std::string_view string_body(body.begin(), body.end());
    Json::Parser parser(string_body);
    Json::Value json;
    if (!parser.parse(json)) {
        return std::nullopt;
    }
    Json::Object *json_obj = json.get_if<Json::Object>();
    if (json_obj == nullptr) {
        return std::nullopt;
    }

    // Media type
    auto type_it = json_obj->find("mediaType");
    if (type_it == json_obj->end()) {
        return std::nullopt;
    }
    Json::String *media_type_str = type_it->second.get_if<Json::String>();
    if (media_type_str == nullptr) {
        return std::nullopt;
    }
    auto media_type = Media::to_type(*media_type_str);
    if (!media_type) {
        return std::nullopt;
    }
    return *media_type;
}
}  // namespace Pss::Content