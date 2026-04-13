#include "GUI.h"

#include <algorithm>
#include <filesystem>
#include <cstdlib>

using namespace SparCraft;

namespace
{
    const sf::Color BackgroundColor(18, 18, 22);
}

GUI::GUI(int width, int height)
    : m_initialWidth(width)
    , m_initialHeight(height)
    , m_isStarted(false)
    , m_guiGame(*this)
{
    onStart();
}

GUI::~GUI()
{
    if (m_isStarted)
    {
        ImGui::SFML::Shutdown();
    }
}

bool GUI::isStarted() const
{
    return m_isStarted;
}

void GUI::onStart()
{
    if (m_isStarted)
    {
        return;
    }

    m_window.create(sf::VideoMode({ static_cast<unsigned>(m_initialWidth), static_cast<unsigned>(m_initialHeight) }), "SparCraft - SFML3 / ImGui");
    m_window.setFramerateLimit(60);
    m_window.setKeyRepeatEnabled(false);

    if (!ImGui::SFML::Init(m_window))
    {
        System::FatalError("Failed to initialize ImGui-SFML");
    }

    //ImGui::GetStyle().ScaleAllSizes(1.5f);
    //ImGui::GetIO().FontGlobalScale = 1.5f;

    loadTextures();

    m_isStarted = true;
}

void GUI::handleEvents()
{
    while (auto event = m_window.pollEvent())
    {
        ImGui::SFML::ProcessEvent(m_window, *event);

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
    m_window.clear(BackgroundColor);
    m_guiGame.onFrame(m_window);
    ImGui::SFML::Render(m_window);
    m_window.display();
}

void GUI::onFrame()
{
    SPARCRAFT_ASSERT(isStarted(), "Must initialize GUI before calling onFrame()");

    handleEvents();
    ImGui::SFML::Update(m_window, m_deltaClock.restart());
    render();
}

void GUI::loadTextures()
{
    const std::string imageDir = "assets/images/";

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
            m_unitTextures.emplace(type.getID(), std::move(texture));
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
    const auto texture = m_unitTextures.find(type.getID());
    if (texture != m_unitTextures.end())
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

void GUI::drawUnitTypeIcon(const BWAPI::UnitType & type, const sf::Vector2f & topLeft, float iconSize, sf::RenderTarget & target) const
{
    if (iconSize <= 0.0f)
    {
        return;
    }

    sf::RectangleShape back(sf::Vector2f(iconSize, iconSize));
    back.setPosition(topLeft);
    back.setFillColor(sf::Color(22, 22, 22, 240));
    target.draw(back);

    const auto texture = m_unitTextures.find(type.getID());
    if (texture != m_unitTextures.end())
    {
        const auto textureSize = texture->second.getSize();
        if (textureSize.x > 0 && textureSize.y > 0)
        {
            sf::Sprite sprite(texture->second);

            const float sx = iconSize / static_cast<float>(textureSize.x);
            const float sy = iconSize / static_cast<float>(textureSize.y);
            const float scale = std::min(sx, sy);

            sprite.setScale({ scale, scale });

            const float drawnW = static_cast<float>(textureSize.x) * scale;
            const float drawnH = static_cast<float>(textureSize.y) * scale;
            sprite.setPosition({
                topLeft.x + (iconSize - drawnW) * 0.5f,
                topLeft.y + (iconSize - drawnH) * 0.5f
            });
            target.draw(sprite);
            return;
        }
    }

    sf::CircleShape marker(iconSize * 0.35f, 12);
    marker.setOrigin({ iconSize * 0.35f, iconSize * 0.35f });
    marker.setPosition({ topLeft.x + iconSize * 0.5f, topLeft.y + iconSize * 0.5f });
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
    return static_cast<int>(m_window.getSize().x);
}

int GUI::height() const
{
    return static_cast<int>(m_window.getSize().y);
}

void GUI::setCenter(int x, int y)
{
    sf::View view = m_window.getView();
    view.setCenter({ static_cast<float>(x), static_cast<float>(y) });
    m_window.setView(view);
}

void GUI::setGame(const Game & game)
{
    m_guiGame.setGame(game);
}

const Game & GUI::getGame() const
{
    return m_guiGame.getGame();
}

bool GUI::saveScreenshotBMP(const std::string & filename)
{
    (void)filename;
    return false;
}

sf::RenderWindow & GUI::window()
{
    return m_window;
}

