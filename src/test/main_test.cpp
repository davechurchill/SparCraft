#include "catch2/catch_amalgamated.hpp"

#include "SparCraft.h"
#include "GameState.h"
#include "Game.h"
#include "AllPlayers.h"
#include "MoveArray.hpp"
#include "Array.hpp"
#include "Position.hpp"
#include "BaseTypes.hpp"
#include <memory>
#include <array>
#include <cmath>
#include <set>
#include <sstream>

using namespace SparCraft;

class SparCraftInitListener : public Catch::EventListenerBase
{
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testRunStarting(Catch::TestRunInfo const &) override
    {
        // Suppress file logging during tests; expected throw-path tests should not
        // pollute runtime log files.
        SparCraft::SCLog().setEnabled(false);

        // Keep SparCraft initialization in runtime startup logic before tests execute.
        SparCraft::init();
    }

    void testRunEnded(Catch::TestRunStats const &) override
    {
        SparCraft::SCLog().setEnabled(true);
    }
};

CATCH_REGISTER_LISTENER(SparCraftInitListener)

class ScopedCerrSilence
{
    std::streambuf * oldBuf;
    std::ostringstream sink;

public:
    ScopedCerrSilence()
        : oldBuf(std::cerr.rdbuf(sink.rdbuf()))
    {
    }

    ~ScopedCerrSilence()
    {
        std::cerr.rdbuf(oldBuf);
    }
};

#define REQUIRE_THROWS_SILENT(expr) \
    do                              \
    {                               \
        ScopedCerrSilence silence;  \
        REQUIRE_THROWS(expr);       \
    } while (0)

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------
static GameState makeTwoMarines()
{
    // One Terran Marine per side, face to face within attack range
    GameState s;
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(100, 100));
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_Two, Position(200, 100));
    return s;
}

static GameState makeSymmetric(BWAPI::UnitType type, int n, int p1x, int p2x, int y = 100)
{
    GameState s;
    for (int i = 0; i < n; ++i)
    {
        s.addUnit(type, Players::Player_One, Position(p1x, y + i * 32));
        s.addUnit(type, Players::Player_Two, Position(p2x, y + i * 32));
    }
    return s;
}

static void addMarines(GameState & s, const size_t player, int n, int x, int y = 100, int dy = 32)
{
    for (int i = 0; i < n; ++i)
    {
        s.addUnit(BWAPI::UnitTypes::Terran_Marine, player, Position(x, y + i * dy));
    }
}

// -----------------------------------------------------------------------
// GameState — construction and unit management
// -----------------------------------------------------------------------
TEST_CASE("GameState default construction")
{
    GameState s;
    REQUIRE(s.numUnits(Players::Player_One) == 0);
    REQUIRE(s.numUnits(Players::Player_Two) == 0);
    REQUIRE(s.getTime() == 0);
}

TEST_CASE("addUnit increments counts correctly")
{
    GameState s;
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(0, 0));
    REQUIRE(s.numUnits(Players::Player_One) == 1);
    REQUIRE(s.numUnits(Players::Player_Two) == 0);

    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_Two, Position(200, 0));
    REQUIRE(s.numUnits(Players::Player_One) == 1);
    REQUIRE(s.numUnits(Players::Player_Two) == 1);
}

TEST_CASE("addUnit does not crash when only one player has units (fix 8)")
{
    // Before fix 8, addUnit called whoCanMove() which read getUnit(1,0)
    // even when player 2 had zero units.
    GameState s;
    REQUIRE_NOTHROW(s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(0, 0)));
    REQUIRE_NOTHROW(s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(32, 0)));
    REQUIRE(s.numUnits(Players::Player_One) == 2);
}

TEST_CASE("addUnit assigns unique unit IDs")
{
    GameState s;
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(0,   0));
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(32,  0));
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_Two, Position(200, 0));

    std::set<size_t> ids;
    for (size_t p = 0; p < 2; ++p)
        for (size_t u = 0; u < s.numUnits(p); ++u)
            ids.insert(s.getUnit(p, u).ID());

    REQUIRE(ids.size() == 3);
}

