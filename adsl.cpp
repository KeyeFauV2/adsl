#include "adsl.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <iomanip>

using namespace std;

/* UTILITIES */

namespace {

    /* trim helpers */
    inline void ltrim(string &s) { s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                       [](unsigned char c){ return !std::isspace(c); })); }
    inline void rtrim(string &s) { s.erase(std::find_if(s.rbegin(), s.rend(),
                                       [](unsigned char c){ return !std::isspace(c); }).base(), s.end()); }
    inline string trimmed(string s){ ltrim(s); rtrim(s); return s; }

    inline bool startsWith(const string& s, const string& prefix)
    {
        return s.size() >= prefix.size() &&
               equal(prefix.begin(), prefix.end(), s.begin());
    }

    /* Remove a // comment outside of quotes */
    string stripComment(string line)
    {
        bool inString = false;
        for (size_t i = 0; i + 1 < line.size(); ++i)
        {
            if (line[i] == '"' && (i == 0 || line[i-1] != '\\'))
                inString = !inString;

            if (!inString && line[i] == '/' && line[i+1] == '/')
            {
                return line.substr(0, i);
            }
        }
        return line;
    }

    bool isInteger(const string& s)
    {
        if (s.empty()) return false;
        size_t i = (s[0]=='+' || s[0]=='-') ? 1 : 0;
        return i < s.size() && all_of(s.begin()+i, s.end(), ::isdigit);
    }

    bool isFloat(const string& s)
    {
        bool dot=false; bool digit=false;
        size_t i = (s[0]=='+'||s[0]=='-')?1:0;
        for(; i<s.size(); ++i){
            if (isdigit(s[i])) digit=true;
            else if (s[i]=='.' && !dot) dot=true;
            else return false;
        }
        return digit && dot;
    }

    /* Forward */
    AdslValue parseValue(const string& raw);

    /* Parse list: assume raw begins with '[' and ends with ']' (already trimmed) */
    AdslValue parseList(const string& raw)
    {
        string inner = trimmed(raw.substr(1, raw.size()-2));   // drop [ ]
        vector<string> items;
        string tmp;
        bool inString=false;
        for(size_t i=0;i<=inner.size();++i){
            char c = (i==inner.size())? ',' : inner[i];
            if (c=='"' && (i==0 || inner[i-1]!='\\')) inString = !inString;
            if (c==',' && !inString){
                items.push_back(trimmed(tmp));
                tmp.clear();
            }else if(i<inner.size()){
                tmp.push_back(c);
            }
        }
        if(items.empty()) throw runtime_error("Empty list not supported");

        /* Decide type from first item */
        const string& first = items.front();
        if (first.size()>0 && first.front()=='"')
        {
            vector<string> out;
            for(auto& it:items){
                if(it.size()<2 || it.front()!='"' || it.back()!='"')
                    throw runtime_error("Mixed or invalid string list");
                out.push_back(it.substr(1,it.size()-2));
            }
            return out;
        }
        else if (first=="true" || first=="false")
        {
            vector<bool> out;
            for(auto& it:items){
                if(it=="true") out.push_back(true);
                else if(it=="false") out.push_back(false);
                else throw runtime_error("Mixed bool list");
            }
            return out;
        }
        else if(isInteger(first))
        {
            vector<int> out;
            for(auto& it:items){
                if(!isInteger(it)) throw runtime_error("Mixed int list");
                out.push_back(stoi(it));
            }
            return out;
        }
        else if(isFloat(first))
        {
            vector<float> out;
            for(auto& it:items){
                if(!isFloat(it)) throw runtime_error("Mixed float list");
                out.push_back(stof(it));
            }
            return out;
        }
        throw runtime_error("Unknown list item type");
    }

    AdslValue parseValue(const string& raw)
    {
        string s = trimmed(raw);
        if(s.empty()) throw runtime_error("missing value");

        if(s.front()=='[' && s.back()==']')            /* list */
            return parseList(s);

        if(s.front()=='"' && s.back()=='"')            /* string */
            return s.substr(1,s.size()-2);

        if(s=="true") return true;
        if(s=="false") return false;

        if(isInteger(s))   return stoi(s);
        if(isFloat(s))     return stof(s);

        throw runtime_error("Unrecognised value: "+s);
    }

    /* Split a line of @groups, collects tokens into vector */
    void extractGroups(const string& part, vector<string>& outGroups)
    {
        string s = part;
        ltrim(s);
        string token;
        for(size_t i=0;i<=s.size();++i){
            char c = (i==s.size())? ' ' : s[i];
            if(isspace(c) || c==','){
                if(!token.empty()){
                    if(token[0]=='@') outGroups.push_back(token.substr(1));
                    token.clear();
                }
            }else{
                token.push_back(c);
            }
        }
    }

} // namespace utils

/*  AdslDatabase impl   */

vector<const AdslEntity*> AdslDatabase::findEntitiesByType(const string& type) const
{
    vector<const AdslEntity*> res;
    for (auto& e : entities)
        if (e.type == type) res.push_back(&e);
    return res;
}

vector<const AdslField*> AdslDatabase::findFieldsByGroup(const string& group) const
{
    vector<const AdslField*> res;
    for (auto& e : entities)
        for (auto& f : e.fields)
            if (std::find(f.groups.begin(), f.groups.end(), group) != f.groups.end())
                res.push_back(&f);
    return res;
}

