#include "config.h"
#include "module_manager.h"
#include "visuals_config.h"
#include "../../sdk/includes.h"
#include <fstream>
#include <filesystem>
#include <sstream>

std::string config::get_dir() {
    char appdata[MAX_PATH]{};
    if (GetEnvironmentVariableA("APPDATA", appdata, MAX_PATH) == 0)
        return "";
    auto dir = std::filesystem::path(appdata) / "WellLovely" / "configs";
    std::filesystem::create_directories(dir);
    return dir.string();
}

bool config::save(const std::string& name) {
    if (name.empty() || !modules::instance) return false;

    auto dir = get_dir();
    if (dir.empty()) return false;

    auto path = std::filesystem::path(dir) / (name + ".cfg");
    std::ofstream file(path);
    if (!file) return false;

    for (const auto& m : modules::instance->get()) {
        file << "[" << m->name << "]\n";
        file << "enabled=" << (m->enabled ? "1" : "0") << "\n";
        file << "keybind=" << m->keybind << "\n";

        config_data data;
        m->save_config(data);
        for (const auto& [k, v] : data.values())
            file << k << "=" << v << "\n";

        file << "\n";
    }

    file << "[__visuals__]\n";
    char buf[128];
    snprintf(buf, sizeof(buf), "%.4f,%.4f,%.4f,%.4f",
        visuals::esp_color[0], visuals::esp_color[1],
        visuals::esp_color[2], visuals::esp_color[3]);
    file << "esp_color=" << buf << "\n\n";

    return true;
}

bool config::load(const std::string& name) {
    if (name.empty() || !modules::instance) return false;

    auto dir = get_dir();
    if (dir.empty()) return false;

    auto path = std::filesystem::path(dir) / (name + ".cfg");
    std::ifstream file(path);
    if (!file) return false;

    std::map<std::string, std::map<std::string, std::string>> sections;
    std::string current_section;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.front() == '[' && line.back() == ']') {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        sections[current_section][line.substr(0, eq)] = line.substr(eq + 1);
    }

    for (auto& m : modules::instance->get()) {
        auto it = sections.find(m->name);
        if (it == sections.end()) continue;

        const auto& kvs = it->second;

        auto find = [&](const std::string& key) -> std::string {
            auto kv = kvs.find(key);
            return kv != kvs.end() ? kv->second : "";
        };

        auto en = find("enabled");
        if (!en.empty()) m->enabled = (en == "1");

        auto kb = find("keybind");
        if (!kb.empty()) {
            try { m->keybind = std::stoi(kb); } catch (...) {}
        }

        config_data data;
        for (const auto& [k, v] : kvs) {
            if (k != "enabled" && k != "keybind")
                data.values()[k] = v;
        }
        m->load_config(data);
    }

    auto vis_it = sections.find("__visuals__");
    if (vis_it != sections.end()) {
        auto col_it = vis_it->second.find("esp_color");
        if (col_it != vis_it->second.end()) {
            sscanf_s(col_it->second.c_str(), "%f,%f,%f,%f",
                &visuals::esp_color[0], &visuals::esp_color[1],
                &visuals::esp_color[2], &visuals::esp_color[3]);
        }
    }

    return true;
}

bool config::remove(const std::string& name) {
    auto dir = get_dir();
    if (dir.empty()) return false;
    auto path = std::filesystem::path(dir) / (name + ".cfg");
    return std::filesystem::remove(path);
}

void config::save_last(const std::string& name) {
    char appdata[MAX_PATH]{};
    if (GetEnvironmentVariableA("APPDATA", appdata, MAX_PATH) == 0) return;
    auto path = std::filesystem::path(appdata) / "WellLovely" / "last_config.txt";
    std::ofstream f(path);
    if (f) f << name;
}

bool config::load_last() {
    char appdata[MAX_PATH]{};
    if (GetEnvironmentVariableA("APPDATA", appdata, MAX_PATH) == 0) return false;
    auto path = std::filesystem::path(appdata) / "WellLovely" / "last_config.txt";
    std::ifstream f(path);
    if (!f) return false;
    std::string name;
    std::getline(f, name);
    if (name.empty()) return false;
    return load(name);
}

std::vector<std::string> config::list() {
    std::vector<std::string> result;
    auto dir = get_dir();
    if (dir.empty()) return result;

    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() == ".cfg")
            result.push_back(entry.path().stem().string());
    }
    std::sort(result.begin(), result.end());
    return result;
}
