#include "Game.h"
#include "Vec2.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

Game::Game(const std::string & config)
    : m_text(m_font, "Default", 24)
{ 
    m_window.setFramerateLimit(60);
    init(config);
}

void Game::init(const std::string & path)
{
    std::ifstream fin(path);
    std::string token;
    int w, h, fps, fs, fontSize, r, g, b;
    while (fin.good())
    {
        // first string in the line defines the shape type
        fin >> token;
        if (token == "Player")
        {
            fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S;
            fin >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB;
            fin >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
        }
        else if (token == "Enemy")
        {
            fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX;
            fin >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT;
            fin >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
        }
        else if (token == "Bullet")
        {
            fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S;
            fin >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB;
            fin >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
        }
        else if (token == "Font")
        {
            fin >> token >> fontSize >> r >> g >> b;
            if (!m_font.openFromFile(token))
            {
                std::cerr << "Could not load font\n";
            }
            m_text.setFont(m_font);
            m_text.setPosition({ 10, 5 });
            m_text.setCharacterSize(fontSize);
        }
        else if (token == "Window")
        {
            fin >> w >> h >> fps >> fs;
            m_window.create(sf::VideoMode({uint32_t(w), uint32_t(h) }), "Game");
            m_window.setFramerateLimit(fps);
        }
        else
        {
            std::cerr << "Unknown shape\n";
        }
    }

    if (!ImGui::SFML::Init(m_window)) {}
    
    // scale the imgui ui and text size by 2
    ImGui::GetStyle().ScaleAllSizes(2.0f);
    ImGui::GetIO().FontGlobalScale = 2.0f;

    spawnPlayer();
}

void Game::run()
{
    while (true)
    {
        m_entities.update();
            
        ImGui::SFML::Update(m_window, m_deltaClock.restart());

        sUserInput();

        if (!m_paused)
        {
            if (m_doSpawning) { sEnemySpawner(); }
            if (m_doMovement) { sMovement(); }
            if (m_doLifespan) { sLifespan(); }
            if (m_doCollision) { sCollision(); }
            m_currentFrame++;
        }

        if (m_doGUI) { sGUI(); }
        if (m_doRender) { sRender(); }
    }
}

void Game::setPaused(bool paused)
{
    m_paused = paused;
}

// respawn the player in the middle of the screen
void Game::spawnPlayer()
{
    auto player = m_entities.addEntity("player");

    player->add<CTransform>(Vec2f(m_window.getSize().x / 2.0f, m_window.getSize().y / 2.0f), Vec2f(0.0f, 0.0f), 0.0f);
    player->add<CShape>((float)m_playerConfig.SR, m_playerConfig.V,
                            sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
                            sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), (float)m_playerConfig.OT);
    player->add<CCollision>((float)m_playerConfig.CR);
    player->add<CInput>();
}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
    Vec2f enemySpeed = Vec2f(rand() % 10 * 0.5f, rand() % 10 * 0.5f);
    int numVertices = m_enemyConfig.VMIN + (rand() % (m_enemyConfig.VMAX - m_enemyConfig.VMIN));
    float randX     = (float)m_enemyConfig.SR + (rand() % (m_window.getSize().x - 2 * m_enemyConfig.SR));
    float randY     = (float)m_enemyConfig.SR + (rand() % (m_window.getSize().y - 2 * m_enemyConfig.SR));

    auto entity = m_entities.addEntity("enemy");
    entity->add<CTransform>(Vec2f(randX, randY), enemySpeed, 0.0f);
    entity->add<CShape>((float)m_enemyConfig.SR, numVertices, sf::Color(rand() % 255, rand() % 255, rand() % 255),
                        sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), (float)m_enemyConfig.OT);
    entity->add<CCollision>((float)m_enemyConfig.CR);
    entity->add<CScore>(numVertices * 100);
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    size_t num = e->get<CShape>().circle.getPointCount();
    float angle = (360.0f / num) * (3.14159f / 180.0f);
    for (size_t i = 0; i < num; i++)
    {
        Vec2f enemySpeed = Vec2f(2*cosf(angle*i), 2*sinf(angle*i));

        auto entity = m_entities.addEntity("small");
        entity->add<CTransform>(e->get<CTransform>().pos, enemySpeed, 0.0f);
        entity->add<CShape>(e->get<CShape>().circle.getRadius()/2,
                            e->get<CShape>().circle.getPointCount(), 
                            e->get<CShape>().circle.getFillColor(), 
                            e->get<CShape>().circle.getOutlineColor(), 
                            e->get<CShape>().circle.getOutlineThickness());
        entity->add<CCollision>(e->get<CShape>().circle.getRadius() / 2);
        entity->add<CScore>((int)(2 * e->get<CShape>().circle.getPointCount() * 100));
        entity->add<CLifespan>(m_enemyConfig.L);
    }
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2f & target)
{
    Vec2f delta         = target - entity->get<CTransform>().pos;
    float angle         = atan2f(delta.y, delta.x);
    Vec2f bulletSpeed   = Vec2f(8*cosf(angle), 8*sinf(angle));

    auto bullet         = m_entities.addEntity("bullet");
    bullet->add<CTransform>(entity->get<CTransform>().pos, bulletSpeed, 0.0f);
    bullet->add<CShape>((float)m_bulletConfig.SR, m_bulletConfig.V,
                        sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), 
                        sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), (float)m_bulletConfig.OT);
    bullet->add<CCollision>((float)m_bulletConfig.CR);
    bullet->add<CLifespan>(m_bulletConfig.L);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
    size_t numBullets = 18;
    float adelta = (360.0f / numBullets) * (3.14159f / 180.0f);
    for (size_t i = 0; i < numBullets; i++)
    {
        float angle = i * adelta;
        Vec2f bulletSpeed = Vec2f(8*cosf(angle), 8*sinf(angle));

        auto bullet = m_entities.addEntity("bullet");
        bullet->add<CTransform>(entity->get<CTransform>().pos, bulletSpeed, 0.0f);
        bullet->add<CShape>(8.0f, 8, sf::Color(0, 0, 0), sf::Color(255, 0, 0), 2.0f);
        bullet->add<CCollision>(5.0f);
        bullet->add<CLifespan>(30);
    }
}

