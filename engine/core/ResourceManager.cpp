#include "ResourceManager.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace AIForge {

namespace fs = std::filesystem;

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

void ResourceManager::SetAssetRoot(const std::string& root) { m_root = root; }

std::string ResourceManager::Resolve(const std::string& relPath) const {
    fs::path p(relPath);
    if (p.is_absolute()) return relPath;
    fs::path rooted = fs::path(m_root) / p;
    return rooted.string();
}

bool ResourceManager::ReadText(const std::string& path, std::string& out,
                               std::string& errMsg) const {
    std::string full = Resolve(path);
    std::ifstream f(full, std::ios::in | std::ios::binary);
    if (!f.is_open()) {
        errMsg = "Failed to open: " + full;
        return false;
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

std::vector<std::string> ResourceManager::ListFiles(
    const std::string& subdir, const std::string& extFilter) const {
    std::vector<std::string> out;
    fs::path base = fs::path(m_root) / subdir;
    if (!fs::exists(base) || !fs::is_directory(base)) return out;

    std::error_code ec;
    for (auto it = fs::recursive_directory_iterator(base, ec);
         it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) {
            ec.clear();
            continue;
        }
        const auto& entry = *it;
        if (!entry.is_regular_file()) continue;
        if (!extFilter.empty() && entry.path().extension().string() != extFilter)
            continue;
        // 输出相对 root 的路径
        fs::path rel = fs::relative(entry.path(), m_root);
        out.push_back(rel.generic_string());
    }
    std::sort(out.begin(), out.end());
    return out;
}

}  // namespace AIForge
