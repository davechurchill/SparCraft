#pragma once

#include "../Position.hpp"

#include <string>

namespace SparCraft
{
namespace GUITools
{
    using GLfloat = float;

    void DrawLine(const Position & p1, const Position & p2, float thickness, const GLfloat * rgba);
    void DrawString(const Position & p, const std::string & text, const GLfloat * rgba);
    void DrawChar(const Position & tl, const Position & br, char ch, const GLfloat * rgba);
    void DrawCircle(const Position & p, float r, int numSegments);
    void DrawTexturedRect(const Position & tl, const Position & br, int textureID, const GLfloat * rgba);
    void DrawRect(const Position & tl, const Position & br, const GLfloat * rgba);
    void DrawRectGradient(const Position & tl, const Position & br, const GLfloat * rgbaLeft, const GLfloat * rgbaRight);
}
}