void Game::sMovement()
{
    // player movement
    Vec2f playerSpeed(0.0f, 0.0f);

    if (player()->get<CInput>().up)       { playerSpeed.y -= 4; }
    if (player()->get<CInput>().down)     { playerSpeed.y += 4; }
    if (player()->get<CInput>().left)     { playerSpeed.x -= 4; }
    if (player()->get<CInput>().right)    { playerSpeed.x += 4; }

    player()->get<CTransform>().velocity = playerSpeed;

    for (auto e : m_entities.getEntities())
    {
        auto& transform = e->get<CTransform>();

        // move the object based on its speed
        transform.pos = transform.velocity + transform.pos;
    }
}

void Game::sCollision()
{
    // collision of player with walls
    Vec2f minPlayerPos(player()->get<CCollision>().radius, player()->get<CCollision>().radius);
    Vec2f maxPlayerPos(m_window.getSize().x - player()->get<CCollision>().radius, m_window.getSize().y - player()->get<CCollision>().radius);
    auto& pPos = player()->get<CTransform>().pos;
    if (pPos.x < minPlayerPos.x) { pPos.x = minPlayerPos.x; }
    if (pPos.y < minPlayerPos.y) { pPos.y = minPlayerPos.y; }
    if (pPos.x > maxPlayerPos.x) { pPos.x = maxPlayerPos.x; }
    if (pPos.y > maxPlayerPos.y) { pPos.y = maxPlayerPos.y; }

    for (auto e : m_entities.getEntities("enemy"))
    {
        if (e->has<CCollision>())
        {
            auto& t = e->get<CTransform>();
            auto& r = e->get<CCollision>().radius;

            if (t.pos.x - r < 0) { t.velocity.x *= -1; }
            if (t.pos.y - r < 0) { t.velocity.y *= -1; }
            if (t.pos.x + r > m_window.getSize().x) { t.velocity.x *= -1; }
            if (t.pos.y + r > m_window.getSize().y) { t.velocity.y *= -1; }
        }
    }

    for (auto e : m_entities.getEntities("small"))
    {
        if (e->has<CCollision>())
        {
            auto& transform = e->get<CTransform>();
            auto& r = e->get<CCollision>().radius;

            if (transform.pos.x - r < 0) { transform.velocity.x *= -1; }
            if (transform.pos.y - r < 0) { transform.velocity.y *= -1; }
            if (transform.pos.x + r > m_window.getSize().x) { transform.velocity.x *= -1; }
            if (transform.pos.y + r > m_window.getSize().y) { transform.velocity.y *= -1; }
        }
    }

    // bullet collisions with enemies
    // collision with bullets
    for (auto b : m_entities.getEntities("bullet"))
    {
        for (auto e : m_entities.getEntities("enemy"))
        {
            if (b->get<CTransform>().pos.dist(e->get<CTransform>().pos) < (b->get<CCollision>().radius + e->get<CCollision>().radius))
            {
                m_score += e->get<CScore>().score;
                b->destroy();
                e->destroy();
                spawnSmallEnemies(e);
            }
        }

        for (auto e : m_entities.getEntities("small"))
        {
            if (b->get<CTransform>().pos.dist(e->get<CTransform>().pos) < (b->get<CCollision>().radius + e->get<CCollision>().radius))
            {
                m_score += e->get<CScore>().score;
                b->destroy();
                e->destroy();
            }
        }
    }

    bool playerRespawn = false;

    // player enemy collisions
    for (auto e : m_entities.getEntities("small"))
    {
        if (player()->get<CTransform>().pos.dist(e->get<CTransform>().pos) < (player()->get<CCollision>().radius + e->get<CCollision>().radius))
        {
            e->destroy();
            playerRespawn = true;
        }
    }

    for (auto e : m_entities.getEntities("enemy"))
    {
        if (player()->get<CTransform>().pos.dist(e->get<CTransform>().pos) < (player()->get<CCollision>().radius + e->get<CCollision>().radius))
        {
            e->destroy();
            spawnSmallEnemies(e);
            playerRespawn = true;
        }
    }
    
    if (playerRespawn)
    {
        player()->destroy();
        spawnPlayer();
    }
}

