#pragma once

#include <cstdint>
#include <unordered_set>

namespace AIForge {

/// @ai_summary 键码（SDL3 SDL_Scancode 透传，封装为简单 enum 方便 AI 使用）。
/// @ai_summary 命名遵循"动作含义而非物理布局"，如 K_ACTION = 空格。
/// @ai_summary 完整列表请用 SDL_Scancode（SDL_SCANCODE_*）；这里只列常用。
enum Key : int {
    K_UNKNOWN = 0,
    K_ESC = 41,        // SDL_SCANCODE_ESCAPE
    K_SPACE = 44,      // SDL_SCANCODE_SPACE
    K_ENTER = 40,      // SDL_SCANCODE_RETURN
    K_TAB = 43,        // SDL_SCANCODE_TAB
    K_BACKSPACE = 42,  // SDL_SCANCODE_BACKSPACE
    K_LSHIFT = 225,    // SDL_SCANCODE_LSHIFT
    K_LCTRL = 224,     // SDL_SCANCODE_LCTRL
    K_LALT = 226,      // SDL_SCANCODE_LALT

    K_W = 26,
    K_A = 4,
    K_S = 22,
    K_D = 7,
    K_Q = 20,
    K_E = 8,
    K_F = 9,
    K_R = 21,

    K_UP = 82,
    K_DOWN = 81,
    K_LEFT = 80,
    K_RIGHT = 79,

    K_1 = 30, K_2 = 31, K_3 = 32, K_4 = 33, K_5 = 34,
    K_6 = 35, K_7 = 36, K_8 = 37, K_9 = 38, K_0 = 39,

    K_F1 = 58, K_F2 = 59, K_F3 = 60, K_F4 = 61, K_F5 = 62,
    K_F6 = 63, K_F7 = 64, K_F8 = 65, K_F9 = 66, K_F10 = 67,
    K_F11 = 68, K_F12 = 69,
};

enum MouseButton : int {
    MB_LEFT = 1,
    MB_MIDDLE = 2,
    MB_RIGHT = 3,
};

/// @ai_summary 输入系统：每帧由 App 投递 SDL 事件，提供"按下/按住/释放"三态查询。
/// @ai_summary "Pressed" = 这一帧刚按下；"Down" = 此刻处于按下状态；"Released" = 这一帧刚松开。
/// @ai_example
///   if (input->IsKeyPressed(K_SPACE)) jump();
///   if (input->IsKeyDown(K_W)) moveForward(dt);
/// @ai_related App, Window
class Input {
public:
    Input();

    /// @ai_summary 每帧开始前调用：清掉上一帧的"刚按下/刚松开"集合。Down 集合保持。
    void NewFrame();

    void OnKeyDown(int scancode);
    void OnKeyUp(int scancode);
    void OnMouseMotion(int x, int y, int dx, int dy);
    void OnMouseButtonDown(int btn);
    void OnMouseButtonUp(int btn);
    void OnMouseWheel(float yScroll);

    /// @ai_summary 当前是否处于按下状态
    bool IsKeyDown(int scancode) const;

    /// @ai_summary 这一帧刚按下（边缘触发）
    bool IsKeyPressed(int scancode) const;

    /// @ai_summary 这一帧刚松开
    bool IsKeyReleased(int scancode) const;

    bool IsMouseDown(int btn) const;
    bool IsMousePressed(int btn) const;
    bool IsMouseReleased(int btn) const;

    int MouseX() const { return m_mouseX; }
    int MouseY() const { return m_mouseY; }
    int MouseDX() const { return m_mouseDX; }
    int MouseDY() const { return m_mouseDY; }
    float MouseWheel() const { return m_mouseWheel; }

private:
    std::unordered_set<int> m_down;
    std::unordered_set<int> m_pressed;
    std::unordered_set<int> m_released;
    std::unordered_set<int> m_mouseDown;
    std::unordered_set<int> m_mousePressed;
    std::unordered_set<int> m_mouseReleased;

    int m_mouseX = 0, m_mouseY = 0;
    int m_mouseDX = 0, m_mouseDY = 0;
    float m_mouseWheel = 0.0f;
};

}  // namespace AIForge
