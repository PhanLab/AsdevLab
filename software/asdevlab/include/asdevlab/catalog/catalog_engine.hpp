#pragma once

#include "asdevlab/catalog/catalog_data_source.hpp"
#include "asdevlab/catalog/catalog_object.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

namespace asdevlab {
namespace catalog {

class CatalogEngine {
public:
    CatalogEngine() = default;

    bool loadCatalog(const std::string& path);
    bool loadCatalog(const CatalogDataSource& data_source);
    std::optional<const CatalogObject*> getObject(const std::string& id) const;

    std::vector<CatalogObject> search(const std::string& query) const;
    std::vector<const CatalogObject*> searchByAlias(const std::string& alias) const;
    std::vector<const CatalogObject*> filterByType(const std::string& object_type) const;
    std::vector<const CatalogObject*> filterByConstellation(const std::string& constellation) const;

private:
    std::vector<CatalogObject> objects_;
    std::unordered_map<std::string, const CatalogObject*> object_index_;
    std::unordered_multimap<std::string, const CatalogObject*> alias_index_;

    static std::string normalizeToken(const std::string& token);
    static bool containsToken(const std::string& haystack, const std::string& needle);

    bool loadFromSource(const CatalogDataSource& data_source, std::vector<CatalogObject>& objects);
    void indexObjects(const std::vector<CatalogObject>& objects);
};

} // namespace catalog
} // namespace asdevlab