void Game::sLifespan()
{
    for (auto e : m_entities.getEntities())
    {
        if (e->has<CLifespan>())
        {
            if (e->get<CLifespan>().remaining <= 0)
            {
                e->destroy();
            }
            else
            {
                sf::Color c = e->get<CShape>().circle.getFillColor();
                sf::Color o = e->get<CShape>().circle.getOutlineColor();
                float ratio = (float)e->get<CLifespan>().remaining / e->get<CLifespan>().lifespan;
                e->get<CShape>().circle.setFillColor(sf::Color(c.r, c.g, c.b, (int)(ratio * 255)));
                e->get<CShape>().circle.setOutlineColor(sf::Color(o.r, o.g, o.b, (int)(ratio * 255)));
            }
            e->get<CLifespan>().remaining--;
        }
    }
}

void Game::sEnemySpawner()
{
    int timeSinceLastEnemySpawn = (m_currentFrame - m_lastEnemySpawnTime);
    if (timeSinceLastEnemySpawn >= m_enemyConfig.SI)
    {
        spawnEnemy();
        m_lastEnemySpawnTime = m_currentFrame;
    }
}

sf::RectangleShape grey(sf::Vector2f(32, 32));

void destroyButton(std::shared_ptr<Entity> e)
{
    auto  c = e->get<CShape>().circle.getFillColor();
    ImVec4 color((float)c.r / 255, (float)c.g / 255, (float)c.b / 255, (float)c.a/255);
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    std::stringstream ss;
    ss << "D##" << e->id();
    if (ImGui::Button(ss.str().c_str()))
    {
        e->destroy();
    }
    ImGui::PopStyleColor();
}

