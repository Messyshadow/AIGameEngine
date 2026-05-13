#include "CommandRegistry.h"

#include <algorithm>
#include <cstdio>
#include <vector>

namespace AIForge {

namespace {
// Levenshtein 编辑距离 — 经典 DP,O(m*n) 空间和时间
int Levenshtein(const std::string& a, const std::string& b) {
    const int m = static_cast<int>(a.size());
    const int n = static_cast<int>(b.size());
    if (m == 0) return n;
    if (n == 0) return m;
    std::vector<int> prev(n + 1), cur(n + 1);
    for (int j = 0; j <= n; ++j) prev[j] = j;
    for (int i = 1; i <= m; ++i) {
        cur[0] = i;
        for (int j = 1; j <= n; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            cur[j] = std::min({prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + cost});
        }
        std::swap(prev, cur);
    }
    return prev[n];
}
}  // namespace

CommandRegistry::CommandRegistry() = default;
CommandRegistry::~CommandRegistry() = default;

void CommandRegistry::Register(const std::string& verb,
                               const std::string& description,
                               const std::string& usage,
                               CommandHandler handler) {
    std::string key = CommandParser::ToLower(verb);
    if (m_entries.count(key)) {
        std::fprintf(stderr,
                     "[CommandRegistry] WARN: '%s' re-registered (overriding)\n",
                     key.c_str());
    }
    m_entries[key] = Entry{key, description, usage, std::move(handler)};
}

CommandResult CommandRegistry::Execute(const std::string& cmd) {
    auto parsed = CommandParser::Parse(cmd);
    if (parsed.verb.empty()) return CommandResult::Fail("empty command");

    auto it = m_entries.find(parsed.verb);
    if (it == m_entries.end()) {
        std::string msg = "unknown command: '" + parsed.verb + "'";
        // AI 友好:找最近的命令给建议(Levenshtein ≤ 2)
        auto nearest = FindNearest(parsed.verb);
        if (!nearest.empty()) {
            msg += "; did you mean: '" + nearest + "'?";
        }
        msg += " Try 'help' for the full command list.";
        return CommandResult::Fail(msg);
    }
    return it->second.handler(parsed);
}

std::string CommandRegistry::FindNearest(const std::string& verb,
                                         int maxDist) const {
    std::string key = CommandParser::ToLower(verb);
    std::string bestVerb;
    int bestDist = maxDist + 1;
    for (auto& kv : m_entries) {
        int d = Levenshtein(key, kv.first);
        if (d < bestDist) {
            bestDist = d;
            bestVerb = kv.first;
        }
    }
    if (bestDist > maxDist) return "";
    return bestVerb;
}

std::vector<const CommandRegistry::Entry*> CommandRegistry::List() const {
    std::vector<const Entry*> out;
    out.reserve(m_entries.size());
    for (auto& kv : m_entries) out.push_back(&kv.second);
    std::sort(out.begin(), out.end(),
              [](const Entry* a, const Entry* b) { return a->verb < b->verb; });
    return out;
}

const CommandRegistry::Entry* CommandRegistry::Find(const std::string& verb) const {
    auto key = CommandParser::ToLower(verb);
    auto it = m_entries.find(key);
    return it == m_entries.end() ? nullptr : &it->second;
}

}  // namespace AIForge
