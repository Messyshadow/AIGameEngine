// =============================================================================
// AIForge Engine — Chapter 03: AI Command Protocol REPL
// =============================================================================
// 本章 demo:一个交互式 REPL(读-执行-打印 循环),让你亲手验证 AIForge 的
// "AI 命令协议 + Context 系统"——本引擎区别于所有现有游戏引擎的核心创新。
//
// 你看到的:
//   - 一个小窗口(640x360,深紫色),标题栏实时显示 World 状态
//   - 在【启动 demo 的控制台】里,有 "AIForge> " 命令行提示符
//   - 你输入命令(spawn / destroy / list / set / get / help / quit),引擎执行并打印结果
//   - meta 命令:`:ctx`、`:ctx tokens=N`、`:diff`、`:budget`、`:reset`、`:help`
//
// 你感受到的:
//   1. 在控制台输入 `spawn zombie at 5,0,0` → 引擎实时响应
//   2. 输入 `:ctx tokens=300` → 看到 AIContext 输出 + 实际 token 消耗数字
//   3. 输入 `spawn ...` 改一改,再输入 `:diff` → token 大幅下降(这就是 Diff 的力量)
//   4. 输入 `spwan` 故意拼错 → 引擎答:"did you mean: 'spawn'?"
//
// 必须从控制台启动(PowerShell / 终端),双击会看不到 REPL。
//
// 架构说明:
//   - 主线程:SDL3 主循环(刷新窗口 + 处理 SDL 事件)
//   - 后台线程:阻塞读 stdin,把每一行塞进队列(g_cmdQueue + g_qmtx)
//   - 主线程每帧 drain 队列,在主线程上执行命令(避免多线程访问 World)
//   - 这是教科书级的"消费者-生产者"模式
// =============================================================================

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

#include "engine/command/AIContext.h"
#include "engine/command/CommandParser.h"
#include "engine/command/CommandRegistry.h"
#include "engine/core/App.h"

// --------------------- 跨线程通信状态 ---------------------
static std::queue<std::string> g_cmdQueue;
static std::mutex              g_qmtx;
static std::atomic<bool>       g_done{false};

// 上次 :ctx / :diff 的统计(供 :budget 查看)
static int  g_lastExportTokens   = 0;
static int  g_lastExportMaxBudget = 0;
static bool g_lastExportTruncated = false;
static int  g_lastDiffTokens     = 0;

// --------------------- 控制台输出帮助 ---------------------
static void PrintBanner() {
    std::cout << "\n";
    std::cout << "==================================================\n";
    std::cout << " AIForge Chapter 03 - AI Command Protocol REPL\n";
    std::cout << "==================================================\n";
    std::cout << " Built-in commands:\n";
    std::cout << "   spawn <name> [at x,y,z]   destroy <name>\n";
    std::cout << "   list entities|commands    set <e>.<prop> <v>\n";
    std::cout << "   get <e>.<prop>            export context [<file>]\n";
    std::cout << "   help [<verb>]             quit\n";
    std::cout << "\n";
    std::cout << " Meta-commands (this demo only, prefix with ':'):\n";
    std::cout << "   :ctx [tokens=N]           export AIContext with budget\n";
    std::cout << "   :diff                     show diff since last export\n";
    std::cout << "   :budget                   show last export's token use\n";
    std::cout << "   :reset                    clear snapshot for diff\n";
    std::cout << "   :help                     list meta-commands\n";
    std::cout << "==================================================\n";
    std::cout << "\nTip: try 'spwan' (typo) to see 'did you mean' suggestion.\n";
    std::cout << "Tip: do :ctx, then 'spawn boss', then :diff to see savings.\n\n";
}

static void PrintPrompt() {
    std::cout << "AIForge> " << std::flush;
}

// --------------------- 后台线程:无脑读 stdin ---------------------
static void ReplWorker() {
    std::string line;
    while (!g_done.load()) {
        if (!std::getline(std::cin, line)) break;  // EOF / 关闭管道
        {
            std::lock_guard<std::mutex> lk(g_qmtx);
            g_cmdQueue.push(line);
        }
    }
}