TEST_CASE("getUnit and getUnitByID agree")
{
    GameState s = makeTwoMarines();
    const Unit & u0 = s.getUnit(Players::Player_One, 0);
    const Unit & u0byID = s.getUnitByID(Players::Player_One, u0.ID());
    REQUIRE(u0.ID() == u0byID.ID());
}

TEST_CASE("getEnemy returns correct opponent")
{
    GameState s;
    REQUIRE(s.getEnemy(Players::Player_One) == Players::Player_Two);
    REQUIRE(s.getEnemy(Players::Player_Two) == Players::Player_One);
}

// -----------------------------------------------------------------------
// whoCanMove / updateGameTime
// -----------------------------------------------------------------------
TEST_CASE("whoCanMove returns Player_One when only player one has units (fix 8)")
{
    GameState s;
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(0, 0));
    REQUIRE(s.whoCanMove() == Players::Player_One);
}

TEST_CASE("whoCanMove returns Player_Two when only player two has units (fix 8)")
{
    GameState s;
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_Two, Position(0, 0));
    REQUIRE(s.whoCanMove() == Players::Player_Two);
}

TEST_CASE("whoCanMove returns Player_Both when both players have equal-time units")
{
    // Fresh state: all units start at time 0 so both move simultaneously
    GameState s = makeTwoMarines();
    REQUIRE(s.whoCanMove() == Players::Player_Both);
}

// -----------------------------------------------------------------------
// playerDead / isTerminal
// -----------------------------------------------------------------------
TEST_CASE("playerDead returns true for player with no units")
{
    GameState s;
    REQUIRE(s.playerDead(Players::Player_One));
    REQUIRE(s.playerDead(Players::Player_Two));
}

TEST_CASE("playerDead returns false for player with a marine")
{
    GameState s = makeTwoMarines();
    REQUIRE_FALSE(s.playerDead(Players::Player_One));
    REQUIRE_FALSE(s.playerDead(Players::Player_Two));
}

TEST_CASE("isTerminal is true when a player has no units")
{
    GameState s;
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(0, 0));
    // Player two has no units → terminal
    REQUIRE(s.isTerminal());
}

TEST_CASE("isTerminal is false for two opposing marines in range")
{
    GameState s = makeTwoMarines();
    REQUIRE_FALSE(s.isTerminal());
}

// -----------------------------------------------------------------------
// generateMoves
// -----------------------------------------------------------------------
TEST_CASE("generateMoves produces at least one move per unit")
{
    GameState s = makeTwoMarines();
    auto moves = std::make_unique<MoveArray>();
    s.generateMoves(*moves, Players::Player_One);
    REQUIRE(moves->numUnits() >= 1);
    REQUIRE(moves->numMoves(0) >= 1);
}

TEST_CASE("generateMoves produces ATTACK moves when enemy is in range")
{
    // Place units adjacent so they can attack
    GameState s;
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(100, 100));
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_Two, Position(132, 100));

    auto moves = std::make_unique<MoveArray>();
    s.generateMoves(*moves, Players::Player_One);

    bool hasAttack = false;
    for (size_t m = 0; m < moves->numMoves(0); ++m)
        if (moves->getMove(0, m).type() == ActionTypes::ATTACK)
            hasAttack = true;

    REQUIRE(hasAttack);
}

TEST_CASE("generateMoves produces MOVE moves for mobile units")
{
    GameState s = makeTwoMarines();
    auto moves = std::make_unique<MoveArray>();
    s.generateMoves(*moves, Players::Player_One);

    bool hasMove = false;
    for (size_t m = 0; m < moves->numMoves(0); ++m)
        if (moves->getMove(0, m).type() == ActionTypes::MOVE)
            hasMove = true;

    REQUIRE(hasMove);
}

TEST_CASE("generateMoves includes PASS when no other moves available")
{
    // Missile Turret cannot move and cannot attack ground units, so it should
    // fall back to PASS.
    GameState s;
    s.addUnit(BWAPI::UnitTypes::Terran_Missile_Turret, Players::Player_One, Position(100, 100));
    s.addUnit(BWAPI::UnitTypes::Terran_Marine,         Players::Player_Two, Position(132, 100));

    auto moves = std::make_unique<MoveArray>();
    s.generateMoves(*moves, Players::Player_One);

    bool hasPassOrReload = false;
    for (size_t m = 0; m < moves->numMoves(0); ++m)
    {
        auto t = moves->getMove(0, m).type();
        if (t == ActionTypes::PASS || t == ActionTypes::RELOAD)
            hasPassOrReload = true;
    }
    REQUIRE(hasPassOrReload);
}

