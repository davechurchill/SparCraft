#pragma once

#include "../SparCraft.h"
#include "../Position.hpp"
#include "BWAPI.h"
#include "GUIGame.h"

#include <SFML/Graphics.hpp>

#include "../imgui/imgui.h"
#include "../imgui/imgui-SFML.h"
#include "../imgui/imgui_stdlib.h"

#include <string>
#include <unordered_map>

namespace SparCraft
{

class GUI
{
    int m_initialWidth;
    int m_initialHeight;
    bool m_isStarted;

    sf::RenderWindow m_window;
    sf::Clock m_deltaClock;

    GUIGame m_guiGame;

    std::unordered_map<int, sf::Texture> m_unitTextures;

    void handleEvents();
    void render();
    void loadTextures();
    void onStart();

    bool isStarted() const;

    static std::string GetTextureFileName(const BWAPI::UnitType & type);

public:

    GUI(int width, int height);
    ~GUI();

    int width() const;
    int height() const;

    void onFrame();
    void setCenter(int x, int y);
    void setGame(const Game & game);

    const Game & getGame() const;

    void drawUnitType(const BWAPI::UnitType & type, const Position & p, sf::RenderTarget & target) const;
    void drawUnitTypeIcon(const BWAPI::UnitType & type, const sf::Vector2f & topLeft, float iconSize, sf::RenderTarget & target) const;
    void drawLine(const Position & p1, const Position & p2, float thickness, const sf::Color & color, sf::RenderTarget & target) const;

    bool saveScreenshotBMP(const std::string & filename);

    sf::RenderWindow & window();
};

}

