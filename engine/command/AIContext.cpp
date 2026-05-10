#include "AIContext.h"

#include <cstdio>
#include <fstream>
#include <sstream>

#include "../core/App.h"
#include "../core/ECS.h"
#include "../core/ResourceManager.h"
#include "CommandRegistry.h"

namespace AIForge {

namespace {

void AppendEntities(std::ostringstream& ss, const World& world) {
    ss << "## Entities (" << world.GetEntityCount() << ")\n";
    if (world.GetEntityCount() == 0) {
        ss << "_(no entities)_\n\n";
        return;
    }
    for (const auto& e : world.GetEntities()) {
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
}

void AppendCommands(std::ostringstream& ss, const CommandRegistry& cmds) {
    auto list = cmds.List();
    ss << "## Available Commands (" << list.size() << ")\n";
    for (auto* e : list) {
        ss << "- `" << e->verb << "` — " << e->description << "\n";
        ss << "    - usage: `" << e->usage << "`\n";
    }
    ss << "\n";
}

void AppendResources(std::ostringstream& ss, const ResourceManager& rm) {
    auto files = rm.ListFiles();
    ss << "## Resources under `" << rm.GetAssetRoot() << "/` ("
       << files.size() << " files)\n";
    if (files.empty()) {
        ss << "_(no files)_\n\n";
        return;
    }
    int cap = static_cast<int>(files.size());
    if (cap > 100) cap = 100;
    for (int i = 0; i < cap; ++i) {
        ss << "- `" << files[i] << "`\n";
    }
    if (static_cast<int>(files.size()) > cap) {
        ss << "- ... (" << (files.size() - cap) << " more)\n";
    }
    ss << "\n";
}

}  // namespace

std::string AIContext::Export(App& app) {
    std::ostringstream ss;
    ss << "# AIForge Project State\n\n";
    ss << "Phase: 1 (Engine Kernel)\n\n";
    if (app.GetWorld()) AppendEntities(ss, *app.GetWorld());
    if (app.GetCommands()) AppendCommands(ss, *app.GetCommands());
    if (app.GetResources()) AppendResources(ss, *app.GetResources());
    ss << "## Hints for AI\n";
    ss << "- Use commands above (via App::Execute) — they are stable text APIs.\n";
    ss << "- To add new game logic, prefer JSON config + new commands over new C++.\n";
    ss << "- See docs/AI_GUIDE.md for the AI development workflow.\n";
    return ss.str();
}

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