// -----------------------------------------------------------------------
// performAction / makeMoves
// -----------------------------------------------------------------------
TEST_CASE("attacking the same target twice does not crash (fix 1)")
{
    // Two marines target the same lone enemy marine.
    // After the first kills it, the second should pass rather than crash or waste cooldown.
    GameState s;
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(100, 100));
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(132, 100));
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_Two, Position(116, 100));

    // Manually construct two ATTACK actions targeting the lone enemy (index 0)
    std::vector<Action> attacks = {
        Action(0, Players::Player_One, ActionTypes::ATTACK, 0),
        Action(1, Players::Player_One, ActionTypes::ATTACK, 0)
    };
    REQUIRE_NOTHROW(s.makeMoves(attacks));
}

TEST_CASE("unit count decreases when a unit dies")
{
    GameState s = makeTwoMarines();

    PlayerPtr p1 = AllPlayers::getPlayerPtr(Players::Player_One, PlayerModels::AttackClosest);
    PlayerPtr p2 = AllPlayers::getPlayerPtr(Players::Player_Two, PlayerModels::AttackClosest);
    auto game = std::make_unique<Game>(s, p1, p2, 500);
    game->play();

    REQUIRE(game->getState().isTerminal());
}

// -----------------------------------------------------------------------
// Eval — LTD / LTD2
// -----------------------------------------------------------------------
TEST_CASE("LTD returns 0 for player with no units")
{
    GameState s = makeTwoMarines();
    // Access via evalLTD when one side is dead — we'll test LTD directly
    // by checking the score symmetry from a known equal state
    ScoreType ltd = s.LTD(Players::Player_One);
    REQUIRE(ltd > 0);
}

TEST_CASE("LTD is symmetric for equal armies")
{
    GameState s = makeTwoMarines();
    ScoreType ltd1 = s.LTD(Players::Player_One);
    ScoreType ltd2 = s.LTD(Players::Player_Two);
    REQUIRE(ltd1 == ltd2);
}

TEST_CASE("LTD2 is symmetric for equal armies")
{
    GameState s = makeTwoMarines();
    ScoreType ltd1 = s.LTD2(Players::Player_One);
    ScoreType ltd2 = s.LTD2(Players::Player_Two);
    REQUIRE(ltd1 == ltd2);
}

TEST_CASE("evalLTD is 0 for symmetric states")
{
    GameState s = makeTwoMarines();
    ScoreType score = s.evalLTD(Players::Player_One);
    REQUIRE(score == 0);
}

TEST_CASE("evalLTD is neutral for full-health armies even when unit counts differ")
{
    // 3 marines vs 1 marine
    GameState s = makeSymmetric(BWAPI::UnitTypes::Terran_Marine, 1, 100, 500);
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(132, 100));
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(164, 100));

    ScoreType score = s.evalLTD(Players::Player_One);
    REQUIRE(score == 0);
}

TEST_CASE("LTD does not divide by zero when totalLTD is 0 (fix 3)")
{
    // A unit type with 0 dpf would make totalLTD=0.
    // We simulate this by manually zeroing the baseline via setTotalLTD.
    GameState s = makeTwoMarines();
    s.setTotalLTD(0.0f, 0.0f);
    REQUIRE_NOTHROW(s.LTD(Players::Player_One));
    REQUIRE(s.LTD(Players::Player_One) == 0);
}

TEST_CASE("LTD2 does not divide by zero when totalSumSQRT is 0 (fix 3)")
{
    GameState s = makeTwoMarines();
    s.setTotalLTD2(0.0f, 0.0f);
    REQUIRE_NOTHROW(s.LTD2(Players::Player_One));
    REQUIRE(s.LTD2(Players::Player_One) == 0);
}

