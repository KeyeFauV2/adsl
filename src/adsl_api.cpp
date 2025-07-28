#include "../include/adsl/adsl_api.hpp"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>   // std::find

using namespace adsl;

/* ******************************************************************** */
/*  --------------------------- API  impl ----------------------------- */
/* ******************************************************************** */

bool API::loadFile(const std::string& path)
{
    return parseAdslFile(path, m_db);
}

bool API::loadString(const std::string& data)
{
    return parseAdslString(data, m_db);
}

bool API::saveFile(const std::string& path) const
{
    std::ofstream out(path);
    if (!out) return false;
    out << serialize(m_db);
    return true;
}

std::string API::toString() const
{
    return serialize(m_db);
}

/* ----------  Queries (non-const versions reconstruits)  ------------- */

std::vector<AdslEntity*> API::entitiesByType(const std::string& type)
{
    std::vector<AdslEntity*> res;
    for (auto& e : m_db.entities)
        if (e.type == type)
            res.push_back(&e);
    return res;
}

std::vector<const AdslEntity*> API::entitiesByType(const std::string& type) const
{
    return m_db.findEntitiesByType(type);
}

std::vector<AdslEntity*> API::entitiesByGroup(const std::string& grp)
{
    std::vector<AdslEntity*> res;
    for (auto& e : m_db.entities)
        if (std::find(e.groups.begin(), e.groups.end(), grp) != e.groups.end())
            res.push_back(&e);
    return res;
}

std::vector<const AdslEntity*> API::entitiesByGroup(const std::string& grp) const
{
    return m_db.findEntitiesByGroup(grp);
}

std::vector<AdslField*> API::fieldsByGroup(const std::string& grp)
{
    std::vector<AdslField*> res;
    for (auto& e : m_db.entities)
        for (auto& f : e.fields)
            if (std::find(f.groups.begin(), f.groups.end(), grp) != f.groups.end())
                res.push_back(&f);
    return res;
}

std::vector<const AdslField*> API::fieldsByGroup(const std::string& grp) const
{
    return m_db.findFieldsByGroup(grp);
}

/* ----------- creation / edition ------------- */

AdslGroup& API::addGroup(const std::string& name,
                         const std::vector<std::string>& values)
{
    AdslGroup g{ name, values };
    auto res = m_db.groups.emplace(name, std::move(g));
    return res.first->second;
}

AdslEntity& API::addEntity(const std::string& type,
                           const std::vector<std::string>& groups)
{
    m_db.entities.emplace_back();
    AdslEntity& e = m_db.entities.back();
    e.type   = type;
    e.groups = groups;
    return e;
}

AdslField& API::addField(AdslEntity& ent,
                         const std::string& name,
                         const AdslValue& value,
                         const std::vector<std::string>& groups)
{
    ent.fields.push_back({ name, value, groups });
    return ent.fields.back();
}

void API::forEachEntity(const std::function<void(AdslEntity&)>& fn)
{
    for (auto& e : m_db.entities)
        fn(e);
}

/* ******************************************************************** */
/*  --------------------------- Writer  ------------------------------- */
/* ******************************************************************** */

// helper: join vector<string> with separator
static std::string join(const std::vector<std::string>& v,
                        const std::string& sep)
{
    std::ostringstream oss;
    for (size_t i = 0; i < v.size(); ++i) {
        oss << v[i];
        if (i + 1 < v.size()) oss << sep;
    }
    return oss.str();
}

std::string adsl::serialize(const AdslDatabase& db)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(2);

    for (const auto& kv : db.groups) {
        const AdslGroup& g = kv.second;
        out << '@' << g.name;
        if (!g.values.empty())
            out << '[' << join(g.values, ",") << ']';
        out << '\n';
    }
    if (!db.groups.empty()) out << '\n';

    for (const auto& e : db.entities) {
        out << '#' << e.type;
        for (auto& g : e.groups) out << " @" << g;
        out << '\n';

        for (const auto& f : e.fields) {
            out << "    - " << f.name << '='
                << adslValueToString(f.value);

            if (!f.groups.empty()) out << ' ';
            for (size_t i = 0; i < f.groups.size(); ++i) {
                out << '@' << f.groups[i];
                if (i + 1 < f.groups.size())
                    out << ' ';
            }

            out << '\n';
        }
        out << '\n';
    }
    return out.str();
}