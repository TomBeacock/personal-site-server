#pragma once

#include "read_write_mutex.h"
#include "record.h"

#include <web/http/http_header.h>
#include <web/uri/uri.h>

#include <filesystem>
#include <map>
#include <memory>
#include <optional>

namespace Pss::Res {
class Database {
  private:
    using TypeMap = std::map<::Web::Media::Type, std::filesystem::path>;
    using UriMap = std::map<std::string, TypeMap, std::less<>>;

  public:
    Database(const std::filesystem::path &path);
    ~Database();

    std::optional<Record> get(
        const ::Web::Uri::Uri &uri,
        const ::Web::Http::Accept &accept) const;
    void put(const ::Web::Uri::Uri &uri, const Record &record);
    bool remove(const ::Web::Uri::Uri &uri);
    bool remove(const ::Web::Uri::Uri &uri, ::Web::Media::Type type);

  private:
    void write_to_file() const;

  private:
    std::filesystem::path path;
    UriMap resources;
    mutable ReadWriteMutex resource_mutex;
    mutable std::mutex file_mutex;
};
}  // namespace Pss::Res