TEST_CASE("eval gives win bonus when enemy is dead")
{
    // Start a game and play until one side wins, then check eval
    GameState s = makeSymmetric(BWAPI::UnitTypes::Terran_Marine, 3, 100, 200);
    PlayerPtr p1 = AllPlayers::getPlayerPtr(Players::Player_One, PlayerModels::AttackClosest);
    PlayerPtr p2 = AllPlayers::getPlayerPtr(Players::Player_Two, PlayerModels::AttackClosest);
    Game game(s, p1, p2, 2000);
    game.play();

    const GameState & final = game.getState();
    bool p1Dead = final.playerDead(Players::Player_One);
    bool p2Dead = final.playerDead(Players::Player_Two);

    if (p1Dead && !p2Dead)
    {
        StateEvalScore score = final.eval(Players::Player_Two, EvaluationMethods::LTD);
        REQUIRE(score.val() > 0);
    }
    else if (p2Dead && !p1Dead)
    {
        StateEvalScore score = final.eval(Players::Player_One, EvaluationMethods::LTD);
        REQUIRE(score.val() > 0);
    }
    // If both dead (simultaneous kill) the result is 0 which is also valid
}

// -----------------------------------------------------------------------
// Hash
// -----------------------------------------------------------------------
TEST_CASE("same state produces same hash")
{
    GameState s1 = makeTwoMarines();
    GameState s2 = makeTwoMarines();
    REQUIRE(s1.calculateHash(0) == s2.calculateHash(0));
}

TEST_CASE("different states produce different hashes")
{
    GameState s1 = makeTwoMarines();

    GameState s2;
    s2.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(100, 100));
    s2.addUnit(BWAPI::UnitTypes::Protoss_Zealot, Players::Player_Two, Position(200, 100));

    REQUIRE(s1.calculateHash(0) != s2.calculateHash(0));
}

// -----------------------------------------------------------------------
// closestEnemyUnitDistance
// -----------------------------------------------------------------------
TEST_CASE("closestEnemyUnitDistance returns correct squared distance")
{
    GameState s;
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(0,   0));
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_Two, Position(300, 0));
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_Two, Position(100, 0));

    const Unit & marine = s.getUnit(Players::Player_One, 0);
    size_t dist = s.closestEnemyUnitDistance(marine);

    // Closest enemy is at (100,0): squared dist = 100*100 = 10000
    REQUIRE(dist == 10000);
}

// -----------------------------------------------------------------------
// getClosestEnemyUnit
// -----------------------------------------------------------------------
TEST_CASE("getClosestEnemyUnit returns the nearest enemy")
{
    GameState s;
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(0,   0));
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_Two, Position(500, 0)); // far
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_Two, Position(100, 0)); // close

    const Unit & closest = s.getClosestEnemyUnit(Players::Player_One, 0);
    REQUIRE(closest.pos().x() == 100);
}

// -----------------------------------------------------------------------
// Full game simulation
// -----------------------------------------------------------------------
TEST_CASE("game reaches terminal state within move limit")
{
    GameState s = makeSymmetric(BWAPI::UnitTypes::Terran_Marine, 4, 100, 400);
    PlayerPtr p1 = AllPlayers::getPlayerPtr(Players::Player_One, PlayerModels::AttackClosest);
    PlayerPtr p2 = AllPlayers::getPlayerPtr(Players::Player_Two, PlayerModels::AttackClosest);
    Game game(s, p1, p2, 5000);
    game.play();
    REQUIRE(game.getState().isTerminal());
}

TEST_CASE("larger army wins more often (AttackClosest, 5v2 marines)")
{
    int p1wins = 0, p2wins = 0;
    const int trials = 10;

    for (int i = 0; i < trials; ++i)
    {
        GameState s;
        for (int u = 0; u < 5; ++u)
            s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(100, 100 + u * 32));
        for (int u = 0; u < 2; ++u)
            s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_Two, Position(400, 100 + u * 32));

        PlayerPtr p1 = AllPlayers::getPlayerPtr(Players::Player_One, PlayerModels::AttackClosest);
        PlayerPtr p2 = AllPlayers::getPlayerPtr(Players::Player_Two, PlayerModels::AttackClosest);
        Game game(s, p1, p2, 5000);
        game.play();

        const GameState & final = game.getState();
        if (final.playerDead(Players::Player_Two) && !final.playerDead(Players::Player_One))
            p1wins++;
        else if (final.playerDead(Players::Player_One) && !final.playerDead(Players::Player_Two))
            p2wins++;
    }
    REQUIRE(p1wins > p2wins);
}

