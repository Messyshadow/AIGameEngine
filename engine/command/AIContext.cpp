#include "AIContext.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "../core/App.h"
#include "../core/ECS.h"
#include "../core/ResourceManager.h"
#include "CommandRegistry.h"

namespace AIForge {

// ====================================================================
//  Snapshot —— 用于 Diff(只输出"上次 Export 之后变化的部分")
// ====================================================================
namespace {

struct EntitySnap {
    std::string name;
    Vec3 pos{0, 0, 0};
    std::vector<std::string> components;
};

struct ContextSnap {
    bool valid = false;
    std::unordered_map<uint32_t, EntitySnap> entities;
};

ContextSnap g_lastSnap;

EntitySnap CaptureEntity(const Entity& e) {
    EntitySnap s;
    s.name = e.GetName();
    s.components = e.ListComponentTypes();
    if (auto* t = e.GetComponent<Transform>()) {
        s.pos = t->position;
    }
    return s;
}

ContextSnap CaptureWorld(const World& w) {
    ContextSnap s;
    s.valid = true;
    s.entities.reserve(w.GetEntities().size());
    for (auto& e : w.GetEntities()) {
        s.entities[e->GetID()] = CaptureEntity(*e);
    }
    return s;
}

bool Vec3NearlyEqual(const Vec3& a, const Vec3& b, float eps = 1e-3f) {
    return std::fabs(a.x - b.x) < eps && std::fabs(a.y - b.y) < eps &&
           std::fabs(a.z - b.z) < eps;
}

bool SameComponents(const std::vector<std::string>& a,
                    const std::vector<std::string>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i] != b[i]) return false;
    return true;
}

}  // namespace

// ====================================================================
//  Token 估算 —— 近似:ASCII 4 char/token,非 ASCII 1.5 char/token
// ====================================================================
int AIContext::EstimateTokens(const std::string& text) {
    int ascii = 0, other = 0;
    for (unsigned char c : text) {
        if (c < 128) ++ascii;
        else ++other;
    }
    // ascii / 4 + other * 2 / 3   ≈   ascii*0.25 + other*0.67
    return ascii / 4 + (other * 2) / 3;
}

void AIContext::ResetSnapshot() { g_lastSnap = ContextSnap{}; }

// ====================================================================
//  Export(simple) —— 向后兼容 Ch 02 的 std::string 接口
// ====================================================================
std::string AIContext::Export(App& app) {
    ExportOptions opts;
    return Export(app, opts).markdown;
}

// ====================================================================
//  Section builders —— 把单独的段落构造成 markdown 片段
// ====================================================================
namespace {

std::string SectionEntities(const World& w) {
    std::ostringstream ss;
    ss << "## Entities (" << w.GetEntityCount() << ")\n";
    if (w.GetEntityCount() == 0) {
        ss << "_(no entities)_\n\n";
        return ss.str();
    }
    for (auto& e : w.GetEntities()) {
        ss << "- **" << e->GetName() << "** (id=" << e->GetID()
           << (e->IsActive() ? "" : ", inactive") << ")\n";
        auto types = e->ListComponentTypes();
        if (types.empty()) {
            ss << "    - components: _(none)_\n";
        } else {
            ss << "    - components:";
            for (auto& t : types) ss << " " << t;
            ss << "\n";
        }
        if (auto* t = e->GetComponent<Transform>()) {
            ss << "    - position: (" << t->position.x << ", " << t->position.y
               << ", " << t->position.z << ")\n";
        }
    }
    ss << "\n";
    return ss.str();
}

std::string SectionCommands(const CommandRegistry& cmds) {
    std::ostringstream ss;
    auto list = cmds.List();
    ss << "## Available Commands (" << list.size() << ")\n";
    for (auto* e : list) {
        ss << "- `" << e->verb << "` — " << e->description << "\n";
        ss << "    - usage: `" << e->usage << "`\n";
    }
    ss << "\n";
    return ss.str();
}

std::string SectionResources(const ResourceManager& rm) {
    std::ostringstream ss;
    auto files = rm.ListFiles();
    ss << "## Resources under `" << rm.GetAssetRoot() << "/` (" << files.size()
       << " files)\n";
    if (files.empty()) {
        ss << "_(no files)_\n\n";
        return ss.str();
    }
    int cap = static_cast<int>(files.size());
    if (cap > 100) cap = 100;
    for (int i = 0; i < cap; ++i) ss << "- `" << files[i] << "`\n";
    if (static_cast<int>(files.size()) > cap)
        ss << "- ... (" << (files.size() - cap) << " more)\n";
    ss << "\n";
    return ss.str();
}

std::string SectionHints() {
    std::ostringstream ss;
    ss << "## Hints for AI\n";
    ss << "- Use commands above (via App::Execute) — they are stable text APIs.\n";
    ss << "- For incremental updates, call AIContext::Diff() instead of full Export — saves 70-95% tokens.\n";
    ss << "- See docs/AI_GUIDE.md for the AI development workflow.\n";
    return ss.str();
}

}  // namespace