void Game::sGUI()
{
    ImGui::Begin("Geometry Wars");
    
    if (ImGui::BeginTabBar("MyTabBar"))
    {
        if (ImGui::BeginTabItem("Systems"))
        {
            
            ImGui::Separator();
            ImGui::Checkbox("Movement", &m_doMovement);
            ImGui::Checkbox("Lifespan", &m_doLifespan);
            ImGui::Checkbox("Collision", &m_doCollision);
            ImGui::Checkbox("Spawning", &m_doSpawning);
            ImGui::Indent();
            ImGui::SliderInt("Spawn Interval", &m_enemyConfig.SI, 1, 180);
            if (ImGui::Button("Manual Spawn"))
            {
                spawnEnemy();
            }
            ImGui::Unindent();
            ImGui::Checkbox("GUI", &m_doGUI);
            ImGui::Checkbox("Rendering", &m_doRender);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Entity Manager"))
        {
            if (ImGui::CollapsingHeader("Entities by Tag", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Indent();
                for (auto& kv : m_entities.getEntityMap())
                {
                    if (ImGui::CollapsingHeader(kv.first.c_str()))
                    {
                        ImGui::Indent();
                        auto& entities = m_entities.getEntities(kv.first);
                        for (auto e : entities)
                        {
                            destroyButton(e);
                            ImGui::SameLine();
                            ImGui::Text("%3d %8s  (%.0f,%.0f)", (int)e->id(), e->tag().c_str(), e->get<CTransform>().pos.x, e->get<CTransform>().pos.y);
                        }
                        ImGui::Unindent();
                    }
                }

                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("All Entities"))
            {
                ImGui::Indent();
                auto& entities = m_entities.getEntities();
                for (auto e : entities)
                {
                    destroyButton(e);
                    ImGui::SameLine();
                    ImGui::Text("%3d %8s  (%.0f,%.0f)", (int)e->id(), e->tag().c_str(), e->get<CTransform>().pos.x, e->get<CTransform>().pos.y);
                }
                ImGui::Unindent();
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    
    ImGui::End();
}

void Game::sRender()
{
    grey.setFillColor(sf::Color(15, 15, 15));
    m_window.clear();

    // draw background
    for (size_t i = 0; i < (32+m_window.getSize().x)/32; i ++)
    {
        for (size_t j = 0; j < (32+m_window.getSize().y)/32; j++)
        {
            if ((i + j) % 2)
            {
                grey.setPosition({ i * 32.0f, j * 32.0f });
                m_window.draw(grey);
            }
        }
    }

    // draw entities
    for (auto e : m_entities.getEntities())
    {
        auto & pos = e->get<CTransform>().pos;

        if (e->has<CShape>())     
        { 
            e->get<CTransform>().angle += 3.0f;
            e->get<CShape>().circle.setPosition(pos);
            e->get<CShape>().circle.setRotation(sf::degrees(e->get<CTransform>().angle));
            m_window.draw(e->get<CShape>().circle);
        }
    }

    // draw score
    std::stringstream ss;
    ss << "SCORE: " << m_score;
    m_text.setString(ss.str());
    m_window.draw(m_text);
    ImGui::SFML::Render(m_window);
    m_window.display();
}

void Game::sUserInput()
{
    while (auto event = m_window.pollEvent())
    {
        // pass the event to imgui to be parsed
        ImGui::SFML::ProcessEvent(m_window, *event);
        
        // this event triggers when the window is closed
        if (event->is<sf::Event::Closed>())
        {
            std::exit(0);
        }

        // this event is triggered when a key is pressed
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            switch (keyPressed->scancode)
            {
                case sf::Keyboard::Scancode::Escape: std::exit(0); break;
                case sf::Keyboard::Scancode::W: player()->get<CInput>().up = true; break;
                case sf::Keyboard::Scancode::A: player()->get<CInput>().left = true; break;
                case sf::Keyboard::Scancode::S: player()->get<CInput>().down = true; break;
                case sf::Keyboard::Scancode::D: player()->get<CInput>().right = true; break;
                case sf::Keyboard::Scancode::P: setPaused(!m_paused); break;
                default: break;
            }
        }

        // this event is triggered when a key is pressed
        if (const auto* keyPressed = event->getIf<sf::Event::KeyReleased>())
        {
            switch (keyPressed->scancode)
            {
                case sf::Keyboard::Scancode::W: player()->get<CInput>().up = false; break;
                case sf::Keyboard::Scancode::A: player()->get<CInput>().left = false; break;
                case sf::Keyboard::Scancode::S: player()->get<CInput>().down = false; break;
                case sf::Keyboard::Scancode::D: player()->get<CInput>().right = false; break;
                default: break;
            }
        }

        if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (ImGui::GetIO().WantCaptureMouse) { continue; }
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                spawnBullet(player(), mousePressed->position);
            }
            else if (mousePressed->button == sf::Mouse::Button::Right)
            {
                spawnSpecialWeapon(player());
            }
        }
    }
}

std::shared_ptr<Entity> Game::player()
{
    return m_entities.getEntities("player").back();
}