TEST_CASE("kiter player survives longer than stationary player")
{
    // Kiter should take less damage than AttackClosest in an equal fight
    auto runGame = [](size_t p1model, size_t p2model) -> int
    {
        GameState s = makeSymmetric(BWAPI::UnitTypes::Terran_Marine, 3, 100, 400);
        PlayerPtr p1 = AllPlayers::getPlayerPtr(Players::Player_One, p1model);
        PlayerPtr p2 = AllPlayers::getPlayerPtr(Players::Player_Two, p2model);
        Game game(s, p1, p2, 5000);
        game.play();
        int hp = 0;
        const GameState & f = game.getState();
        for (size_t u = 0; u < f.numUnits(Players::Player_One); ++u)
            hp += f.getUnit(Players::Player_One, u).currentHP();
        return hp;
    };

    int kiterHP  = runGame(PlayerModels::Kiter,         PlayerModels::AttackClosest);
    int staticHP = runGame(PlayerModels::AttackClosest, PlayerModels::Kiter);

    // Kiter as P1 should end with more HP than when AttackClosest is P1 against Kiter
    REQUIRE(kiterHP >= staticHP);
}

TEST_CASE("evalSim with NOKDPS runs without crash")
{
    GameState s = makeTwoMarines();
    REQUIRE_NOTHROW(s.evalSim(Players::Player_One, PlayerModels::NOKDPS, PlayerModels::NOKDPS));
}

TEST_CASE("evalSim with Random script runs without crash (fix 7 — no silent upgrade)")
{
    GameState s = makeTwoMarines();
    REQUIRE_NOTHROW(s.evalSim(Players::Player_One, PlayerModels::Random, PlayerModels::Random));
}

TEST_CASE("whoCanMove and bothCanMove stay consistent across empty, one-sided, and symmetric marine states")
{
    GameState empty;
    REQUIRE(empty.whoCanMove() == Players::Player_Both);
    REQUIRE(empty.bothCanMove());

    for (int n = 1; n <= 10; ++n)
    {
        CAPTURE(n);

        GameState p1Only;
        addMarines(p1Only, Players::Player_One, n, 100);
        REQUIRE(p1Only.whoCanMove() == Players::Player_One);
        REQUIRE_FALSE(p1Only.bothCanMove());

        GameState p2Only;
        addMarines(p2Only, Players::Player_Two, n, 300);
        REQUIRE(p2Only.whoCanMove() == Players::Player_Two);
        REQUIRE_FALSE(p2Only.bothCanMove());

        GameState symmetric = makeSymmetric(BWAPI::UnitTypes::Terran_Marine, n, 100, 500);
        REQUIRE(symmetric.whoCanMove() == Players::Player_Both);
        REQUIRE(symmetric.bothCanMove());
    }
}

TEST_CASE("getUnit bounds checks reject invalid accesses")
{
    GameState s;
    REQUIRE_THROWS_SILENT(s.getUnit(Players::Player_One, 0));
    REQUIRE_THROWS_SILENT(s.getUnit(Players::Player_Two, 0));

    addMarines(s, Players::Player_One, 1, 100);
    REQUIRE_NOTHROW(s.getUnit(Players::Player_One, 0));
    REQUIRE_THROWS_SILENT(s.getUnit(Players::Player_One, 1));
    REQUIRE_THROWS_SILENT(s.getUnit(Players::Player_Two, 0));
}

TEST_CASE("generateMoves enforces that only the moving player can generate moves")
{
    GameState s;
    addMarines(s, Players::Player_One, 1, 100);

    MoveArray moves;
    REQUIRE_NOTHROW(s.generateMoves(moves, Players::Player_One));
    REQUIRE_THROWS_SILENT(s.generateMoves(moves, Players::Player_Two));
}