// ====================================================================
//  Export(opts) —— 主入口,带 token 预算 + 选项控制 + 快照更新
// ====================================================================
AIContext::ExportResult AIContext::Export(App& app, const ExportOptions& opts) {
    if (opts.diffSinceLastExport) return Diff(app);

    ExportResult result;
    std::ostringstream ss;
    ss << "# AIForge Project State\n\n";
    ss << "Phase: 1 (Engine Kernel)\n\n";
    std::string header = ss.str();

    // 按优先级依次拼接 section,每加一段检查 budget
    auto tryAppend = [&](const std::string& section) -> bool {
        int totalIfAdded = EstimateTokens(result.markdown + section);
        if (totalIfAdded > opts.maxTokens) {
            result.truncated = true;
            return false;
        }
        result.markdown += section;
        return true;
    };

    result.markdown = header;
    if (opts.includeEntities && app.GetWorld()) {
        tryAppend(SectionEntities(*app.GetWorld()));
    }
    if (opts.includeCommands && app.GetCommands() && !result.truncated) {
        tryAppend(SectionCommands(*app.GetCommands()));
    }
    if (opts.includeResources && app.GetResources() && !result.truncated) {
        tryAppend(SectionResources(*app.GetResources()));
    }
    if (opts.includeHints && !result.truncated) {
        tryAppend(SectionHints());
    }
    if (result.truncated) {
        result.markdown +=
            "\n_(... truncated to fit token budget; raise maxTokens or use "
            "Diff)_\n";
    }
    result.estimatedTokens = EstimateTokens(result.markdown);

    // 更新快照供后续 Diff
    if (app.GetWorld()) g_lastSnap = CaptureWorld(*app.GetWorld());
    return result;
}

// ====================================================================
//  Diff —— 对比 g_lastSnap 与当前 World,只输出变化
// ====================================================================
AIContext::ExportResult AIContext::Diff(App& app) {
    ExportResult result;
    if (!app.GetWorld()) {
        result.markdown = "(no world)\n";
        result.estimatedTokens = EstimateTokens(result.markdown);
        return result;
    }
    if (!g_lastSnap.valid) {
        // 首次调用 — 退化为完整 Export
        ExportOptions opts;
        auto full = Export(app, opts);
        full.markdown = "## (no previous snapshot — full export instead)\n\n" +
                         full.markdown;
        full.estimatedTokens = EstimateTokens(full.markdown);
        return full;
    }

    auto current = CaptureWorld(*app.GetWorld());

    // 把 id 收集起来排序,输出稳定
    std::vector<uint32_t> currentIDs, oldIDs;
    currentIDs.reserve(current.entities.size());
    oldIDs.reserve(g_lastSnap.entities.size());
    for (auto& kv : current.entities) currentIDs.push_back(kv.first);
    for (auto& kv : g_lastSnap.entities) oldIDs.push_back(kv.first);
    std::sort(currentIDs.begin(), currentIDs.end());
    std::sort(oldIDs.begin(), oldIDs.end());

    std::ostringstream ss;
    ss << "## Diff since last export\n";

    bool anyChange = false;

    // Added / Modified
    for (uint32_t id : currentIDs) {
        const auto& cur = current.entities[id];
        auto it = g_lastSnap.entities.find(id);
        if (it == g_lastSnap.entities.end()) {
            // 新增
            ss << "+ entity '" << cur.name << "' (id=" << id << ") added with [";
            for (size_t i = 0; i < cur.components.size(); ++i) {
                if (i) ss << ",";
                ss << cur.components[i];
            }
            ss << "]\n";
            anyChange = true;
            continue;
        }
        const auto& old = it->second;

        if (cur.name != old.name) {
            ss << "~ entity (id=" << id << ") renamed: '" << old.name
               << "' → '" << cur.name << "'\n";
            anyChange = true;
        }
        if (!Vec3NearlyEqual(cur.pos, old.pos)) {
            ss.precision(2);
            ss << std::fixed;
            ss << "~ '" << cur.name << "' position: (" << old.pos.x << ","
               << old.pos.y << "," << old.pos.z << ") -> (" << cur.pos.x
               << "," << cur.pos.y << "," << cur.pos.z << ")\n";
            ss.unsetf(std::ios::fixed);
            anyChange = true;
        }
        if (!SameComponents(cur.components, old.components)) {
            ss << "~ '" << cur.name << "' components changed: [";
            for (size_t i = 0; i < old.components.size(); ++i) {
                if (i) ss << ",";
                ss << old.components[i];
            }
            ss << "] -> [";
            for (size_t i = 0; i < cur.components.size(); ++i) {
                if (i) ss << ",";
                ss << cur.components[i];
            }
            ss << "]\n";
            anyChange = true;
        }
    }

    // Removed
    for (uint32_t id : oldIDs) {
        if (current.entities.find(id) == current.entities.end()) {
            const auto& old = g_lastSnap.entities[id];
            ss << "- entity '" << old.name << "' (id=" << id << ") removed\n";
            anyChange = true;
        }
    }

    if (!anyChange) ss << "_(no changes)_\n";

    result.markdown = ss.str();
    result.estimatedTokens = EstimateTokens(result.markdown);

    // 更新快照
    g_lastSnap = std::move(current);
    return result;
}

// ====================================================================
//  ExportToFile —— 简单导出 + 写文件
// ====================================================================
bool AIContext::ExportToFile(const std::string& path, App& app,
                             std::string& errMsg) {
    std::string text = Export(app);
    std::ofstream f(path, std::ios::out | std::ios::binary);
    if (!f.is_open()) {
        errMsg = "cannot open output file: " + path;
        return false;
    }
    f.write(text.data(), static_cast<std::streamsize>(text.size()));
    return true;
}

}  // namespace AIForge
