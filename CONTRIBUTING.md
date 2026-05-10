# Contributing to AIForge Engine

## 项目愿景

AIForge是全球首个AI-First游戏引擎框架。我们欢迎所有开发者贡献代码、文档、模板和创意。

## 贡献规范

### 代码规范

1. C++17标准
2. 所有公开API必须有 `@ai_summary` `@ai_params` `@ai_example` 注释
3. 使用智能指针管理内存，禁止裸new/delete
4. 错误处理：返回nullptr/false + 打印可读错误信息，不抛异常
5. 文件命名：PascalCase（如 CommandParser.h）
6. 类命名：PascalCase
7. 函数命名：PascalCase
8. 变量命名：camelCase
9. 成员变量：m_ 前缀

### AI模板贡献

在 templates/ 目录下添加新的JSON任务模板，让AI能快速生成游戏内容。

### 提交规范

```
feat: 新功能
fix: 修复bug
docs: 文档更新
refactor: 重构
test: 测试
```

## 开发环境

- Windows 10/11
- Visual Studio 2019/2022
- CMake 3.17+
- C++17

## 联系

GitHub Issues: https://github.com/Messyshadow/AIGameEngine/issues