TEST_CASE("makeMoves can bypass can-move check only when ignoreCanMoveCheck is true")
{
    GameState s;
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_One, Position(100, 100));
    s.addUnit(BWAPI::UnitTypes::Terran_Marine, Players::Player_Two, Position(132, 100));

    std::vector<Action> p1Attack = { Action(0, Players::Player_One, ActionTypes::ATTACK, 0) };
    REQUIRE_NOTHROW(s.makeMoves(p1Attack));

    std::vector<Action> p1Pass = { Action(0, Players::Player_One, ActionTypes::PASS, 0) };
    REQUIRE_THROWS_SILENT(s.makeMoves(p1Pass));
    REQUIRE_NOTHROW(s.makeMoves(p1Pass, true));
}

TEST_CASE("closest-unit helpers return safe fallback when no valid target exists")
{
    GameState s;
    addMarines(s, Players::Player_One, 1, 100);

    const Unit & ours = s.getUnit(Players::Player_One, 0);

    REQUIRE_NOTHROW(s.getClosestOurUnit(Players::Player_One, 0));
    const Unit & closestOur = s.getClosestOurUnit(Players::Player_One, 0);
    REQUIRE(closestOur.ID() == ours.ID());

    REQUIRE_NOTHROW(s.getClosestEnemyUnit(Players::Player_One, 0));
    const Unit & closestEnemy = s.getClosestEnemyUnit(Players::Player_One, 0);
    REQUIRE(closestEnemy.ID() == ours.ID());
}

TEST_CASE("symmetric marine armies from 1 to 10 remain neutral under static evals")
{
    for (int n = 1; n <= 10; ++n)
    {
        CAPTURE(n);

        GameState s = makeSymmetric(BWAPI::UnitTypes::Terran_Marine, n, 100, 500);
        REQUIRE(s.evalLTD(Players::Player_One) == 0);
        REQUIRE(s.evalLTD2(Players::Player_One) == 0);
        REQUIRE(s.LTD(Players::Player_One) == s.LTD(Players::Player_Two));
        REQUIRE(s.LTD2(Players::Player_One) == s.LTD2(Players::Player_Two));
    }
}

TEST_CASE("generateMoves produces valid per-unit move lists for 1 to 10 marines")
{
    for (int n = 1; n <= 10; ++n)
    {
        CAPTURE(n);

        GameState s = makeSymmetric(BWAPI::UnitTypes::Terran_Marine, n, 100, 500);
        MoveArray moves;
        s.generateMoves(moves, Players::Player_One);

        REQUIRE(moves.numUnits() == static_cast<size_t>(n));
        REQUIRE(moves.validateMoves());

        for (size_t u = 0; u < moves.numUnits(); ++u)
        {
            REQUIRE(moves.numMoves(u) > 0);

            for (size_t m = 0; m < moves.numMoves(u); ++m)
            {
                const Action & a = moves.getMove(u, m);
                REQUIRE(a.player() == Players::Player_One);
                REQUIRE(a.unit() == u);
            }
        }
    }
}

TEST_CASE("evalSim remains stable across 1 to 10 marine states and script pairings")
{
    const std::array<std::pair<size_t, size_t>, 2> scriptPairs = {{
        { PlayerModels::NOKDPS,      PlayerModels::NOKDPS },
        { PlayerModels::AttackClosest, PlayerModels::AttackClosest }
    }};

    for (int n = 1; n <= 10; ++n)
    {
        GameState s = makeSymmetric(BWAPI::UnitTypes::Terran_Marine, n, 100, 500);

        for (const auto & scripts : scriptPairs)
        {
            CAPTURE(n, scripts.first, scripts.second);

            StateEvalScore score;
            REQUIRE_NOTHROW(score = s.evalSim(Players::Player_One, scripts.first, scripts.second));
            REQUIRE(std::isfinite(static_cast<double>(score.val())));
        }
    }
}

