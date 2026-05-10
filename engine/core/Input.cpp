#include "Input.h"

namespace AIForge {

Input::Input() = default;

void Input::NewFrame() {
    m_pressed.clear();
    m_released.clear();
    m_mousePressed.clear();
    m_mouseReleased.clear();
    m_mouseDX = 0;
    m_mouseDY = 0;
    m_mouseWheel = 0.0f;
}

void Input::OnKeyDown(int scancode) {
    if (m_down.insert(scancode).second) {
        m_pressed.insert(scancode);
    }
}

void Input::OnKeyUp(int scancode) {
    if (m_down.erase(scancode) > 0) {
        m_released.insert(scancode);
    }
}

void Input::OnMouseMotion(int x, int y, int dx, int dy) {
    m_mouseX = x;
    m_mouseY = y;
    m_mouseDX += dx;
    m_mouseDY += dy;
}

void Input::OnMouseButtonDown(int btn) {
    if (m_mouseDown.insert(btn).second) {
        m_mousePressed.insert(btn);
    }
}

void Input::OnMouseButtonUp(int btn) {
    if (m_mouseDown.erase(btn) > 0) {
        m_mouseReleased.insert(btn);
    }
}

void Input::OnMouseWheel(float yScroll) { m_mouseWheel += yScroll; }

bool Input::IsKeyDown(int sc) const { return m_down.count(sc) > 0; }
bool Input::IsKeyPressed(int sc) const { return m_pressed.count(sc) > 0; }
bool Input::IsKeyReleased(int sc) const { return m_released.count(sc) > 0; }

bool Input::IsMouseDown(int b) const { return m_mouseDown.count(b) > 0; }
bool Input::IsMousePressed(int b) const { return m_mousePressed.count(b) > 0; }
bool Input::IsMouseReleased(int b) const { return m_mouseReleased.count(b) > 0; }

}  // namespace AIForge
