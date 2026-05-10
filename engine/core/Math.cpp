#include "Math.h"

#include <cstdlib>
#include <cstring>

namespace AIForge {

bool ParseVec3(const std::string& text, Vec3& out) {
    out = Vec3{};
    float values[3] = {0, 0, 0};
    int idx = 0;
    const char* p = text.c_str();
    while (*p && idx < 3) {
        while (*p == ' ' || *p == '\t') ++p;
        char* end = nullptr;
        float v = std::strtof(p, &end);
        if (end == p) return false;
        values[idx++] = v;
        p = end;
        while (*p == ' ' || *p == '\t') ++p;
        if (*p == ',') ++p;
    }
    if (idx != 3) return false;
    out = Vec3{values[0], values[1], values[2]};
    return true;
}

}  // namespace AIForge