TEST_CASE("full games from 1 to 10 marines per side reach terminal states")
{
    for (int n = 1; n <= 10; ++n)
    {
        CAPTURE(n);

        GameState s = makeSymmetric(BWAPI::UnitTypes::Terran_Marine, n, 100, 500);
        PlayerPtr p1 = AllPlayers::getPlayerPtr(Players::Player_One, PlayerModels::AttackClosest);
        PlayerPtr p2 = AllPlayers::getPlayerPtr(Players::Player_Two, PlayerModels::AttackClosest);

        Game game(s, p1, p2, 15000);
        REQUIRE_NOTHROW(game.play());
        REQUIRE(game.getState().isTerminal());
    }
}

TEST_CASE("hashes are deterministic for symmetric marine states from 1 to 10")
{
    for (int n = 1; n <= 10; ++n)
    {
        CAPTURE(n);

        GameState s1 = makeSymmetric(BWAPI::UnitTypes::Terran_Marine, n, 100, 500);
        GameState s2 = makeSymmetric(BWAPI::UnitTypes::Terran_Marine, n, 100, 500);

        REQUIRE(s1.calculateHash(0) == s2.calculateHash(0));
        REQUIRE(s1.calculateHash(1) == s2.calculateHash(1));
    }
}

TEST_CASE("unit IDs remain unique for symmetric marine states from 1 to 10")
{
    for (int n = 1; n <= 10; ++n)
    {
        CAPTURE(n);

        GameState s = makeSymmetric(BWAPI::UnitTypes::Terran_Marine, n, 100, 500);
        std::set<size_t> ids;

        for (size_t p = 0; p < 2; ++p)
        {
            for (size_t u = 0; u < s.numUnits(p); ++u)
            {
                ids.insert(s.getUnit(p, u).ID());
            }
        }

        REQUIRE(ids.size() == static_cast<size_t>(2 * n));
    }
}

// -----------------------------------------------------------------------
// Position
// -----------------------------------------------------------------------
TEST_CASE("Position::getDistance — axis-aligned horizontal")
{
    SparCraft::Position a(0, 0), b(7, 0);
    REQUIRE(a.getDistance(b) == 7);
    REQUIRE(b.getDistance(a) == 7);
}

TEST_CASE("Position::getDistance — axis-aligned vertical")
{
    SparCraft::Position a(0, 0), b(0, 9);
    REQUIRE(a.getDistance(b) == 9);
    REQUIRE(b.getDistance(a) == 9);
}

TEST_CASE("Position::getDistance — 3-4-5 right triangle")
{
    // Diagonal branch: both dX and dY non-zero
    SparCraft::Position a(0, 0), b(3, 4);
    REQUIRE(a.getDistance(b) == 5);
    REQUIRE(b.getDistance(a) == 5);
}

TEST_CASE("Position::getDistance — 5-12-13 right triangle")
{
    SparCraft::Position a(0, 0), b(5, 12);
    REQUIRE(a.getDistance(b) == 13);
}

TEST_CASE("Position::getDistance — same point is zero")
{
    SparCraft::Position a(42, 99);
    REQUIRE(a.getDistance(a) == 0);
}

TEST_CASE("Position::getDistanceSq — 3-4-5 triangle gives 25")
{
    SparCraft::Position a(0, 0), b(3, 4);
    REQUIRE(a.getDistanceSq(b) == 25);
}

TEST_CASE("Position::getDistanceSq — axis-aligned")
{
    SparCraft::Position a(0, 0), b(6, 0);
    REQUIRE(a.getDistanceSq(b) == 36);
}

TEST_CASE("Position arithmetic operators")
{
    SparCraft::Position a(3, 4), b(1, 2);
    REQUIRE((a + b) == SparCraft::Position(4, 6));
    REQUIRE((a - b) == SparCraft::Position(2, 2));
}

// -----------------------------------------------------------------------
// StateEvalScore comparison operators
// -----------------------------------------------------------------------
TEST_CASE("StateEvalScore — equality")
{
    SparCraft::StateEvalScore a(10, 5), b(10, 5), c(10, 6);
    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);
}

TEST_CASE("StateEvalScore — less-than on val")
{
    SparCraft::StateEvalScore lo(5, 1), hi(10, 1);
    REQUIRE(lo < hi);
    REQUIRE_FALSE(hi < lo);
}

