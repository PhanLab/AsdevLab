#include "asdevlab/catalog/catalog_engine.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace asdevlab {
namespace catalog {

std::string CatalogEngine::normalizeToken(const std::string& token) {
    std::string normalized;
    normalized.reserve(token.size());
    for (char ch : token) {
        if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '-') {
            normalized.push_back(std::tolower(static_cast<unsigned char>(ch)));
        }
    }
    return normalized;
}

bool CatalogEngine::containsToken(const std::string& haystack, const std::string& needle) {
    const auto normalized_haystack = normalizeToken(haystack);
    const auto normalized_needle = normalizeToken(needle);
    if (normalized_needle.empty()) {
        return true;
    }
    return normalized_haystack.find(normalized_needle) != std::string::npos;
}

bool CatalogEngine::loadCatalog(const std::string& path) {
    try {
        return loadCatalog(FileCatalogDataSource{path});
    } catch (const std::exception& ex) {
        std::cerr << "Catalog load failed: " << ex.what() << "\n";
        return false;
    }
}

bool CatalogEngine::loadCatalog(const CatalogDataSource& data_source) {
    try {
        std::vector<CatalogObject> parsed_objects = data_source.load();
        objects_ = std::move(parsed_objects);
        object_index_.clear();
        alias_index_.clear();
        indexObjects(objects_);
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Catalog load failed: " << ex.what() << "\n";
        return false;
    }
}

std::optional<const CatalogObject*> CatalogEngine::getObject(const std::string& id) const {
    const auto normalized_id = normalizeToken(id);
    auto it = object_index_.find(normalized_id);
    if (it == object_index_.end()) {
        return std::nullopt;
    }
    return it->second;
}

namespace {

std::string normalizeSearchToken(const std::string& token) {
    std::string normalized;
    normalized.reserve(token.size());
    for (char ch : token) {
        if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '-') {
            normalized.push_back(std::tolower(static_cast<unsigned char>(ch)));
        }
    }
    return normalized;
}

int scoreField(const std::string& field, const std::string& normalized_query) {
    const auto normalized_field = normalizeSearchToken(field);
    if (normalized_field.empty()) {
        return 0;
    }
    if (normalized_field == normalized_query) {
        return 500;
    }
    if (normalized_field.rfind(normalized_query, 0) == 0) {
        return 300;
    }
    if (normalized_field.find(normalized_query) != std::string::npos) {
        return 150;
    }
    return 0;
}

} // namespace

std::vector<CatalogObject> CatalogEngine::search(const std::string& keyword) const {
    const std::string normalized = normalizeToken(keyword);
    struct ScoredMatch {
        CatalogObject object;
        int score = 0;
    };

    std::vector<ScoredMatch> matches;
    matches.reserve(objects_.size());

    for (const auto& object : objects_) {
        int score = 0;
        score += scoreField(object.id, normalized);
        score += scoreField(object.name, normalized);
        score += scoreField(object.display_name, normalized);
        score += scoreField(object.type, normalized);
        score += scoreField(object.messier, normalized);
        score += scoreField(object.ngc, normalized);
        score += scoreField(object.ic, normalized);
        if (score == 0) {
            for (const auto& alias : object.alias) {
                score += scoreField(alias, normalized);
                if (score > 0) {
                    break;
                }
            }
        }

        if (score > 0) {
            matches.push_back({object, score});
        }
    }

    std::sort(matches.begin(), matches.end(), [](const ScoredMatch& lhs, const ScoredMatch& rhs) {
        if (lhs.score != rhs.score) {
            return lhs.score > rhs.score;
        }
        if (lhs.object.name != rhs.object.name) {
            return lhs.object.name < rhs.object.name;
        }
        return lhs.object.id < rhs.object.id;
    });

    std::vector<CatalogObject> ordered;
    ordered.reserve(matches.size());
    for (const auto& match : matches) {
        ordered.push_back(match.object);
    }
    return ordered;
}

std::vector<const CatalogObject*> CatalogEngine::searchByAlias(const std::string& alias) const {
    const auto normalized_alias = normalizeToken(alias);
    std::vector<const CatalogObject*> matches;
    auto range = alias_index_.equal_range(normalized_alias);
    for (auto it = range.first; it != range.second; ++it) {
        matches.push_back(it->second);
    }
    return matches;
}

std::vector<const CatalogObject*> CatalogEngine::filterByType(const std::string& object_type) const {
    const auto normalized_type = normalizeToken(object_type);
    std::vector<const CatalogObject*> matches;
    for (const auto& object : objects_) {
        if (normalizeToken(object.type) == normalized_type) {
            matches.push_back(&object);
        }
    }
    return matches;
}

std::vector<const CatalogObject*> CatalogEngine::filterByConstellation(const std::string&) const {
    return {};
}

bool CatalogEngine::loadFromSource(const CatalogDataSource& data_source, std::vector<CatalogObject>& objects) {
    objects = data_source.load();
    return true;
}

void CatalogEngine::indexObjects(const std::vector<CatalogObject>& objects) {
    for (const auto& object : objects) {
        const auto key = normalizeToken(object.id);
        object_index_[key] = &object;

        const auto name_key = normalizeToken(object.name);
        if (!name_key.empty()) {
            object_index_[name_key] = &object;
        }

        const auto display_key = normalizeToken(object.display_name.empty() ? object.name : object.display_name);
        if (!display_key.empty()) {
            object_index_[display_key] = &object;
        }

        for (const auto& alias : object.alias) {
            const auto alias_key = normalizeToken(alias);
            if (!alias_key.empty()) {
                object_index_[alias_key] = &object;
                alias_index_.emplace(alias_key, &object);
            }
        }
    }
}

} // namespace catalog
} // namespace asdevlab
