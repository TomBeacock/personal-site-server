#pragma once

#include <web/media/media_type.h>

#include <filesystem>

namespace Pss::Res {
struct Record {
    ::Web::Media::Type type = ::Web::Media::Type::Any;
    std::filesystem::path path;
};
}  // namespace Pss::Res