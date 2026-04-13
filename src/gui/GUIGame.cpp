#include "GUIGame.h"

#include "GUI.h"

#include "../imgui/imgui.h"
#include <algorithm>

using namespace SparCraft;

namespace
{
    const sf::Color PlayerColors[2] = {
        sf::Color(220, 70, 70),
        sf::Color(70, 200, 90)
    };

    const sf::Color PlayerColorsDark[2] = {
        sf::Color(140, 40, 40),
        sf::Color(40, 120, 60)
    };

    ImVec4 ToImVec4(const sf::Color & c)
    {
        return ImVec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
    }

    void DrawNameValueTable(const char * id, const std::vector<std::vector<std::string> > & values)
    {
        if (values.size() != 2 || values[0].empty())
        {
            ImGui::TextUnformatted("No data");
            return;
        }

        if (ImGui::BeginTable(id, 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();

            const size_t rows = std::min(values[0].size(), values[1].size());
            for (size_t i = 0; i < rows; ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(values[0][i].c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(values[1][i].c_str());
            }

            ImGui::EndTable();
        }
    }
}

GUIGame::GUIGame(GUI & gui)
    : _gui(gui)
    , _game(GameState(), 0)
    , _previousDrawGameTimer(0.0)
    , _previousTurnTimer(0.0)
    , _paused(false)
    , _stepOneTurn(false)
    , _renderWorld(true)
    , _renderHPBars(true)
{
}

void GUIGame::onFrame(sf::RenderTarget & target)
{
    if ((!_paused || _stepOneTurn) && !_game.gameOver())
    {
        Timer turnTimer;
        turnTimer.start();

        _game.playNextTurn();

        _previousTurnTimer = turnTimer.getElapsedTimeInMilliSec();
        _stepOneTurn = false;

        for (size_t p = 0; p < 2; ++p)
        {
            Player_UCT * uct = dynamic_cast<Player_UCT *>(_game.getPlayer(p).get());
            Player_AlphaBeta * ab = dynamic_cast<Player_AlphaBeta *>(_game.getPlayer(p).get());

            if (uct)
            {
                setParams(p, uct->getParams().getDescription());
                setResults(p, uct->getResults().getDescription());
            }

            if (ab)
            {
                setParams(p, ab->getParams().getDescription());
                setResults(p, ab->results().getDescription());
            }
        }
    }

    Timer drawTimer;
    drawTimer.start();

    if (_renderWorld)
    {
        drawGame(target);
    }

    if (_renderHPBars)
    {
        drawHPBars(target);
    }

    _previousDrawGameTimer = drawTimer.getElapsedTimeInMilliSec();

    drawControlsWindow();
    drawPlayerDataWindow();
    drawUnitsWindow();
}

void GUIGame::drawGame(sf::RenderTarget & target)
{
    const GameState & state = _game.getState();

    for (size_t p = 0; p < 2; ++p)
    {
        for (size_t u = 0; u < state.numUnits(p); ++u)
        {
            drawUnit(state.getUnit(p, u), target);
        }
    }
}

void GUIGame::drawUnit(const Unit & unit, sf::RenderTarget & target)
{
    if (!unit.isAlive())
    {
        return;
    }

    const GameState & state = _game.getState();
    const BWAPI::UnitType & type = unit.type();
    const Position pos(unit.currentPosition(state.getTime()));

    _gui.drawUnitType(type, pos, target);

    const int x0 = pos.x() - type.dimensionUp();
    const int x1 = pos.x() + type.dimensionDown();
    const int y0 = pos.y() - type.dimensionUp();
    const int y1 = pos.y() + type.dimensionDown();

    const float healthRatio = unit.maxHP() > 0 ? static_cast<float>(unit.currentHP()) / static_cast<float>(unit.maxHP()) : 0.0f;
    const float barWidth = static_cast<float>(x1 - x0);
    const float xx = static_cast<float>(pos.x() - (x1 - x0) / 2);
    const float yy = static_cast<float>(pos.y() - 5 - (y1 - y0) / 2);

    sf::RectangleShape hpBack(sf::Vector2f(barWidth, 4.0f));
    hpBack.setPosition(sf::Vector2f(xx, yy));
    hpBack.setFillColor(sf::Color(30, 30, 30, 220));
    target.draw(hpBack);

    sf::RectangleShape hpFill(sf::Vector2f(barWidth * healthRatio, 4.0f));
    hpFill.setPosition(sf::Vector2f(xx, yy));
    hpFill.setFillColor(PlayerColors[unit.player()]);
    target.draw(hpFill);

    const Action & action = unit.previousAction();

    if (action.type() == ActionTypes::MOVE)
    {
        _gui.drawLine(pos, unit.pos(), 1.0f, PlayerColors[unit.player()], target);
    }
    else if (action.type() == ActionTypes::ATTACK)
    {
        const size_t enemyPlayer = state.getEnemy(unit.player());
        if (action.index() < state.numUnits(enemyPlayer))
        {
            const Unit & targetUnit = state.getUnit(enemyPlayer, action.index());
            const Position targetPos(targetUnit.currentPosition(state.getTime()));
            _gui.drawLine(pos, targetPos, 1.0f, PlayerColors[unit.player()], target);
        }
    }
}

void GUIGame::drawHPBars(sf::RenderTarget & target)
{
    const GameState & state = _game.getState();

    for (size_t p = 0; p < Constants::Num_Players; ++p)
    {
        for (size_t u = 0; u < _initialState.numUnits(p); ++u)
        {
            const Unit & unit = state.getUnitDirect(p, u);

            const float hpPercent = unit.maxHP() > 0 ? static_cast<float>(unit.currentHP()) / static_cast<float>(unit.maxHP()) : 0.0f;
            const float w = 150.0f;
            const float h = 10.0f;
            const float cw = w * hpPercent;
            const float xx = 1000.0f + (170.0f * p) - w / 2.0f;
            const float yy = 40.0f + (h + 2.0f) * u;

            sf::RectangleShape back(sf::Vector2f(w, h));
            back.setPosition(sf::Vector2f(xx, yy));
            back.setFillColor(sf::Color(22, 22, 22, 240));
            target.draw(back);

            if (unit.isAlive())
            {
                sf::RectangleShape fill(sf::Vector2f(cw, h));
                fill.setPosition(sf::Vector2f(xx, yy));
                fill.setFillColor(PlayerColors[p]);
                target.draw(fill);
            }
        }
    }
}

void GUIGame::drawControlsWindow()
{
    ImGui::Begin("SparCraft Controls");

    ImGui::Checkbox("Pause Simulation", &_paused);
    ImGui::Checkbox("Render World", &_renderWorld);
    ImGui::Checkbox("Render HP Bars", &_renderHPBars);

    if (ImGui::Button("Step One Turn"))
    {
        _stepOneTurn = true;
    }

    ImGui::Separator();
    ImGui::Text("Rounds: %d", _game.getRounds());
    ImGui::Text("Turn Time: %.2f ms", _previousTurnTimer);
    ImGui::Text("Draw Time: %.2f ms", _previousDrawGameTimer);
    ImGui::Text("State Eval (P1, LTD2): %d", _game.getState().eval(Players::Player_One, EvaluationMethods::LTD2).val());

    if (_game.gameOver())
    {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Game Over");
    }

    ImGui::End();
}

void GUIGame::drawPlayerDataWindow()
{
    ImGui::Begin("Search Data");

    if (ImGui::BeginTabBar("PlayersTab"))
    {
        for (size_t p = 0; p < 2; ++p)
        {
            const std::string label = std::string("Player ") + std::to_string(p + 1);
            if (ImGui::BeginTabItem(label.c_str()))
            {
                ImGui::TextColored(ToImVec4(PlayerColors[p]), "Settings");
                DrawNameValueTable((std::string("settings") + std::to_string(p)).c_str(), _params[p]);

                ImGui::Separator();
                ImGui::TextColored(ToImVec4(PlayerColorsDark[p]), "Search Results");
                DrawNameValueTable((std::string("results") + std::to_string(p)).c_str(), _results[p]);

                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void GUIGame::drawUnitsWindow()
{
    ImGui::Begin("Units");

    const GameState & state = _game.getState();

    for (size_t p = 0; p < 2; ++p)
    {
        const std::string header = std::string("Player ") + std::to_string(p + 1);
        if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            const std::string tableId = std::string("units-table-") + std::to_string(p);
            if (ImGui::BeginTable(tableId.c_str(), 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("ID");
                ImGui::TableSetupColumn("Type");
                ImGui::TableSetupColumn("HP");
                ImGui::TableSetupColumn("X");
                ImGui::TableSetupColumn("Y");
                ImGui::TableSetupColumn("Alive");
                ImGui::TableHeadersRow();

                for (size_t u = 0; u < state.numUnits(p); ++u)
                {
                    const Unit & unit = state.getUnit(p, u);
                    const Position pos(unit.currentPosition(state.getTime()));

                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", static_cast<int>(unit.ID()));

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(unit.name().c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d / %d", unit.currentHP(), unit.maxHP());

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", pos.x());

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%d", pos.y());

                    ImGui::TableSetColumnIndex(5);
                    ImGui::TextUnformatted(unit.isAlive() ? "yes" : "no");
                }

                ImGui::EndTable();
            }
        }
    }

    ImGui::End();
}

void GUIGame::setGame(const Game & g)
{
    _game = g;
    _initialState = g.getState();

    _paused = false;
    _stepOneTurn = false;

    for (size_t p = 0; p < 2; ++p)
    {
        _params[p].clear();
        _results[p].clear();
    }
}

const Game & GUIGame::getGame() const
{
    return _game;
}

void GUIGame::setResults(const size_t & player, const std::vector<std::vector<std::string> > & r)
{
    _results[player] = r;
}

void GUIGame::setParams(const size_t & player, const std::vector<std::vector<std::string> > & p)
{
    _params[player] = p;
}
