#include "database.h"

#include "endian.h"
#include "web/log.h"

#include <fstream>

using namespace ::Web;

namespace Pss::Res {
Database::Database(const std::filesystem::path &path) : path(path)
{
    read_from_file(path);

    std::jthread file_update_thread([this](std::stop_token stop_token) {
        while (!stop_token.stop_requested()) {
            if (this->dirty) {
                write_to_file();
            }
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        // Final write before stopping
        if (this->dirty) {
            write_to_file();
        }
    });
    this->file_update_thread = std::move(file_update_thread);
}

std::optional<Record> Database::get(
    const Uri::Uri &uri,
    const Http::Accept &accept) const
{
    ReadLock lock(this->resource_mutex);

    auto res_it = this->resources.find(uri.get_path());
    if (res_it == this->resources.end()) {
        return std::nullopt;
    }

    const auto &res_map = res_it->second;
    Nat16 highest_weight = 0;
    Record record;
    auto find_matching_type = [&](bool (*cond)(Media::Type)) -> bool {
        for (const auto &[type, path] : res_map) {
            if (cond(type)) {
                record.media_type = type;
                record.path = path;
                return true;
            }
        }
        return false;
    };
    for (const Http::Accept::Type &weighted_type : accept.types) {
        if (weighted_type.weight <= highest_weight) {
            continue;
        }
        switch (weighted_type.media_type) {
            case Media::Type::Any: {
                highest_weight = weighted_type.weight;
                const auto &[type, path] = *res_map.begin();
                record.media_type = type;
                record.path = path;
                break;
            }
            case Media::Type::Application_Any: {
                if (find_matching_type(Media::is_application)) {
                    highest_weight = weighted_type.weight;
                }
                break;
            }
            case Media::Type::Audio_Any: {
                if (find_matching_type(Media::is_audio)) {
                    highest_weight = weighted_type.weight;
                }
                break;
            }
            case Media::Type::Font_Any: {
                if (find_matching_type(Media::is_font)) {
                    highest_weight = weighted_type.weight;
                }
                break;
            }
            case Media::Type::Haptics_Any: {
                if (find_matching_type(Media::is_haptics)) {
                    highest_weight = weighted_type.weight;
                }
                break;
            }
            case Media::Type::Image_Any: {
                if (find_matching_type(Media::is_image)) {
                    highest_weight = weighted_type.weight;
                }
                break;
            }
            case Media::Type::Message_Any: {
                if (find_matching_type(Media::is_message)) {
                    highest_weight = weighted_type.weight;
                }
                break;
            }
            case Media::Type::Model_Any: {
                if (find_matching_type(Media::is_model)) {
                    highest_weight = weighted_type.weight;
                }
                break;
            }
            case Media::Type::Multipart_Any: {
                if (find_matching_type(Media::is_multipart)) {
                    highest_weight = weighted_type.weight;
                }
                break;
            }
            case Media::Type::Text_Any: {
                if (find_matching_type(Media::is_text)) {
                    highest_weight = weighted_type.weight;
                }
                break;
            }
            case Media::Type::Video_Any: {
                if (find_matching_type(Media::is_video)) {
                    highest_weight = weighted_type.weight;
                }
                break;
            }
            default: {
                if (auto type_it = res_map.find(weighted_type.media_type);
                    type_it != res_map.end()) {
                    highest_weight = weighted_type.weight;
                    record.media_type = weighted_type.media_type;
                    record.path = type_it->second;
                }
                break;
            }
        }
        if (highest_weight == 1000) {
            break;
        }
    }
    if (highest_weight > 0) {
        return record;
    }
    return std::nullopt;
}

void Database::put(const Uri::Uri &uri, const Record &record)
{
    WriteLock lock(this->resource_mutex);
    if (auto res_it = this->resources.find(uri.get_path());
        res_it != this->resources.end()) {
        TypeMap &type_map = res_it->second;

        if (auto type_it = type_map.find(record.media_type);
            type_it != type_map.end()) {
            type_it->second = record.path;
        } else {
            type_map.emplace(record.media_type, record.path);
        }
    } else {
        this->resources.emplace(
            uri, TypeMap({{record.media_type, record.path}}));
    }
    this->dirty = true;
}

bool Database::remove(const Uri::Uri &uri)
{
    WriteLock lock(this->resource_mutex);

    Size removed_count = this->resources.erase(uri.get_path());
    bool removed = removed_count > 0;
    this->dirty = this->dirty || removed;
    return removed;
}

bool Database::remove(const Uri::Uri &uri, Media::Type type)
{
    WriteLock lock(this->resource_mutex);

    bool removed = false;
    if (auto res_it = this->resources.find(uri.get_path());
        res_it != this->resources.end()) {
        Size removed_count = res_it->second.erase(type);
        removed = removed_count > 0;
    }
    this->dirty = this->dirty || removed;
    return removed;
}

bool Database::read_from_file(const std::filesystem::path &path)
{
    std::ifstream file(this->path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR(
            "Failed to open database, file not found: {}.", path.string());
        return false;
    }
    Nat64 uri_count = 0;
    file.read(reinterpret_cast<char *>(&uri_count), sizeof(uri_count));
    if (file.fail()) {
        LOG_ERROR("Failed to read database, reading URI count failed.");
        return false;
    }
    uri_count = from_big_endian(uri_count);

    std::string uri_str, type_str, path_str;
    Nat64 type_count = 0;
    for (Nat64 i = 0; i < uri_count; i++) {
        std::getline(file, uri_str, '\0');
        if (file.fail()) {
            LOG_ERROR("Failed to read database, reading URI string failed.");
            return false;
        }

        file.read(reinterpret_cast<char *>(&type_count), sizeof(type_count));
        type_count = from_big_endian(type_count);
        if (file.fail()) {
            LOG_ERROR("Failed to read database, reading type count failed.");
            return false;
        }

        TypeMap type_map;
        for (Nat64 j = 0; j < type_count; j++) {
            std::getline(file, type_str, '\0');
            if (file.fail()) {
                LOG_ERROR(
                    "Failed to read database, reading type string failed.");
                return false;
            }
            std::getline(file, path_str, '\0');
            if (file.fail()) {
                LOG_ERROR(
                    "Failed to read database, reading path string failed.");
                return false;
            }
            auto type = Media::to_type(type_str);
            if (!type) {
                LOG_ERROR(
                    "Failed to read database, invalid type: {}.", type_str);
                return false;
            }
            type_map.emplace(*type, path_str);
        }
        this->resources.emplace(uri_str, std::move(type_map));
    }
    return true;
}

void Database::write_to_file() const
{
    std::lock_guard file_lock(this->file_mutex);
    ReadLock lock(this->resource_mutex);

    std::ofstream file(this->path, std::ios::out | std::ios::binary);
    Nat64 uri_count =
        Pss::to_big_endian(static_cast<Nat64>(this->resources.size()));
    file.write(reinterpret_cast<Byte *>(&uri_count), sizeof(uri_count));
    for (const auto &[uri, type_map] : this->resources) {
        file.write(uri.c_str(), uri.size());
        file.write("\0", 1);
        Nat64 type_count =
            Pss::to_big_endian(static_cast<Nat64>(type_map.size()));
        file.write(reinterpret_cast<Byte *>(&type_count), sizeof(type_count));
        for (const auto &[type, path] : type_map) {
            std::string_view type_str = Media::to_string(type);
            file.write(type_str.data(), type_str.size());
            file.write("\0", 1);
            std::string path_str = path.string();
            file.write(path_str.c_str(), path_str.size());
            file.write("\0", 1);
        }
    }
    this->dirty = false;
}
}  // namespace Pss::Res