#include "GUITools.h"

namespace SparCraft
{
namespace GUITools
{
    void DrawLine(const Position & p1, const Position & p2, float thickness, const GLfloat * rgba)
    {
        (void)p1;
        (void)p2;
        (void)thickness;
        (void)rgba;
    }

    void DrawString(const Position & p, const std::string & text, const GLfloat * rgba)
    {
        (void)p;
        (void)text;
        (void)rgba;
    }

    void DrawChar(const Position & tl, const Position & br, char ch, const GLfloat * rgba)
    {
        (void)tl;
        (void)br;
        (void)ch;
        (void)rgba;
    }

    void DrawCircle(const Position & p, float r, int numSegments)
    {
        (void)p;
        (void)r;
        (void)numSegments;
    }

    void DrawTexturedRect(const Position & tl, const Position & br, int textureID, const GLfloat * rgba)
    {
        (void)tl;
        (void)br;
        (void)textureID;
        (void)rgba;
    }

    void DrawRect(const Position & tl, const Position & br, const GLfloat * rgba)
    {
        (void)tl;
        (void)br;
        (void)rgba;
    }

    void DrawRectGradient(const Position & tl, const Position & br, const GLfloat * rgbaLeft, const GLfloat * rgbaRight)
    {
        (void)tl;
        (void)br;
        (void)rgbaLeft;
        (void)rgbaRight;
    }
}
}
