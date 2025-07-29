#ifndef ADSL_API_HPP
#define ADSL_API_HPP

#include "adsl.hpp"
#include <optional>
#include <functional>

namespace adsl {
/*
*                                                              
*                              dddddddd                         
?                              d::::::d                 lllllll 
*                              d::::::d                 l:::::l 
!                             d::::::d                 l:::::l 
*                              d:::::d                  l:::::l 
?   aaaaaaaaaaaaa      ddddddddd:::::d     ssssssssss    l::::l 
*   a::::::::::::a   dd::::::::::::::d   ss::::::::::s   l::::l 
!   aaaaaaaaa:::::a d::::::::::::::::d ss:::::::::::::s  l::::l 
*            a::::ad:::::::ddddd:::::d s::::::ssss:::::s l::::l  
?     aaaaaaa:::::ad::::::d    d:::::d  s:::::s  ssssss  l::::l  
*   aa::::::::::::ad:::::d     d:::::d    s::::::s       l::::l 
!  a::::aaaa::::::ad:::::d     d:::::d       s::::::s    l::::l 
* a::::a    a:::::ad:::::d     d:::::d ssssss   s:::::s  l::::l 
? a::::a    a:::::ad::::::ddddd::::::dds:::::ssss::::::sl::::::l
* a:::::aaaa::::::a d:::::::::::::::::ds::::::::::::::s l::::::l
!  a::::::::::aa:::a d:::::::::ddd::::d s:::::::::::ss  l::::::l
*   aaaaaaaaaa  aaaa  ddddddddd   ddddd  sssssssssss    llllllll
?                                                              
*            ADSL API - Advanced Data Structure Language
                (c) 2025 Keye_fau > https://github.com/KeyeFauV2/adsl/ -- Under MIT License, see LICENSE file for more details
*
*/


/* ----------------------------- value helpers ----------------------------- */
// Safe cast : returns std::optional<DesiredType> (empty if bad type)
template<typename T>
std::optional<T> getIf(const AdslValue& v)
{
    if (auto p = std::get_if<T>(&v)) return *p;
    return std::nullopt;
}

// Same but with default value supplied
template<typename T>
T getOr(const AdslValue& v, const T& def)
{
    if (auto p = std::get_if<T>(&v)) return *p;
    return def;
}

/* ----------------------------- AdslAPI class ----------------------------- */

class API
{
public:
    bool loadFile (const std::string& path);                // reader
    bool loadString(const std::string& data);               // reader
    bool saveFile (const std::string& path) const;          // writer
    std::string toString() const;                           // writer

    std::vector<AdslEntity*>       entitiesByType (const std::string& type);
    std::vector<const AdslEntity*> entitiesByType (const std::string& type) const;

    std::vector<AdslEntity*>       entitiesByGroup(const std::string& grp);
    std::vector<const AdslEntity*> entitiesByGroup(const std::string& grp) const;

    std::vector<AdslField*>        fieldsByGroup  (const std::string& grp);
    std::vector<const AdslField*>  fieldsByGroup  (const std::string& grp) const;

    AdslGroup&  addGroup (const std::string& name,
                          const std::vector<std::string>& values = {});

    AdslEntity& addEntity(const std::string& type,
                          const std::vector<std::string>& groups = {});

    AdslField&  addField (AdslEntity& ent,
                          const std::string& name,
                          const AdslValue&   value,
                          const std::vector<std::string>& groups = {});

    // Iterate with custom lambda
    void forEachEntity(const std::function<void(AdslEntity&)>& fn);

    // Access underlying DB (const / non-const)
    AdslDatabase&       db()       { return m_db; }
    const AdslDatabase& db() const { return m_db; }

    // Reset everything
    void clear() { m_db.clear(); }

private:
    AdslDatabase m_db;
};

std::string serialize(const AdslDatabase& db);   // same impl used by API::toString()

} // namespace adsl
#endif // ADSL_API_HPP