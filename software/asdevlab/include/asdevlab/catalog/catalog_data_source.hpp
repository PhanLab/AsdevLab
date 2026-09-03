#pragma once

#include "asdevlab/catalog/catalog_object.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace asdevlab {
namespace catalog {

class CatalogDataSource {
public:
    virtual ~CatalogDataSource() = default;
    virtual std::vector<CatalogObject> load() const = 0;
};

class FileCatalogDataSource : public CatalogDataSource {
public:
    explicit FileCatalogDataSource(std::string path) : path_(std::move(path)) {}

    std::vector<CatalogObject> load() const override {
        std::vector<CatalogObject> loaded_objects;
        const std::filesystem::path target_path(path_);

        if (std::filesystem::exists(target_path) && std::filesystem::is_directory(target_path)) {
            std::vector<std::filesystem::path> json_files;
            for (const auto& entry : std::filesystem::recursive_directory_iterator(target_path)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    json_files.push_back(entry.path());
                }
            }
            std::sort(json_files.begin(), json_files.end());
            for (const auto& file : json_files) {
                auto file_objects = loadFromFile(file.string());
                loaded_objects.insert(loaded_objects.end(), file_objects.begin(), file_objects.end());
            }
            return loaded_objects;
        }

        return loadFromFile(path_);
    }

private:
    std::string path_;

    static std::string trim(const std::string& value) {
        const auto begin = value.find_first_not_of(" \t\n\r");
        if (begin == std::string::npos) {
            return {};
        }
        const auto end = value.find_last_not_of(" \t\n\r");
        return value.substr(begin, end - begin + 1);
    }

    static bool stringContainsCaseInsensitive(const std::string& haystack, const std::string& needle) {
        if (needle.empty()) {
            return true;
        }
        auto lower_haystack = haystack;
        auto lower_needle = needle;
        std::transform(lower_haystack.begin(), lower_haystack.end(), lower_haystack.begin(), [](unsigned char c) { return std::tolower(c); });
        std::transform(lower_needle.begin(), lower_needle.end(), lower_needle.begin(), [](unsigned char c) { return std::tolower(c); });
        return lower_haystack.find(lower_needle) != std::string::npos;
    }

    static std::string readFile(const std::string& path) {
        std::ifstream input(path);
        if (!input.is_open()) {
            throw std::runtime_error("Unable to open catalog file: " + path);
        }
        std::ostringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
    }

    enum class JsonValueType {
        Null,
        Number,
        String,
        Bool,
        Array,
        Object
    };

    struct JsonValue {
        JsonValueType type = JsonValueType::Null;
        double number = 0.0;
        bool boolean = false;
        std::string string_value;
        std::vector<JsonValue> array_value;
        std::unordered_map<std::string, JsonValue> object_value;
    };

    class JsonParser {
    public:
        explicit JsonParser(std::string input) : input_(std::move(input)) {}

        JsonValue parse() {
            skipWhitespace();
            JsonValue value = parseValue();
            skipWhitespace();
            if (pos_ != input_.size()) {
                throw std::runtime_error("Unexpected trailing content");
            }
            return value;
        }

