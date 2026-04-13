#include "GUI.h"

#include <filesystem>
#include <cstdlib>

using namespace SparCraft;

namespace
{
    const sf::Color BackgroundColor(18, 18, 22);
}

GUI::GUI(int width, int height)
    : _initialWidth(width)
    , _initialHeight(height)
    , _isStarted(false)
    , _guiGame(*this)
{
    onStart();
}

GUI::~GUI()
{
    if (_isStarted)
    {
        ImGui::SFML::Shutdown();
    }
}

bool GUI::isStarted() const
{
    return _isStarted;
}

void GUI::onStart()
{
    if (_isStarted)
    {
        return;
    }

    _window.create(sf::VideoMode({ static_cast<unsigned>(_initialWidth), static_cast<unsigned>(_initialHeight) }), "SparCraft - SFML3 / ImGui");
    _window.setFramerateLimit(60);
    _window.setKeyRepeatEnabled(false);

    if (!ImGui::SFML::Init(_window))
    {
        System::FatalError("Failed to initialize ImGui-SFML");
    }

    ImGui::GetStyle().ScaleAllSizes(1.5f);
    ImGui::GetIO().FontGlobalScale = 1.5f;

    loadTextures();

    _isStarted = true;
}

void GUI::handleEvents()
{
    while (auto event = _window.pollEvent())
    {
        ImGui::SFML::ProcessEvent(_window, *event);

        if (event->is<sf::Event::Closed>())
        {
            std::exit(0);
        }

        if (const auto * keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
            {
                std::exit(0);
            }
        }
    }
}

void GUI::render()
{
    _window.clear(BackgroundColor);
    _guiGame.onFrame(_window);
    ImGui::SFML::Render(_window);
    _window.display();
}

void GUI::onFrame()
{
    SPARCRAFT_ASSERT(isStarted(), "Must initialize GUI before calling onFrame()");

    handleEvents();
    ImGui::SFML::Update(_window, _deltaClock.restart());
    render();
}

void GUI::loadTextures()
{
    const std::string imageDir = "asset/images/";

    for (const BWAPI::UnitType & type : BWAPI::UnitTypes::allUnitTypes())
    {
        const std::string fileName = imageDir + GetTextureFileName(type);

        if (!std::filesystem::exists(fileName))
        {
            continue;
        }

        sf::Texture texture;
        if (texture.loadFromFile(fileName))
        {
            _unitTextures.emplace(type.getID(), std::move(texture));
        }
    }
}

std::string GUI::GetTextureFileName(const BWAPI::UnitType & type)
{
    std::string filename = "units/" + type.getName() + ".png";

    for (char & c : filename)
    {
        if (c == ' ')
        {
            c = '_';
        }
    }

    return filename;
}

void GUI::drawUnitType(const BWAPI::UnitType & type, const Position & p, sf::RenderTarget & target) const
{
    const auto texture = _unitTextures.find(type.getID());
    if (texture != _unitTextures.end())
    {
        sf::Sprite sprite(texture->second);
        const auto size = texture->second.getSize();
        sprite.setOrigin({ static_cast<float>(size.x) * 0.5f, static_cast<float>(size.y) * 0.5f });
        sprite.setPosition({ static_cast<float>(p.x()), static_cast<float>(p.y()) });
        target.draw(sprite);
        return;
    }

    sf::CircleShape marker(8.0f, 12);
    marker.setOrigin({ 8.0f, 8.0f });
    marker.setPosition({ static_cast<float>(p.x()), static_cast<float>(p.y()) });
    marker.setFillColor(sf::Color(60, 60, 60));
    marker.setOutlineColor(sf::Color::White);
    marker.setOutlineThickness(1.0f);
    target.draw(marker);
}

void GUI::drawLine(const Position & p1, const Position & p2, float thickness, const sf::Color & color, sf::RenderTarget & target) const
{
    (void)thickness;

    sf::Vertex vertices[2] = {
        sf::Vertex({ static_cast<float>(p1.x()), static_cast<float>(p1.y()) }, color),
        sf::Vertex({ static_cast<float>(p2.x()), static_cast<float>(p2.y()) }, color)
    };

    target.draw(vertices, 2, sf::PrimitiveType::Lines);
}

int GUI::width() const
{
    return static_cast<int>(_window.getSize().x);
}

int GUI::height() const
{
    return static_cast<int>(_window.getSize().y);
}

void GUI::setCenter(int x, int y)
{
    sf::View view = _window.getView();
    view.setCenter({ static_cast<float>(x), static_cast<float>(y) });
    _window.setView(view);
}

void GUI::setGame(const Game & game)
{
    _guiGame.setGame(game);
}

const Game & GUI::getGame() const
{
    return _guiGame.getGame();
}

bool GUI::saveScreenshotBMP(const std::string & filename)
{
    (void)filename;
    return false;
}

sf::RenderWindow & GUI::window()
{
    return _window;
}
