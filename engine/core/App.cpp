#include "App.h"

#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>

#include <cstdio>
#include <fstream>
#include <sstream>

#include "../command/AIContext.h"
#include "../command/CommandRegistry.h"

namespace AIForge {

App::App()
    : m_window(std::make_unique<Window>()),
      m_input(std::make_unique<Input>()),
      m_time(std::make_unique<Time>()),
      m_world(std::make_unique<World>()),
      m_resources(std::make_unique<ResourceManager>()),
      m_commands(std::make_unique<CommandRegistry>()) {}

App::~App() { Shutdown(); }

std::unique_ptr<App> App::Create() { return std::make_unique<App>(); }

std::unique_ptr<App> App::Create(const std::string& configPath) {
    auto app = std::make_unique<App>();

    std::ifstream f(configPath, std::ios::binary);
    if (!f.is_open()) {
        std::fprintf(stderr,
                     "[App] WARN: cannot open '%s'; using default config\n",
                     configPath.c_str());
        return app;
    }

    nlohmann::json j;
    try {
        f >> j;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "[App] WARN: parse '%s' failed: %s; using defaults\n",
                     configPath.c_str(), e.what());
        return app;
    }

    Window::Config wc = app->GetWindowConfig();
    if (j.contains("window")) {
        const auto& w = j["window"];
        if (w.contains("title")) wc.title = w["title"].get<std::string>();
        if (w.contains("width")) wc.width = w["width"].get<int>();
        if (w.contains("height")) wc.height = w["height"].get<int>();
        if (w.contains("fullscreen")) wc.fullscreen = w["fullscreen"].get<bool>();
        if (w.contains("vsync")) wc.vsync = w["vsync"].get<bool>();
    }
    app->SetWindowConfig(wc);

    if (j.contains("engine")) {
        const auto& e = j["engine"];
        if (e.contains("maxDeltaTime"))
            app->m_time->SetMaxDeltaTime(e["maxDeltaTime"].get<float>());
    }

    if (j.contains("resources")) {
        const auto& r = j["resources"];
        if (r.contains("root"))
            app->m_resources->SetAssetRoot(r["root"].get<std::string>());
    }

    return app;
}

bool App::Init() {
    if (m_initialized) return true;

    if (!m_window->Init(m_windowCfg)) {
        std::fprintf(stderr, "[App] Window::Init failed\n");
        return false;
    }
    m_baseTitle = m_windowCfg.title;
    RegisterBuiltinCommands();
    m_initialized = true;
    std::printf("[App] initialized (entities=%d, commands=%zu)\n",
                m_world->GetEntityCount(), m_commands->List().size());
    return true;
}

void App::Shutdown() {
    if (!m_initialized) return;
    m_world->DestroyAll();
    m_window->Shutdown();
    m_initialized = false;
}

void App::SetClearColor(float r, float g, float b, float a) {
    m_clearColor[0] = r;
    m_clearColor[1] = g;
    m_clearColor[2] = b;
    m_clearColor[3] = a;
}

void App::SetShowFPSInTitle(bool enable) { m_showFPS = enable; }

void App::RequestClose() { m_window->RequestClose(); }
bool App::ShouldClose() const { return m_window->ShouldClose(); }

void App::Tick() {
    if (!m_initialized) return;

    m_input->NewFrame();
    PollEvents();
    m_time->Tick();

    if (m_escQuits && m_input->IsKeyPressed(K_ESC)) {
        m_window->RequestClose();
    }

    m_world->Update(m_time->DeltaTime());

    // 顺序很重要:先清屏再调用户回调,这样用户在 SetUpdateCallback 里画的东西
    // 不会被随后的 glClear 抹掉。Ch 02 的调试点渲染依赖这个顺序。
    RenderClear();
    if (m_onUpdate) m_onUpdate(*this, m_time->DeltaTime());

    UpdateTitle();
    m_window->SwapBuffers();
}

void App::Run() {
    while (m_initialized && !m_window->ShouldClose()) {
        Tick();
    }
}

bool App::Execute(const std::string& cmd, std::string& errMsg) {
    auto r = m_commands->Execute(cmd);
    if (!r.ok) {
        errMsg = r.errMsg;
        return false;
    }
    if (!r.output.empty()) std::printf("%s\n", r.output.c_str());
    return true;
}

