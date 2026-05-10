#include "CommandRegistry.h"

#include <algorithm>
#include <cstdio>

namespace AIForge {

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
        return CommandResult::Fail("unknown command: '" + parsed.verb +
                                   "'. Try 'help'.");
    }
    return it->second.handler(parsed);
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
