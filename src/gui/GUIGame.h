#pragma once

#include "../SparCraft.h"

#include <SFML/Graphics.hpp>

namespace SparCraft
{

class GUI;

class GUIGame
{
    GUI & _gui;
    Game _game;

    double _previousDrawGameTimer;
    double _previousTurnTimer;

    GameState _initialState;

    std::vector<std::vector<std::string> > _params[2];
    std::vector<std::vector<std::string> > _results[2];

    bool _paused;
    bool _stepOneTurn;
    bool _renderWorld;
    bool _renderHPBars;

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