bool App::Execute(const std::string& cmd) {
    std::string e;
    bool ok = Execute(cmd, e);
    if (!ok) std::fprintf(stderr, "[Execute] %s\n", e.c_str());
    return ok;
}

void App::PollEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_EVENT_QUIT:
                m_window->RequestClose();
                break;
            case SDL_EVENT_KEY_DOWN:
                if (!e.key.repeat) m_input->OnKeyDown((int)e.key.scancode);
                break;
            case SDL_EVENT_KEY_UP:
                m_input->OnKeyUp((int)e.key.scancode);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                m_input->OnMouseMotion((int)e.motion.x, (int)e.motion.y,
                                        (int)e.motion.xrel, (int)e.motion.yrel);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                m_input->OnMouseButtonDown(e.button.button);
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                m_input->OnMouseButtonUp(e.button.button);
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                m_input->OnMouseWheel(e.wheel.y);
                break;
            default:
                break;
        }
    }
}

void App::RenderClear() {
    int w = m_window->GetWidth();
    int h = m_window->GetHeight();
    glViewport(0, 0, w, h);
    glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2],
                 m_clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void App::UpdateTitle() {
    if (!m_showFPS) return;
    m_titleUpdateAccum += m_time->DeltaTime();
    if (m_titleUpdateAccum < 0.25f) return;
    m_titleUpdateAccum = 0.0f;
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s — %.1f FPS", m_baseTitle.c_str(),
                  m_time->FPS());
    m_window->SetTitle(buf);
}

