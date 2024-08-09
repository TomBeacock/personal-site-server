#include "web_server.h"

using namespace ::Web;

namespace Pss::Web {
constexpr auto allowed_versions =
    Http::Version::Http_1_0 | Http::Version::Http_1_1;

constexpr auto allowed_methods = Http::Method::Get | Http::Method::Head;

Server::Server(std::string_view ip, Nat16 port)
    : ::Web::Http::Server(ip, port, {allowed_versions, allowed_methods})
{}

void Server::on_request_received(const Http::Request &request)
{
    send_response(Http::Status::InternalServerError);
}
}  // namespace Pss::Web
