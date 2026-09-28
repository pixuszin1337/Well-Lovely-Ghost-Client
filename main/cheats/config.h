#pragma once

#include <string>
#include <map>
#include <vector>

class config_data {
    std::map<std::string, std::string> m_values;
public:
    void set_float(const char* key, float v) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.6f", v);
        m_values[key] = buf;
    }
    void set_int(const char* key, int v) {
        m_values[key] = std::to_string(v);
    }
    void set_bool(const char* key, bool v) {
        m_values[key] = v ? "1" : "0";
    }
    void set_str(const char* key, const char* v) {
        m_values[key] = v;
    }
    void set_color(const char* key, const float* rgba) {
        char buf[128];
        snprintf(buf, sizeof(buf), "%.4f,%.4f,%.4f,%.4f", rgba[0], rgba[1], rgba[2], rgba[3]);
        m_values[key] = buf;
    }

    float get_float(const char* key, float def) const {
        auto it = m_values.find(key);
        if (it == m_values.end()) return def;
        try { return std::stof(it->second); } catch (...) { return def; }
    }
    int get_int(const char* key, int def) const {
        auto it = m_values.find(key);
        if (it == m_values.end()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    }
    bool get_bool(const char* key, bool def) const {
        auto it = m_values.find(key);
        if (it == m_values.end()) return def;
        return it->second == "1";
    }
    std::string get_str(const char* key, const char* def) const {
        auto it = m_values.find(key);
        if (it == m_values.end()) return def;
        return it->second;
    }
    void get_color(const char* key, float* rgba, const float* def) const {
        auto it = m_values.find(key);
        if (it == m_values.end()) { memcpy(rgba, def, 16); return; }
        if (sscanf_s(it->second.c_str(), "%f,%f,%f,%f", &rgba[0], &rgba[1], &rgba[2], &rgba[3]) != 4)
            memcpy(rgba, def, 16);
    }

    const std::map<std::string, std::string>& values() const { return m_values; }
    std::map<std::string, std::string>& values() { return m_values; }
};

namespace config {
    bool save(const std::string& name);
    bool load(const std::string& name);
    bool remove(const std::string& name);
    std::vector<std::string> list();
    std::string get_dir();
    void save_last(const std::string& name);
    bool load_last();
}
