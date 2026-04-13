#include "UnitScriptData.h"

using namespace SparCraft;

UnitScriptData::UnitScriptData() 
{
}

std::vector<Action> & UnitScriptData::getMoves(const size_t player, const size_t actualScript)
{
    return m_allScriptMoves[player][actualScript];
}

Action & UnitScriptData::getMove(const size_t player, const size_t unitIndex, const size_t actualScript)
{
    return m_allScriptMoves[player][actualScript][unitIndex];
}

void UnitScriptData::calculateMoves(const size_t player, MoveArray & moves, GameState & state, std::vector<Action> & moveVec)
{
    // generate all script moves for this player at this state and store them in allScriptMoves
    for (size_t scriptIndex(0); scriptIndex<m_scriptVec[player].size(); ++scriptIndex)
    {
        // get the associated player pointer
        const PlayerPtr & pp = getPlayerPtr(player, scriptIndex);

        // get the actual script we are working with
        const size_t actualScript = getScript(player, scriptIndex);

        // generate the moves inside the appropriate vector
        getMoves(player, actualScript).clear();
        pp->getMoves(state, moves, getMoves(player, actualScript));
    }

    // for each unit the player has to move, populate the move vector with the appropriate script move
    for (size_t unitIndex(0); unitIndex < moves.numUnits(); ++unitIndex)
    {
        // the unit from the state
        const Unit & unit = state.getUnit(player, unitIndex);

        // the move it would choose to do based on its associated script preference
        Action unitMove = getMove(player, unitIndex, getUnitScript(unit));

        // put the unit into the move vector
        moveVec.push_back(unitMove);
    }
}

const size_t UnitScriptData::getUnitScript(const size_t player, const int id) const
{
    return (*m_unitScriptMap[player].find(id)).second;
}
    
const size_t UnitScriptData::getUnitScript(const Unit & unit) const
{
    return getUnitScript(unit.player(), (int)unit.ID());
}

const size_t UnitScriptData::getScript(const size_t player, const size_t index)
{
    return m_scriptVec[player][index];
}

const PlayerPtr & UnitScriptData::getPlayerPtr(const size_t player, const size_t index)
{
    return m_playerPtrVec[player][index];
}

const size_t UnitScriptData::getNumScripts(const size_t player) const
{
    return m_scriptSet[player].size();
}

void UnitScriptData::setUnitScript(const size_t player, const int id, const size_t script)
{
    if (m_scriptSet[player].find(script) == m_scriptSet[player].end())
    {
        m_scriptSet[player].insert(script);
        m_scriptVec[player].push_back(script);
        m_playerPtrVec[player].push_back(PlayerPtr(AllPlayers::getPlayerPtr(player, script)));
    }
        
    m_unitScriptMap[player][id] = script;
}

void UnitScriptData::setUnitScript(const Unit & unit, const size_t script)
{
    setUnitScript(unit.player(), (int)unit.ID(), script);
}