    private:
        JsonValue parseValue() {
            skipWhitespace();
            if (pos_ >= input_.size()) {
                throw std::runtime_error("Unexpected end of input");
            }

            const char ch = input_[pos_];
            if (ch == '{') {
                return parseObject();
            }
            if (ch == '[') {
                return parseArray();
            }
            if (ch == '"') {
                JsonValue value;
                value.type = JsonValueType::String;
                value.string_value = parseString();
                return value;
            }
            if (ch == 'n') {
                expectLiteral("null");
                return {};
            }
            if (ch == 't') {
                expectLiteral("true");
                JsonValue value;
                value.type = JsonValueType::Bool;
                value.boolean = true;
                return value;
            }
            if (ch == 'f') {
                expectLiteral("false");
                JsonValue value;
                value.type = JsonValueType::Bool;
                value.boolean = false;
                return value;
            }
            if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch))) {
                JsonValue value;
                value.type = JsonValueType::Number;
                value.number = parseNumber();
                return value;
            }

            throw std::runtime_error("Unsupported JSON token");
        }

        JsonValue parseObject() {
            expect('{');
            JsonValue value;
            value.type = JsonValueType::Object;
            skipWhitespace();
            if (peek('}')) {
                ++pos_;
                return value;
            }

            while (true) {
                skipWhitespace();
                if (pos_ >= input_.size()) {
                    throw std::runtime_error("Unexpected end of object");
                }
                const std::string key = parseString();
                skipWhitespace();
                expect(':');
                value.object_value[key] = parseValue();
                skipWhitespace();
                if (peek('}')) {
                    ++pos_;
                    break;
                }
                expect(',');
            }
            return value;
        }

        JsonValue parseArray() {
            expect('[');
            JsonValue value;
            value.type = JsonValueType::Array;
            skipWhitespace();
            if (peek(']')) {
                ++pos_;
                return value;
            }

            while (true) {
                value.array_value.push_back(parseValue());
                skipWhitespace();
                if (peek(']')) {
                    ++pos_;
                    break;
                }
                expect(',');
            }
            return value;
        }

        std::string parseString() {
            expect('"');
            std::string result;
            while (pos_ < input_.size()) {
                const char ch = input_[pos_++];
                if (ch == '"') {
                    return result;
                }
                if (ch == '\\') {
                    if (pos_ >= input_.size()) {
                        throw std::runtime_error("Unexpected end of escape sequence");
                    }
                    const char escaped = input_[pos_++];
                    switch (escaped) {
                        case '"': result.push_back('"'); break;
                        case '\\': result.push_back('\\'); break;
                        case '/': result.push_back('/'); break;
                        case 'b': result.push_back('\b'); break;
                        case 'f': result.push_back('\f'); break;
                        case 'n': result.push_back('\n'); break;
                        case 'r': result.push_back('\r'); break;
                        case 't': result.push_back('\t'); break;
                        default: result.push_back(escaped); break;
                    }
                } else {
                    result.push_back(ch);
                }
            }
            throw std::runtime_error("Unterminated string");
        }

        double parseNumber() {
            const auto start = pos_;
            if (input_[pos_] == '-') {
                ++pos_;
            }
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                ++pos_;
            }
            if (pos_ < input_.size() && input_[pos_] == '.') {
                ++pos_;
                while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                    ++pos_;
                }
            }
            if (pos_ < input_.size() && (input_[pos_] == 'e' || input_[pos_] == 'E')) {
                ++pos_;
                if (pos_ < input_.size() && (input_[pos_] == '+' || input_[pos_] == '-')) {
                    ++pos_;
                }
                while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                    ++pos_;
                }
            }
            return std::stod(input_.substr(start, pos_ - start));
        }

        void skipWhitespace() {
            while (pos_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[pos_]))) {
                ++pos_;
            }
        }

        bool peek(char expected) const {
            return pos_ < input_.size() && input_[pos_] == expected;
        }

        void expect(char expected) {
            if (!peek(expected)) {
                throw std::runtime_error("Expected JSON token");
            }
            ++pos_;
        }

        void expectLiteral(const std::string& literal) {
            if (input_.compare(pos_, literal.size(), literal) != 0) {
                throw std::runtime_error("Expected JSON literal");
            }
            pos_ += literal.size();
        }

        std::string input_;
        std::size_t pos_ = 0;
    };

    static std::string readStringValue(const JsonValue& value, const std::string& fallback = {}) {
        if (value.type == JsonValueType::String) {
            return value.string_value;
        }
        return fallback;
    }

    static double readNumberValue(const JsonValue& value, double fallback = 0.0) {
        if (value.type == JsonValueType::Number) {
            return value.number;
        }
        if (value.type == JsonValueType::String) {
            try {
                return std::stod(value.string_value);
            } catch (...) {
                return fallback;
            }
        }
        return fallback;
    }

    static std::vector<std::string> readStringArray(const JsonValue& value) {
        std::vector<std::string> result;
        if (value.type != JsonValueType::Array) {
            return result;
        }
        for (const auto& item : value.array_value) {
            if (item.type == JsonValueType::String) {
                result.push_back(item.string_value);
            }
        }
        return result;
    }

    static std::string readStringField(const std::unordered_map<std::string, JsonValue>& fields, const std::string& key, const std::string& fallback = {}) {
        const auto it = fields.find(key);
        if (it == fields.end()) {
            return fallback;
        }
        return readStringValue(it->second, fallback);
    }

    static double readDoubleField(const std::unordered_map<std::string, JsonValue>& fields, const std::string& key, double fallback = 0.0) {
        const auto it = fields.find(key);
        if (it == fields.end()) {
            return fallback;
        }
        return readNumberValue(it->second, fallback);
    }

    static void appendCatalogObject(const JsonValue& item, std::vector<CatalogObject>& objects) {
        if (item.type != JsonValueType::Object) {
            return;
        }

        const auto& fields = item.object_value;
        CatalogObject current;
        current.id = readStringField(fields, "id");
        current.name = readStringField(fields, "name");
        current.display_name = readStringField(fields, "display_name");
        current.type = readStringField(fields, "type");
        current.provider = readStringField(fields, "provider");

        const auto magnitude = readDoubleField(fields, "magnitude");
        current.magnitude = magnitude != 0.0 ? magnitude : readDoubleField(fields, "mag");
        current.ra = readDoubleField(fields, "ra");
        current.dec = readDoubleField(fields, "dec");
        current.epoch = readStringField(fields, "epoch");
        current.coordinate_source = readStringField(fields, "coordinate_source");
        current.ephemeris_id = readStringField(fields, "ephemeris_id");
        current.messier = readStringField(fields, "messier");
        current.ngc = readStringField(fields, "ngc");
        current.ic = readStringField(fields, "ic");
        current.alias = readStringArray(fields.count("alias") ? fields.at("alias") : JsonValue{});
        current.constellation = readStringField(fields, "constellation");
        current.fun_fact = readStringField(fields, "fun_fact");

        if (current.id.empty() && !current.name.empty()) {
            current.id = current.name;
        }

        current.normalize();
        objects.push_back(current);
    }

    static bool parseCatalogJson(const std::string& content, std::vector<CatalogObject>& objects) {
        JsonParser parser(content);
        const JsonValue root = parser.parse();

        if (root.type == JsonValueType::Array) {
            for (const auto& item : root.array_value) {
                appendCatalogObject(item, objects);
            }
            return true;
        }

        if (root.type == JsonValueType::Object) {
            if (root.object_value.count("objects") && root.object_value.at("objects").type == JsonValueType::Array) {
                for (const auto& item : root.object_value.at("objects").array_value) {
                    appendCatalogObject(item, objects);
                }
                return true;
            }
            appendCatalogObject(root, objects);
            return true;
        }

        return false;
    }

    static std::vector<CatalogObject> loadFromFile(const std::string& path) {
        const std::string content = readFile(path);
        std::vector<CatalogObject> objects;
        if (!parseCatalogJson(content, objects)) {
            throw std::runtime_error("Unsupported catalog payload: " + path);
        }
        return objects;
    }
};

} // namespace catalog
} // namespace asdevlab
