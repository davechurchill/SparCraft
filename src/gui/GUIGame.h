#pragma once

#include "../SparCraft.h"

#include <SFML/Graphics.hpp>

namespace SparCraft
{

class GUI;

class GUIGame
{
    GUI & m_gui;
    Game m_game;

    double m_previousDrawGameTimer;
    double m_previousTurnTimer;

    GameState m_initialState;

    std::vector<std::vector<std::string> > m_params[2];
    std::vector<std::vector<std::string> > m_results[2];

    bool m_paused;
    bool m_stepOneTurn;
    bool m_renderWorld;
    bool m_renderHPBars;

    void drawGame(sf::RenderTarget & target);
    void drawHPBars(sf::RenderTarget & target);
    void drawUnit(const Unit & unit, sf::RenderTarget & target);

    void drawControlsWindow();
    void drawUnitsWindow();

    void setResults(const size_t & player, const std::vector<std::vector<std::string> > & r);
    void setParams(const size_t & player, const std::vector<std::vector<std::string> > & p);

public:

    explicit GUIGame(GUI & gui);

    const Game & getGame() const;
    void setGame(const Game & g);

    void onFrame(sf::RenderTarget & target);
};

}

