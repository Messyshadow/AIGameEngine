// =============================================================================
// AIForge Engine — Chapter 02: ECS + Time + Input
// =============================================================================
// 本章目标:让你"看见 ECS 在工作"。
//
// 你看到的:
//   - 一扇 1280x720 的森林绿窗口
//   - 9 个彩虹色圆点在做圆周运动(各自半径/速度/相位都不同)
//   - 1 个黄色圆点(player)在屏幕中央,WASD/QE 实时控制它
//   - 按 SPACE 屏幕上多一个粉色圆点(extra orbiter),BACKSPACE 减一个
//   - 标题栏实时显示 entities 数量、player 坐标、FPS
//   - 控制台每 0.5 秒打印一次完整 World 快照
//
// 教学要点(本章重点):
//   - 自定义 Component:继承 Component,实现 TypeName() / GetType()
//   - 添加组件:entity->AddComponent<Orbiter>() 返回裸指针,所有权在 World
//   - 查询组件:GetComponent<T>() 找不到返回 nullptr
//   - 按组件类型批量取实体:World::FindByComponent("Orbiter")
//   - 动态增删:Spawn/Destroy 即时生效
//
// 关于本文件里的渲染代码(dbg 命名空间):
//   这是一个【临时】最小 GL 调试渲染器 — 用 GL_POINTS 画彩色圆点。
//   Ch 04 会写正经的 SpriteBatcher,届时这段代码会被替换。
//   现阶段它的作用是:让 ECS 的"数据驱动"看得见,提升学习体验。
// =============================================================================

#include <glad/gl.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "engine/core/App.h"

// -----------------------------------------------------------------------------
// 自定义组件:Orbiter — 让实体绕一个中心点做圆周运动
// -----------------------------------------------------------------------------
struct Orbiter : AIForge::Component {
    AIForge::Vec3 center{0, 0, 0};
    float         radius = 1.0f;
    float         speed  = 1.0f;
    float         phase  = 0.0f;

    static constexpr const char* TypeName() { return "Orbiter"; }
    const char* GetType() const override { return TypeName(); }
};

// =============================================================================
//  TEMPORARY debug renderer  —— 详见文件头注释,Ch 04 的 SpriteBatcher 会取代它
// =============================================================================
namespace dbg {

GLuint vao  = 0;
GLuint vbo  = 0;
GLuint prog = 0;

static void CheckShader(GLuint sh, const char* tag) {
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (ok) return;
    char log[1024] = {0};
    glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
    std::fprintf(stderr, "[dbg] %s shader compile failed:\n%s\n", tag, log);
}

bool Init() {
    const char* vsrc =
        "#version 450 core\n"
        "layout(location=0) in vec2 a_pos;\n"
        "layout(location=1) in vec3 a_color;\n"
        "layout(location=2) in float a_size;\n"
        "out vec3 v_color;\n"
        "void main() {\n"
        "    gl_Position  = vec4(a_pos, 0.0, 1.0);\n"
        "    gl_PointSize = a_size;\n"
        "    v_color      = a_color;\n"
        "}\n";

    const char* fsrc =
        "#version 450 core\n"
        "in vec3 v_color;\n"
        "out vec4 frag;\n"
        "void main() {\n"
        // 把 GL_POINTS 渲成圆而不是方:在像素中心做半径 0.5 的圆裁剪
        "    vec2 c = gl_PointCoord - vec2(0.5);\n"
        "    if (dot(c, c) > 0.25) discard;\n"
        // 边缘平滑(可选):让圆点边缘有一点抗锯齿感
        "    float d = length(c);\n"
        "    float a = smoothstep(0.5, 0.45, d);\n"
        "    frag = vec4(v_color, a);\n"
        "}\n";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsrc, nullptr);
    glCompileShader(vs);
    CheckShader(vs, "vertex");

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsrc, nullptr);
    glCompileShader(fs);
    CheckShader(fs, "fragment");

    prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint linkOk = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &linkOk);
    if (!linkOk) {
        char log[1024] = {0};
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[dbg] program link failed:\n%s\n", log);
        return false;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // 顶点格式: vec2 pos, vec3 color, float size  → 共 6 floats / 顶点
    const GLsizei stride = 6 * sizeof(float);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return prog != 0;
}

void Draw(const std::vector<float>& verts) {
    if (verts.empty() || prog == 0) return;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(verts.size() * sizeof(float)),
                 verts.data(), GL_DYNAMIC_DRAW);
    glUseProgram(prog);
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, (GLsizei)(verts.size() / 6));
    glBindVertexArray(0);
    glUseProgram(0);
}

}  // namespace dbg

