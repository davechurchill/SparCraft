#pragma once

#include "Common.h"
#include <algorithm>
#include "MoveArray.h"
#include "Hash.h"
#include "Map.hpp"
#include "Unit.h"
#include "GraphViz.hpp"
#include "Array.hpp"
#include <memory>

typedef std::shared_ptr<SparCraft::Map> MapPtr;

namespace SparCraft
{
class GameState 
{
    Map *                                                           m_map;               

    std::vector<Unit> m_units[Constants::Num_Players];
    std::vector<int>  m_unitIndex[Constants::Num_Players];

    Array<Unit, 1>                                                  m_neutralUnits;

    Array<size_t, Constants::Num_Players>                    m_numUnits;
    Array<size_t, Constants::Num_Players>                    m_prevNumUnits;

    Array<float, Constants::Num_Players>                            m_totalLTD;
    Array<float, Constants::Num_Players>                            m_totalSumSQRT;

    Array<int, Constants::Num_Players>                              m_numMovements;
    Array<int, Constants::Num_Players>                              m_prevHPSum;
	
    TimeType                                                        m_currentTime;
    size_t                                                          m_maxUnits;
    TimeType                                                        m_turnsWithNoHPChange;

    // checks to see if the unit array is full before adding a unit to the state
    void                    checkFull(const size_t player)                                        const;
    const bool              checkUniqueUnitIDs()                                                    const;

    void                    performAction(const Action & theMove);

public:

    GameState();

	// misc functions
    void                    finishedMoving();
    void                    updateGameTime();
    const bool              playerDead(const size_t player)                                       const;
    const bool              isTerminal()                                                            const;

    // unit data functions
    const size_t            numUnits(const size_t player)                                         const;
    const size_t            prevNumUnits(const size_t player)                                     const;
    const size_t            numNeutralUnits()                                                       const;
    const size_t            closestEnemyUnitDistance(const Unit & unit)                             const;

    // Unit functions
    void                    sortUnits();
    void                    addUnit(const Unit & u);
    void                    addUnit(const BWAPI::UnitType unitType, const size_t playerID, const Position & pos);
    void                    addUnitWithID(const Unit & u);
    void                    addNeutralUnit(const Unit & unit);
    const Unit &            getUnit(const size_t player, const size_t unitIndex)         const;
    const Unit &            getUnitByID(const size_t unitID)                                      const;
          Unit &            getUnit(const size_t player, const size_t unitIndex);
    const Unit &            getUnitByID(const size_t player, const size_t unitID)               const;
          Unit &            getUnitByID(const size_t player, const size_t unitID);
    const Unit &            getClosestEnemyUnit(const size_t player, const size_t unitIndex, bool checkCloaked=false);
    const Unit &            getClosestOurUnit(const size_t player, const size_t unitIndex);
    const Unit &            getUnitDirect(const size_t player, const size_t unit)               const;
    const Unit &            getNeutralUnit(const size_t u)                                        const;
    
    // game time functions
    void                    setTime(const TimeType time);
    const TimeType          getTime()                                                               const;

    // evaluation functions
    const StateEvalScore    eval(   const size_t player, const size_t evalMethod, 
                                    const size_t p1Script = PlayerModels::NOKDPS,
                                    const size_t p2Script = PlayerModels::NOKDPS)                   const;
    const ScoreType         evalLTD(const size_t player)                                        const;
    const ScoreType         evalLTD2(const size_t player)                                       const;
    const ScoreType         LTD(const size_t player)                                            const;
    const ScoreType         LTD2(const size_t player)                                           const;
    const StateEvalScore    evalSim(const size_t player, const size_t p1, const size_t p2)    const;
    const size_t            getEnemy(const size_t player)                                         const;

    // unit hitpoint calculations, needed for LTD2 evaluation
    void                    calculateStartingHealth();
    void                    setTotalLTD(const float p1, const float p2);
    void                    setTotalLTD2(const float p1, const float p2);
    const float           getTotalLTD(const size_t player)                                    const;
    const float           getTotalLTD2(const size_t player)                                   const;

    // move related functions
    void                    generateMoves(MoveArray & moves, const size_t playerIndex)            const;
    void                    makeMoves(const std::vector<Action> & moves, const bool ignoreCanMoveCheck = false);
    const int             getNumMovements(const size_t player)                                  const;
    const size_t            whoCanMove()                                                            const;
    const bool              bothCanMove()                                                           const;
		  
    // map-related functions
    void                    setMap(Map * map);
    Map *                   getMap()                                                                const;
    const bool              isWalkable(const Position & pos)                                        const;
    const bool              isFlyable(const Position & pos)                                         const;

    // hashing functions
    const HashType          calculateHash(const size_t hashNum)                                   const;

    // state i/o functions
    void                    print(int indent = 0) const;
	std::string             toString() const;
    std::string             toStringCompact() const;
};

}

