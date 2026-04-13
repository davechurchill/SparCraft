#include "Game.h"

#include <iostream>

Game::Game(const std::string & config)
    : m_text(m_font, "Default", 24)
{ 
    init(config);
}

void Game::init(const std::string & path)
{
    // TODO: read in config file here
    //       use the premade PlayerConfig, EnemyConfig, BulletConfig variables

    // set up default window parameters
    m_window.create(sf::VideoMode({ 1280, 720 }), "Assignment 2");
    m_window.setKeyRepeatEnabled(false);
    m_window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(m_window)) {}

    // scale the imgui ui and text size by 2
    ImGui::GetStyle().ScaleAllSizes(2.0f);
    ImGui::GetIO().FontGlobalScale = 2.0f;

    spawnPlayer();
}

std::shared_ptr<Entity> Game::player()
{
    return m_entities.getEntities("player").back();
}

void Game::run()
{
    // TODO: add pause functionality in here
    //       some systems should function while paused (rendering)
    //       some systems shouldn't (movement / input)
    while (true)
    {
        // update the entity manager
        m_entities.update();
        
        // required update call to imgui
        ImGui::SFML::Update(m_window, m_deltaClock.restart());

        sUserInput();
        sEnemySpawner();
        sMovement();
        sCollision();
        sGUI();
        sRender();
        
        // increment the current frame
        // may need to be moved when pause implemented
        m_currentFrame++;
    }
}

void Game::setPaused(bool paused)
{
    // TODO
}

// respawn the player in the middle of the screen
void Game::spawnPlayer()
{
    // TODO: Finish adding all properties of the player with the correct values from the config

    // We create every entity by calling EntityManager.addEntity(tag)
    // This returns a std::shared_ptr<Entity>, so we use 'auto' to save typing
    auto e = m_entities.addEntity("player");

    // Give this entity a Transform so it spawns at (200,200) with velocity (1,1) and angle 0
    e->add<CTransform>(Vec2f(200.0f, 200.0f), Vec2f(1.0f, 1.0f), 0.0f);
    
    // The entity's shape will have radius 32, 8 sides, dark grey fill, and red outline of thickness 4
    e->add<CShape>(32.0f, 8, sf::Color(10, 10, 10), sf::Color(255, 0, 0), 4.0f);
    
    // Add an input component to the player so that we can use inputs
    e->add<CInput>();
}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
    // TODO: make sure the enemy is spawned properly with the m_enemyConfig variables
    //       the enemy must be spawned completely within the bounds of the window
    //
    
    // record when the most recent enemy was spawned
    m_lastEnemySpawnTime = m_currentFrame;
}

// spawns the small enemies when a big one (input entity e) explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    // TODO: spawn small enemies at the location of the input enemy e

    // when we create the smaller enemy, we have to read the values of the original enemy
    // - spawn a number of small enemies equal to the vertices of the original enemy
    // - set each small enemy to the same color as the original, half the size
    // - small enemies are worth double points of the original enemy
}

// spawns a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2f & target)
{
    // TODO: implement the spawning of a bullet which travels toward target
    //       - bullet speed is given as a scalar speed
    //       - you must set the velocity by using formula in notes
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
    // TODO: implement your own special weapon
}

void Game::sMovement()
{
    // TODO: implement all entity movement in this function
    //       you should read the m_player->cInput component to determine if the player is moving

    // Sample movement speed update for the player
    auto& transform = player()->get<CTransform>();
    transform.pos.x += transform.velocity.x;
    transform.pos.y += transform.velocity.y;
}

void Game::sLifespan()
{
    // TODO: implement all lifespan functionality
    // 
    // for all entities
    //     if entity has no lifespan component, skip it
    //     if entity has > 0 remaining lifespan, subtract 1
    //     if it has lifespan and is alive
    //         scale its alpha channel properly
    //     if it has lifespan and its time is up
    //         destroy the entity
}

void Game::sCollision()
{
    // TODO: implement all proper collisions between entities
    //       be sure to use the collision radius, NOT the shape radius

    for (auto b : m_entities.getEntities("bullet"))
    {
        for (auto e : m_entities.getEntities("enemy"))
        {
            // do collision logic
        }

        for (auto e : m_entities.getEntities("smallEnemy"))
        {
            // do colision logic
        }
    }
}

void Game::sEnemySpawner()
{
    // TODO: code which implements enemy spawning should go here
}

void Game::sGUI()
{
    ImGui::Begin("Geometry Wars");

    ImGui::Text("Stuff Goes Here");

    ImGui::End();
}

void Game::sRender()
{
    if (!m_window.isOpen()) { return; }

    // TODO: change the code below to draw ALL of the entities
    //       sample drawing of the player Entity that we have created
    m_window.clear();

    // set the position of the shape based on the entity's transform->pos
    player()->get<CShape>().circle.setPosition(player()->get<CTransform>().pos);

    // set the rotation of the shape based on the entity's transform->angle
    player()->get<CTransform>().angle += 1.0f;
    player()->get<CShape>().circle.setRotation(sf::degrees(player()->get<CTransform>().angle));

    // draw the entity's sf::CircleShape
    m_window.draw(player()->get<CShape>().circle);

    // draw the ui last
    ImGui::SFML::Render(m_window);

    m_window.display();
}

void Game::sUserInput()
{
    // TODO: handle user input here
    //       note that you should only be setting the player's input component variables here
    //       you should not implement the player's movement logic here
    //       the movement system will read the variables you set in this function

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
            // print the key that was pressed to the console
            std::cout << "Key pressed = " << int(keyPressed->scancode) << "\n";

            if (keyPressed->scancode == sf::Keyboard::Scancode::W)
            {
                // TODO: set player's input component UP to true
                std::cout << "W Key Pressed\n";
            }
        }

        // this event is triggered when a key is released
        if (const auto* keyPressed = event->getIf<sf::Event::KeyReleased>())
        {
            // print the key that was pressed to the console
            std::cout << "Key released  = " << int(keyPressed->scancode) << "\n";

            if (keyPressed->scancode == sf::Keyboard::Scancode::W)
            {
                // TODO: set player's input component UP to false
                std::cout << "W Key Released\n";
            }
        }

        if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            Vec2f mpos(mousePressed->position);
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                // TODO: call spawnBullet here
                std::cout << "Left mouse (" << mpos.x << ", " << mpos.y << ")\n";
            }
            else if (mousePressed->button == sf::Mouse::Button::Right)
            {
                // TODO: call special weapon here
                std::cout << "Right mouse (" << mpos.x << ", " << mpos.y << ")\n";
            }
        }
    }
}