void App::RegisterBuiltinCommands() {
    auto* world = m_world.get();
    auto* cmds = m_commands.get();
    auto* self = this;

    cmds->Register(
        "spawn", "create a new entity (optionally with position)",
        "spawn <name> [at x,y,z]",
        [world](const ParsedCommand& c) -> CommandResult {
            if (c.tokens.empty())
                return CommandResult::Fail("usage: spawn <name> [at x,y,z]");
            const std::string& name = c.tokens[0];
            auto* e = world->Spawn(name);
            int atIdx = c.FindKeyword("at");
            if (atIdx >= 0 && atIdx + 1 < static_cast<int>(c.tokens.size())) {
                Vec3 pos;
                if (!ParseVec3(c.tokens[atIdx + 1], pos)) {
                    return CommandResult::Fail(
                        "invalid position '" + c.tokens[atIdx + 1] +
                        "'; expected 'x,y,z'");
                }
                e->GetComponent<Transform>()->position = pos;
            }
            std::ostringstream o;
            o << "spawned '" << name << "' (id=" << e->GetID() << ")";
            return CommandResult::Ok(o.str());
        });

    cmds->Register(
        "destroy", "destroy an entity by name", "destroy <name>",
        [world](const ParsedCommand& c) -> CommandResult {
            if (c.tokens.empty())
                return CommandResult::Fail("usage: destroy <name>");
            auto* e = world->Find(c.tokens[0]);
            if (!e) return CommandResult::Fail("no entity named '" + c.tokens[0] + "'");
            world->Destroy(e);
            return CommandResult::Ok("destroyed '" + c.tokens[0] + "'");
        });

    cmds->Register(
        "list", "list entities (or commands)", "list entities | list commands",
        [self, world, cmds](const ParsedCommand& c) -> CommandResult {
            std::string what = c.tokens.empty() ? "entities" : c.tokens[0];
            std::ostringstream o;
            if (what == "entities") {
                o << "entities (" << world->GetEntityCount() << "):";
                for (auto& e : world->GetEntities()) {
                    o << "\n  - " << e->GetName() << " (id=" << e->GetID() << ")";
                }
            } else if (what == "commands") {
                auto list = cmds->List();
                o << "commands (" << list.size() << "):";
                for (auto* en : list) o << "\n  - " << en->verb;
            } else {
                return CommandResult::Fail("usage: list entities | list commands");
            }
            (void)self;
            return CommandResult::Ok(o.str());
        });

    cmds->Register(
        "set", "set entity property", "set <entity>.<prop> <value>",
        [world](const ParsedCommand& c) -> CommandResult {
            if (c.tokens.size() < 2)
                return CommandResult::Fail(
                    "usage: set <entity>.<prop> <value>");
            const std::string& target = c.tokens[0];
            auto dot = target.find('.');
            if (dot == std::string::npos)
                return CommandResult::Fail("target must be 'entity.prop'");
            std::string ename = target.substr(0, dot);
            std::string prop = target.substr(dot + 1);
            auto* e = world->Find(ename);
            if (!e) return CommandResult::Fail("no entity named '" + ename + "'");
            const std::string& val = c.tokens[1];
            if (prop == "position" || prop == "rotation" || prop == "scale") {
                Vec3 v;
                if (!ParseVec3(val, v))
                    return CommandResult::Fail("expected 'x,y,z' for " + prop);
                auto* t = e->GetComponent<Transform>();
                if (prop == "position") t->position = v;
                else if (prop == "rotation") t->rotation = v;
                else t->scale = v;
                return CommandResult::Ok("set " + target + " = " + val);
            }
            if (prop == "active") {
                bool b = (val == "true" || val == "1");
                e->SetActive(b);
                return CommandResult::Ok("set " + target + " = " + (b ? "true" : "false"));
            }
            return CommandResult::Fail(
                "unknown property '" + prop +
                "'. Phase 1 supports: position, rotation, scale, active");
        });

    cmds->Register(
        "get", "get entity property", "get <entity>.<prop>",
        [world](const ParsedCommand& c) -> CommandResult {
            if (c.tokens.empty())
                return CommandResult::Fail("usage: get <entity>.<prop>");
            const std::string& target = c.tokens[0];
            auto dot = target.find('.');
            if (dot == std::string::npos)
                return CommandResult::Fail("target must be 'entity.prop'");
            std::string ename = target.substr(0, dot);
            std::string prop = target.substr(dot + 1);
            auto* e = world->Find(ename);
            if (!e) return CommandResult::Fail("no entity named '" + ename + "'");
            std::ostringstream o;
            if (prop == "position" || prop == "rotation" || prop == "scale") {
                auto* t = e->GetComponent<Transform>();
                Vec3 v = (prop == "position" ? t->position
                          : prop == "rotation" ? t->rotation
                                               : t->scale);
                o << target << " = " << v.x << "," << v.y << "," << v.z;
                return CommandResult::Ok(o.str());
            }
            if (prop == "active") {
                o << target << " = " << (e->IsActive() ? "true" : "false");
                return CommandResult::Ok(o.str());
            }
            if (prop == "id") {
                o << target << " = " << e->GetID();
                return CommandResult::Ok(o.str());
            }
            return CommandResult::Fail("unknown property '" + prop + "'");
        });

    cmds->Register(
        "export", "export AI context as markdown",
        "export context [<filepath>]",
        [self](const ParsedCommand& c) -> CommandResult {
            std::string what = c.tokens.empty() ? "context" : c.tokens[0];
            if (what != "context")
                return CommandResult::Fail("usage: export context [<filepath>]");
            if (c.tokens.size() >= 2) {
                std::string err;
                if (!AIContext::ExportToFile(c.tokens[1], *self, err))
                    return CommandResult::Fail(err);
                return CommandResult::Ok("exported to " + c.tokens[1]);
            }
            return CommandResult::Ok(AIContext::Export(*self));
        });

    cmds->Register(
        "help", "list all commands with usage", "help [<verb>]",
        [cmds](const ParsedCommand& c) -> CommandResult {
            std::ostringstream o;
            if (!c.tokens.empty()) {
                auto* e = cmds->Find(c.tokens[0]);
                if (!e) return CommandResult::Fail("no such command: " + c.tokens[0]);
                o << e->verb << " — " << e->description << "\n  usage: " << e->usage;
                return CommandResult::Ok(o.str());
            }
            o << "AIForge commands:";
            for (auto* en : cmds->List()) {
                o << "\n  " << en->verb << " — " << en->description;
                o << "\n      " << en->usage;
            }
            return CommandResult::Ok(o.str());
        });

    cmds->Register(
        "quit", "request the application to close", "quit",
        [self](const ParsedCommand&) -> CommandResult {
            self->RequestClose();
            return CommandResult::Ok("quit requested");
        });
}

}  // namespace AIForge