vector<const AdslEntity*> AdslDatabase::findEntitiesByGroup(const string& group) const
{
    vector<const AdslEntity*> res;
    for (auto& e : entities)
        if (std::find(e.groups.begin(), e.groups.end(), group) != e.groups.end())
            res.push_back(&e);
    return res;
}

void AdslDatabase::clear()
{
    entities.clear();
    groups.clear();
}

/* Helpers */

AdslValueType getAdslValueType(const AdslValue& v)
{
    switch(v.index())
    {
        case 0:  return AdslValueType::String;
        case 1:  return AdslValueType::Int;
        case 2:  return AdslValueType::Float;
        case 3:  return AdslValueType::Bool;
        case 4:  return AdslValueType::StringList;
        case 5:  return AdslValueType::IntList;
        case 6:  return AdslValueType::FloatList;
        case 7:  return AdslValueType::BoolList;
        default: return AdslValueType::Unknown;
    }
}

string adslValueToString(const AdslValue& v)
{
    struct {
        string operator()(const string& s)      const { return "\""+s+"\""; }
        string operator()(int i)                const { return to_string(i); }
        string operator()(float f)              const { ostringstream oss; oss<<f; return oss.str();}
        string operator()(bool b)               const { return b?"true":"false"; }
        string operator()(const vector<string>& vs) const{
            string out="[";
            for(size_t i=0;i<vs.size();++i){ out+="\""+vs[i]+"\""; if(i+1<vs.size())out+=",";}
            return out+"]";
        }
        string operator()(const vector<int>& v) const{
            string out="[";
            for(size_t i=0;i<v.size();++i){ out+=to_string(v[i]); if(i+1<v.size())out+=",";}
            return out+"]";
        }
        string operator()(const vector<float>& v) const{
            string out="[";
            for(size_t i=0;i<v.size();++i){ ostringstream oss; oss<<v[i]; out+=oss.str();
                if(i+1<v.size())out+=",";}
            return out+"]";
        }
        string operator()(const vector<bool>& v) const{
            string out="[";
            for(size_t i=0;i<v.size();++i){ out+=(v[i]?"true":"false"); if(i+1<v.size())out+=",";}
            return out+"]";
        }
    } visitor;
    return std::visit(visitor, v);
}

/*   PARSER    */

static bool parseInternal(istream& in, AdslDatabase& db)
{
    db.clear();
    string line;
    size_t lineno=0;
    AdslEntity* current = nullptr;

    while (std::getline(in, line))
    {
        ++lineno;
        line = stripComment(line);
        line = trimmed(line);
        if(line.empty()) continue;

        /* Group definition */
        if(startsWith(line,"@"))
        {
            size_t brk = line.find_first_of("[ \t");
            string gName = line.substr(1, brk==string::npos? string::npos : brk-1);
            AdslGroup g;
            g.name = gName;

            size_t open = line.find('[');
            if(open!=string::npos)
            {
                size_t close = line.find_last_of(']');
                if(close==string::npos || close<open)
                    throw runtime_error("Line "+to_string(lineno)+": missing ] in group definition");
                string inside = trimmed(line.substr(open+1, close-open-1));
                string token;
                stringstream ss(inside);
                while(std::getline(ss, token, ','))
                    g.values.push_back(trimmed(token));
            }
            db.groups[gName] = std::move(g);
            continue;
        }

        /* Entity header */
        if(startsWith(line, "#"))
        {
            string tmp = line.substr(1);
            vector<string> groupsLocal;
            extractGroups(tmp, groupsLocal);

            /* remove groups part to leave pure type */
            size_t atPos = tmp.find('@');
            string type = trimmed(atPos==string::npos ? tmp : tmp.substr(0, atPos));

            if(type.empty())
                throw runtime_error("Line "+to_string(lineno)+": empty entity type");

            db.entities.emplace_back();
            current = &db.entities.back();
            current->type = type;
            current->groups = std::move(groupsLocal);
            continue;
        }

        /* Field */
        if(startsWith(line,"-"))
        {
            if(!current)
                throw runtime_error("Line "+to_string(lineno)+": field found outside entity");

            string fld = trimmed(line.substr(1)); // drop '-'
            /* find '=' */
            size_t eq = fld.find('=');
            if(eq==string::npos)
                throw runtime_error("Line "+to_string(lineno)+": '=' expected in field");

            string key = trimmed(fld.substr(0, eq));
            string rest = trimmed(fld.substr(eq+1));

            /* value stops at first @ or end of string */
            size_t at = rest.find('@');
            string valPart = trimmed(at==string::npos ? rest : rest.substr(0, at));

            /* remove trailing ',' from value part */
            if(!valPart.empty() && valPart.back()==',')
                valPart.pop_back();

            AdslField f;
            f.name = key;
            try{
                f.value = parseValue(valPart);
            }catch(const std::exception& ex){
                throw runtime_error("Line "+to_string(lineno)+": "+ex.what());
            }

            /* groups after value */
            if(at!=string::npos){
                string groupPart = rest.substr(at);
                extractGroups(groupPart, f.groups);
            }

            current->fields.push_back(std::move(f));
            continue;
        }

        throw runtime_error("Line "+to_string(lineno)+": Unrecognised syntax -> "+line);
    }
    return true;
}

bool parseAdslFile(const string& filepath, AdslDatabase& db)
{
    ifstream file(filepath);
    if(!file) return false;
    return parseInternal(file, db);
}

bool parseAdslString(const string& data, AdslDatabase& db)
{
    istringstream ss(data);
    return parseInternal(ss, db);
}