// --------------------- meta 命令处理 ---------------------
// 返回 true 表示已消费(meta 命令),false 表示普通命令应交给 App::Execute
static bool HandleMetaCommand(AIForge::App& app, const std::string& cmd) {
    if (cmd.empty() || cmd[0] != ':') return false;

    auto parsed = AIForge::CommandParser::Parse(cmd.substr(1));
    const std::string& verb = parsed.verb;

    if (verb == "help") {
        std::cout
            << "meta-commands:\n"
            << "  :ctx [tokens=N]   export AIContext with budget (default 4000)\n"
            << "  :diff             diff since last export (cheap update)\n"
            << "  :budget           show last :ctx / :diff stats\n"
            << "  :reset            clear AIContext snapshot\n"
            << "  :help             this help\n";
        return true;
    }

    if (verb == "ctx") {
        AIForge::AIContext::ExportOptions opts;
        for (auto& tok : parsed.tokens) {
            auto eq = tok.find('=');
            if (eq == std::string::npos) continue;
            std::string k = tok.substr(0, eq);
            std::string v = tok.substr(eq + 1);
            if (k == "tokens" || k == "budget") {
                opts.maxTokens = std::atoi(v.c_str());
            }
        }
        auto r = AIForge::AIContext::Export(app, opts);
        g_lastExportTokens    = r.estimatedTokens;
        g_lastExportMaxBudget = opts.maxTokens;
        g_lastExportTruncated = r.truncated;
        std::cout << "[AIContext: " << r.estimatedTokens << " / "
                  << opts.maxTokens << " tokens, truncated="
                  << (r.truncated ? "true" : "false") << "]\n";
        std::cout << r.markdown;
        return true;
    }

    if (verb == "diff") {
        auto r = AIForge::AIContext::Diff(app);
        g_lastDiffTokens = r.estimatedTokens;
        std::cout << "[Diff: " << r.estimatedTokens << " tokens]\n";
        std::cout << r.markdown;
        return true;
    }

    if (verb == "budget") {
        std::cout << "Last :ctx export:\n";
        if (g_lastExportTokens == 0) {
            std::cout << "  (none yet — try ':ctx' first)\n";
        } else {
            std::cout << "  tokens used   : " << g_lastExportTokens << "\n";
            std::cout << "  budget        : " << g_lastExportMaxBudget << "\n";
            std::cout << "  truncated     : "
                      << (g_lastExportTruncated ? "yes" : "no") << "\n";
        }
        std::cout << "Last :diff:\n";
        if (g_lastDiffTokens == 0)
            std::cout << "  (none yet — try ':diff' after a ':ctx')\n";
        else {
            std::cout << "  tokens used   : " << g_lastDiffTokens;
            if (g_lastExportTokens > 0) {
                int pct = (g_lastDiffTokens * 100) / g_lastExportTokens;
                std::cout << "  (" << pct << "% of last :ctx — diff is "
                          << (100 - pct) << "% cheaper)";
            }
            std::cout << "\n";
        }
        return true;
    }

    if (verb == "reset") {
        AIForge::AIContext::ResetSnapshot();
        g_lastDiffTokens = 0;
        std::cout << "snapshot cleared.\n";
        return true;
    }

    std::cout << "unknown meta-command: '" << verb
              << "'. Try ':help'.\n";
    return true;
}

// --------------------- main ---------------------
int main(int /*argc*/, char* /*argv*/[]) {
    auto app = AIForge::App::Create();
    if (!app) {
        std::fprintf(stderr, "[Ch03] App::Create returned null\n");
        return 1;
    }
    AIForge::Window::Config wc;
    wc.title  = "AIForge Ch 03 - Command REPL (see console)";
    wc.width  = 640;
    wc.height = 360;
    wc.vsync  = true;
    app->SetWindowConfig(wc);

    if (!app->Init()) {
        std::fprintf(stderr, "[Ch03] App::Init failed\n");
        return 1;
    }
    app->SetClearColor(0.15f, 0.08f, 0.20f, 1.0f);   // 深紫
    app->SetShowFPSInTitle(false);

    PrintBanner();
    PrintPrompt();

    // 启动后台读取线程(detach,进程退出时 OS 回收)
    std::thread worker(ReplWorker);
    worker.detach();

    float titleAccum = 0.0f;
    std::string lastCmd;

    app->SetUpdateCallback([&](AIForge::App& a, float dt) {
        // 1) 从队列里把所有可用命令拉出来,在主线程执行(World 单线程)
        std::vector<std::string> cmds;
        {
            std::lock_guard<std::mutex> lk(g_qmtx);
            while (!g_cmdQueue.empty()) {
                cmds.push_back(g_cmdQueue.front());
                g_cmdQueue.pop();
            }
        }

        for (auto& raw : cmds) {
            // 修剪首尾空白
            size_t b = raw.find_first_not_of(" \t\r\n");
            size_t e = raw.find_last_not_of(" \t\r\n");
            std::string cmd = (b == std::string::npos) ? "" : raw.substr(b, e - b + 1);
            if (cmd.empty()) {
                PrintPrompt();
                continue;
            }
            lastCmd = cmd;

            // 退出命令
            if (cmd == "quit" || cmd == "exit") {
                std::cout << "quit requested.\n";
                g_done.store(true);
                a.RequestClose();
                break;
            }

            // meta 命令
            if (cmd[0] == ':') {
                HandleMetaCommand(a, cmd);
                PrintPrompt();
                continue;
            }

            // 普通命令 — 走 App::Execute,失败会自带 "did you mean" 提示
            auto r = a.GetCommands()->Execute(cmd);
            if (r.ok) {
                if (!r.output.empty()) std::cout << r.output << "\n";
            } else {
                std::cout << "[error] " << r.errMsg << "\n";
            }
            PrintPrompt();
        }

        // 2) 每 0.25s 刷一次窗口标题(让用户在屏上也能看到 World 状态)
        titleAccum += dt;
        if (titleAccum >= 0.25f) {
            titleAccum = 0.0f;
            char buf[256];
            const auto* w = a.GetWorld();
            std::snprintf(buf, sizeof(buf),
                "AIForge Ch 03 | entities=%d | last='%s' | %.0f FPS",
                w ? w->GetEntityCount() : 0,
                lastCmd.empty() ? "(none)" : lastCmd.c_str(),
                a.GetTime()->FPS());
            a.GetWindow()->SetTitle(buf);
        }
    });

    app->Run();
    g_done.store(true);
    app->Shutdown();
    std::cout << "\n[Ch03] clean exit. Bye!\n";
    // 注意:worker 线程可能还卡在 std::getline,我们已 detach,进程退出时 OS 回收。
    std::exit(0);  // 强制立即终止,避免 worker 阻塞导致看起来"卡住"
}