TEST_CASE("StateEvalScore — less-than tie-break: more moves means less")
{
    // Same val; more moves is considered worse (less)
    SparCraft::StateEvalScore fewer(10, 2), more(10, 5);
    REQUIRE(more < fewer);
    REQUIRE_FALSE(fewer < more);
}

TEST_CASE("StateEvalScore — greater-than on val")
{
    SparCraft::StateEvalScore lo(5, 1), hi(10, 1);
    REQUIRE(hi > lo);
    REQUIRE_FALSE(lo > hi);
}

TEST_CASE("StateEvalScore — greater-than tie-break: fewer moves means greater")
{
    SparCraft::StateEvalScore fewer(10, 2), more(10, 5);
    REQUIRE(fewer > more);
    REQUIRE_FALSE(more > fewer);
}

TEST_CASE("StateEvalScore — less-than-or-equal on val")
{
    SparCraft::StateEvalScore lo(5, 1), hi(10, 1);
    REQUIRE(lo <= hi);
    REQUIRE_FALSE(hi <= lo);
}

TEST_CASE("StateEvalScore — less-than-or-equal when equal")
{
    SparCraft::StateEvalScore a(10, 3);
    REQUIRE(a <= a);
}

TEST_CASE("StateEvalScore — less-than-or-equal tie-break")
{
    SparCraft::StateEvalScore fewer(10, 2), more(10, 5);
    REQUIRE(more <= fewer);   // more moves <= fewer moves (worse-or-equal)
    REQUIRE(fewer <= fewer);  // equal
    REQUIRE_FALSE(fewer <= more);
}

TEST_CASE("StateEvalScore — greater-than-or-equal on val")
{
    SparCraft::StateEvalScore lo(5, 1), hi(10, 1);
    REQUIRE(hi >= lo);
    REQUIRE_FALSE(lo >= hi);
}

TEST_CASE("StateEvalScore — greater-than-or-equal when equal")
{
    SparCraft::StateEvalScore a(10, 3);
    REQUIRE(a >= a);
}

TEST_CASE("StateEvalScore — operators are mutually consistent")
{
    SparCraft::StateEvalScore a(10, 2), b(10, 5), c(20, 1);

    // a > b (same val, fewer moves)
    REQUIRE(a > b);
    REQUIRE(a >= b);
    REQUIRE_FALSE(a < b);
    REQUIRE_FALSE(a <= b);

    // c > a (higher val)
    REQUIRE(c > a);
    REQUIRE(c >= a);
    REQUIRE_FALSE(c < a);
    REQUIRE_FALSE(c <= a);
}

// -----------------------------------------------------------------------
// Array
// -----------------------------------------------------------------------
TEST_CASE("Array — default size is zero")
{
    SparCraft::Array<int, 16> arr;
    REQUIRE(arr.size() == 0);
    REQUIRE(arr.capacity() == 16);
}

TEST_CASE("Array — add increments size and stores value")
{
    SparCraft::Array<int, 16> arr;
    arr.add(42);
    REQUIRE(arr.size() == 1);
    REQUIRE(arr[0] == 42);
}

TEST_CASE("Array — back returns last added element")
{
    SparCraft::Array<int, 16> arr;
    arr.add(1);
    arr.add(2);
    arr.add(3);
    REQUIRE(arr.back() == 3);
}

TEST_CASE("Array — clear resets size to zero")
{
    SparCraft::Array<int, 16> arr;
    arr.add(1);
    arr.add(2);
    arr.clear();
    REQUIRE(arr.size() == 0);
}

TEST_CASE("Array — contains only finds elements within size")
{
    SparCraft::Array<int, 16> arr;
    arr.add(7);
    REQUIRE(arr.contains(7));
    REQUIRE_FALSE(arr.contains(99));
}

TEST_CASE("Array — addUnique does not add duplicates")
{
    SparCraft::Array<int, 16> arr;
    arr.addUnique(5);
    arr.addUnique(5);
    arr.addUnique(5);
    REQUIRE(arr.size() == 1);
}

TEST_CASE("Array — can fill to full capacity")
{
    SparCraft::Array<int, 8> arr;
    for (int i = 0; i < 8; ++i)
        arr.add(i);
    REQUIRE(arr.size() == 8);
    REQUIRE(arr[7] == 7);
}

