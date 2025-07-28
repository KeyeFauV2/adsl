#ifndef ADSL_HPP
#define ADSL_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <variant>

/**
 * ADSL - Advanced Data Structuring Language
 * Parsable config/data format supporting types: string, int, float, bool, list, comments, grouping, etc.
 * 
 */

// --- Value type management --- //
using AdslValue = std::variant<
    std::string,             // string
    int,                     // integer
    float,                   // float
    bool,                    // boolean
    std::vector<std::string>,// list of strings
    std::vector<int>,        // list of ints
    std::vector<float>,      // list of floats
    std::vector<bool>        // list of bools
>;

// --- ADSL Field (key, value, groups) --- //
struct AdslField {
    std::string name;                          // The field name (e.g. "color", "age")
    AdslValue value;                           // The field value (could be str, int, etc.)
    std::vector<std::string> groups;           // Associated groups (may be empty)
};

// --- Entity : a set of fields with a type --- //
struct AdslEntity {
    std::string type;                          // e.g. "car", "person"
    std::vector<AdslField> fields;             // Fields for this entity
    std::vector<std::string> groups;           // groups attached to the entity itself (optional)
};

// --- Group : possible to have metadata for each group (see '@group[values]') --- //
struct AdslGroup {
    std::string name;
    std::vector<std::string> values;
};

// --- The database : all parsed content --- //
class AdslDatabase {
public:
    // All parsed entities (by type)
    std::vector<AdslEntity> entities;
    
    // All defined groups (by name)
    std::unordered_map<std::string, AdslGroup> groups;

    // API

    // Find all entities of a given type (e.g. #car)
    std::vector<const AdslEntity*> findEntitiesByType(const std::string& type) const;

    // Find all fields with a given group (@group)
    std::vector<const AdslField*> findFieldsByGroup(const std::string& group) const;

    // Find all entities with a group attached to them (not just their fields)
    std::vector<const AdslEntity*> findEntitiesByGroup(const std::string& group) const;

    // Clear DB
    void clear();
};

// --- Parsing ---

// Parse a file into AdslDatabase; returns true on success, false otherwise.
// Throws std::runtime_error if fatal syntax error.
bool parseAdslFile(const std::string& filepath, AdslDatabase& db);

// Parse a memory buffer (data), useful for testing or embedding.
bool parseAdslString(const std::string& data, AdslDatabase& db);

// --- Helpers for value access/type management --- //

enum class AdslValueType {
    String, Int, Float, Bool,
    StringList, IntList, FloatList, BoolList,
    Unknown
};

// Get the type of an AdslValue
AdslValueType getAdslValueType(const AdslValue& v);

// Convert AdslValue to string (for debugging, display, etc.)
std::string adslValueToString(const AdslValue& v);

#endif // ADSL_HPP