// -----------------------------------------------------------------------------
// 把世界坐标(XZ 平面)映射到屏幕 NDC([-1,1]^2)。
// 假设场景半径约 8,1280x720 窗口,不做 aspect 修正(简化教学)。
// -----------------------------------------------------------------------------
inline float MapX(float wx) { return wx / 8.0f; }
inline float MapY(float wz) { return -wz / 4.5f; }   // 反向:+z 朝屏幕下方

// 给每个实体一个颜色:player 黄、orbiter_N 彩虹、extra_N 粉、其他灰
static void EntityColor(const std::string& name, float& r, float& g, float& b) {
    if (name == "player") { r = 1.0f; g = 0.95f; b = 0.30f; return; }

    if (name.rfind("orbiter_", 0) == 0) {
        int idx = std::atoi(name.c_str() + 8);
        // hue 平均分到 9 个 orbiter
        float h = (float)((idx - 1) % 9) / 9.0f * 6.0f;
        int   i = (int)h;
        float f = h - i;
        switch (i) {
            case 0: r = 1; g = f; b = 0; break;
            case 1: r = 1 - f; g = 1; b = 0; break;
            case 2: r = 0; g = 1; b = f; break;
            case 3: r = 0; g = 1 - f; b = 1; break;
            case 4: r = f; g = 0; b = 1; break;
            default:r = 1; g = 0; b = 1 - f; break;
        }
        return;
    }

    if (name.rfind("extra_", 0) == 0) { r = 1.0f; g = 0.45f; b = 0.85f; return; }
    r = 0.7f; g = 0.7f; b = 0.7f;
}

