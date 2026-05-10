#pragma once

#include <string>
#include <vector>

namespace AIForge {

/// @ai_summary 解析后的命令：动词 + tokens（按空格分隔）+ 原始文本。
/// @ai_summary 如 "spawn player at 0,0,0" → verb="spawn", tokens=["player","at","0,0,0"]。
struct ParsedCommand {
    std::string verb;
    std::vector<std::string> tokens;
    std::string raw;

    /// @ai_summary 在 tokens 中查找关键字（如 "at"）的位置，返回 -1 表示未找到。
    int FindKeyword(const std::string& keyword) const;
};

/// @ai_summary 文本命令解析器（无状态、纯静态方法）。
/// @ai_summary 规则：按空白拆分；遇到引号 "..." 会作为单 token；不区分大小写动词。
/// @ai_example
///   auto cmd = CommandParser::Parse("spawn zombie at 1,2,3");
///   // cmd.verb == "spawn"
///   // cmd.tokens == {"zombie","at","1,2,3"}
/// @ai_related CommandRegistry, App::Execute
class CommandParser {
public:
    static ParsedCommand Parse(const std::string& input);

    /// @ai_summary 工具函数：转小写
    static std::string ToLower(std::string s);
};

}  // namespace AIForge