int main(int /*argc*/, char* /*argv*/[]) {
    auto app = AIForge::App::Create();
    if (!app) {
        std::fprintf(stderr, "[Ch02] App::Create returned null\n");
        return 1;
    }

    AIForge::Window::Config wc;
    wc.title  = "AIForge Ch 02 - ECS + Time + Input";
    wc.width  = 1280;
    wc.height = 720;
    wc.vsync  = true;
    app->SetWindowConfig(wc);

    if (!app->Init()) {
        std::fprintf(stderr, "[Ch02] App::Init failed\n");
        return 1;
    }

    // 必须在 App::Init 之后(此时 GL 上下文才可用)
    if (!dbg::Init()) {
        std::fprintf(stderr, "[Ch02] dbg::Init failed\n");
        return 1;
    }

    app->SetClearColor(0.10f, 0.30f, 0.18f, 1.0f);
    app->SetShowFPSInTitle(false);

    auto* world = app->GetWorld();

    // -------------------------------------------------- 创建实体 ---
    world->Spawn("player");
    for (int i = 1; i <= 9; ++i) {
        char name[32];
        std::snprintf(name, sizeof(name), "orbiter_%d", i);
        auto* e   = world->Spawn(name);
        auto* orb = e->AddComponent<Orbiter>();
        orb->center = {0.0f, 0.0f, 0.0f};
        orb->radius = 0.8f + static_cast<float>(i) * 0.4f;
        orb->speed  = 0.4f + static_cast<float>(i) * 0.15f;
        orb->phase  = static_cast<float>(i) * 0.6f;
    }

    std::printf("\n");
    std::printf("==================================================\n");
    std::printf(" AIForge Chapter 02 - ECS + Time + Input\n");
    std::printf("==================================================\n");
    std::printf(" Entities: 1 player + 9 orbiters\n");
    std::printf(" Controls:\n");
    std::printf("   W A S D    - move player on XZ plane\n");
    std::printf("   Q / E      - move player on Y axis (does not change screen pos)\n");
    std::printf("   SPACE      - spawn extra orbiter\n");
    std::printf("   BACKSPACE  - destroy last extra orbiter\n");
    std::printf("   ESC        - quit\n");
    std::printf("==================================================\n\n");

    float printAccum = 0.0f;
    float titleAccum = 0.0f;
    int   extraCount = 0;

    app->SetUpdateCallback([&](AIForge::App& a, float dt) {
        auto* w  = a.GetWorld();
        auto* in = a.GetInput();

        // 1) WASD/QE 控制 player ---------------------------------------------------
        if (auto* p = w->Find("player")) {
            auto* t     = p->GetComponent<AIForge::Transform>();
            float speed = 3.0f * dt;
            if (in->IsKeyDown(AIForge::K_W)) t->position.z -= speed;
            if (in->IsKeyDown(AIForge::K_S)) t->position.z += speed;
            if (in->IsKeyDown(AIForge::K_A)) t->position.x -= speed;
            if (in->IsKeyDown(AIForge::K_D)) t->position.x += speed;
            if (in->IsKeyDown(AIForge::K_Q)) t->position.y -= speed;
            if (in->IsKeyDown(AIForge::K_E)) t->position.y += speed;
        }

        // 2) 批量更新所有 Orbiter --------------------------------------------------
        const float total = a.GetTime()->TotalTime();
        for (auto* e : w->FindByComponent("Orbiter")) {
            auto* orb = e->GetComponent<Orbiter>();
            auto* t   = e->GetComponent<AIForge::Transform>();
            const float angle = total * orb->speed + orb->phase;
            t->position.x = orb->center.x + std::cos(angle) * orb->radius;
            t->position.z = orb->center.z + std::sin(angle) * orb->radius;
            t->position.y = orb->center.y + std::sin(angle * 0.5f) * 0.3f;
        }

        // 3) SPACE / BACKSPACE 增删 ------------------------------------------------
        if (in->IsKeyPressed(AIForge::K_SPACE)) {
            char name[32];
            std::snprintf(name, sizeof(name), "extra_%d", ++extraCount);
            auto* e   = w->Spawn(name);
            auto* orb = e->AddComponent<Orbiter>();
            orb->radius = 4.0f + static_cast<float>(extraCount) * 0.2f;
            orb->speed  = 1.2f;
            orb->phase  = total;
            std::printf(">>> spawned %s (now %d entities)\n",
                        name, w->GetEntityCount());
        }
        if (in->IsKeyPressed(AIForge::K_BACKSPACE)) {
            if (extraCount <= 0) {
                std::printf(">>> no extra orbiter to destroy\n");
            } else {
                char name[32];
                std::snprintf(name, sizeof(name), "extra_%d", extraCount);
                if (auto* e = w->Find(name)) {
                    w->Destroy(e);
                    std::printf(">>> destroyed %s (now %d entities)\n",
                                name, w->GetEntityCount());
                }
                --extraCount;
            }
        }

        // 4) 标题栏每 0.2s 刷一次 --------------------------------------------------
        titleAccum += dt;
        if (titleAccum >= 0.2f) {
            titleAccum = 0.0f;
            float px = 0, py = 0, pz = 0;
            if (auto* p = w->Find("player"))
                if (auto* t = p->GetComponent<AIForge::Transform>())
                    { px = t->position.x; py = t->position.y; pz = t->position.z; }
            char buf[256];
            std::snprintf(buf, sizeof(buf),
                "AIForge Ch 02 | entities=%d | player=(%.2f,%.2f,%.2f) | %.0f FPS",
                w->GetEntityCount(), px, py, pz, a.GetTime()->FPS());
            a.GetWindow()->SetTitle(buf);
        }

        // 5) 控制台每 0.5s 打印 -----------------------------------------------------
        printAccum += dt;
        if (printAccum >= 0.5f) {
            printAccum = 0.0f;
            std::printf("\n--- t=%.1fs, entities=%d, fps=%.1f ---\n",
                        a.GetTime()->TotalTime(), w->GetEntityCount(),
                        a.GetTime()->FPS());
            for (auto& ent : w->GetEntities()) {
                auto* t = ent->GetComponent<AIForge::Transform>();
                std::printf("  %-12s id=%-2u pos=(%6.2f,%6.2f,%6.2f) comps=[",
                            ent->GetName().c_str(), ent->GetID(),
                            t->position.x, t->position.y, t->position.z);
                auto types = ent->ListComponentTypes();
                for (size_t i = 0; i < types.size(); ++i) {
                    std::printf("%s%s", i ? "," : "", types[i].c_str());
                }
                std::printf("]\n");
            }
        }

        // 6) 把每个实体画成一个彩色圆点(本章核心视觉效果)----------------------
        std::vector<float> verts;
        verts.reserve(w->GetEntityCount() * 6);
        for (auto& ent : w->GetEntities()) {
            auto* t = ent->GetComponent<AIForge::Transform>();
            float r, g, b; EntityColor(ent->GetName(), r, g, b);
            // 从尺寸看出 player 比 orbiter 显眼,extra 中等
            float size = 14.0f;
            if (ent->GetName() == "player") size = 28.0f;
            else if (ent->GetName().rfind("extra_", 0) == 0) size = 20.0f;
            verts.push_back(MapX(t->position.x));
            verts.push_back(MapY(t->position.z));
            verts.push_back(r);
            verts.push_back(g);
            verts.push_back(b);
            verts.push_back(size);
        }
        dbg::Draw(verts);
    });

    app->Run();
    app->Shutdown();
    std::printf("\n[Ch02] clean exit. Bye!\n");
    return 0;
}
