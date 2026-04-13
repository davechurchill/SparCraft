#pragma once

#include "Common.h"
#include "Unit.h"
#include "Action.h"
#include <algorithm>
#include <random>
#include <vector>

namespace SparCraft
{
class MoveArray
{
    // Move storage is heap-backed to avoid very large stack allocations.
    std::vector<std::vector<Action>>    m_moves;
    std::vector<Action>                 m_currentMoves;
    std::vector<size_t>                 m_currentMovesIndex;

	// the number of units that have moves;
	size_t                              m_numUnits;
    bool                                m_hasMoreMoves;

    void ensureUnitStorage(const size_t unit)
    {
        const size_t requiredSize = unit + 1;

        if (m_moves.size() < requiredSize)
        {
            m_moves.resize(requiredSize);
        }

        if (m_currentMoves.size() < requiredSize)
        {
            m_currentMoves.resize(requiredSize);
        }

        if (m_currentMovesIndex.size() < requiredSize)
        {
            m_currentMovesIndex.resize(requiredSize, 0);
        }
    }

public:

    MoveArray()
        : m_numUnits(0)
        , m_hasMoreMoves(false)
    {
        m_moves.reserve(Constants::Max_Units);
        m_currentMoves.reserve(Constants::Max_Units);
        m_currentMovesIndex.reserve(Constants::Max_Units);
    }

    void clear()
    {
        // only clear things if they need to be cleared
        if (m_numUnits == 0)
        {
            return;
        }

        m_numUnits = 0;
        for (size_t u(0); u<m_moves.size(); ++u)
        {
            m_moves[u].clear();
        }

        m_currentMoves.clear();
        m_currentMovesIndex.clear();
        resetMoveIterator();
    }

    // shuffle the MOVE unit actions to prevent bias in experiments
    // this function assumes that all MOVE actions are contiguous in the moves array
    // this should be the case unless you change the move generation ordering
    void shuffleMoveActions()
    {
        static std::mt19937 rng(std::random_device{}());

        // for each unit
        for (size_t u(0); u<numUnits(); ++u)
        {
            int moveEnd(-1);
            int moveBegin(-1);

            // reverse through the list of actions for this unit
            for (int a((int)numMoves(u)-1); a >= 0; --a)
            {
                size_t moveType(getMove(u, a).type());

                // mark the end of the move actions
                if (moveEnd == -1 && (moveType == ActionTypes::MOVE))
                {
                    moveEnd = a;
                }
                // mark the beginning of the MOVE unit actions
                else if ((moveEnd != -1) && (moveBegin == -1) && (moveType != ActionTypes::MOVE))
                {
                    moveBegin = a + 1;
                }
                else if (moveBegin != -1)
                {
                    break;
                }
            }

            // if we found the end but didn't find the beginning
            if (moveEnd != -1 && moveBegin == -1)
            {
                // then the move actions begin at the beginning of the array
                moveBegin = 0;
            }

            // shuffle the movement actions for this unit
            if (moveEnd != -1 && moveBegin != -1 && moveEnd != moveBegin)
            {
                std::shuffle(m_moves[u].begin() + moveBegin, m_moves[u].begin() + moveEnd + 1, rng);
                resetMoveIterator();
            }
        }
    }

    // returns a given move from a unit
    const Action & getMove(size_t unit, size_t move) const
    {
        assert(unit < m_moves.size());
        assert(move < m_moves[unit].size());
        assert(m_moves[unit][move].unit() != 255);
        return m_moves[unit][move];
    }

    void printCurrentMoveIndex()
    {
        for (size_t u(0); u<m_numUnits; ++u)
        {
            std::cout << m_currentMovesIndex[u] << " ";
        }

        std::cout << std::endl;
    }

    void incrementMove(size_t unit)
    {
        if (unit >= m_numUnits || numMoves(unit) == 0)
        {
            m_hasMoreMoves = false;
            return;
        }

        // increment the index for this unit
        m_currentMovesIndex[unit] = (m_currentMovesIndex[unit] + 1) % numMoves(unit);

        // if the value rolled over, we need to do the carry calculation
        if (m_currentMovesIndex[unit] == 0)
        {
            // the next unit index
            size_t nextUnit = unit + 1;

            // if we have space left to increment, do it
            if (nextUnit < m_numUnits)
            {
                incrementMove(nextUnit);
            }
            // otherwise we have no more moves
            else
            {
                m_hasMoreMoves = false;
            }
        }

        m_currentMoves[unit] = m_moves[unit][m_currentMovesIndex[unit]];
    }

    bool hasMoreMoves() const
    {
        return m_hasMoreMoves;
    }

    void resetMoveIterator()
    {
        m_hasMoreMoves = (m_numUnits > 0);
        m_currentMovesIndex.assign(m_numUnits, 0);
        m_currentMoves.resize(m_numUnits);

        for (size_t u(0); u<numUnits(); ++u)
        {
            if (numMoves(u) == 0)
            {
                m_hasMoreMoves = false;
                continue;
            }

            m_currentMoves[u] = m_moves[u][m_currentMovesIndex[u]];
        }
    }

    void getNextMoveVec(std::vector<Action> & moves)
    {
        if (!m_hasMoreMoves || m_numUnits == 0)
        {
            moves.clear();
            return;
        }

        moves.assign(m_currentMoves.begin(), m_currentMoves.begin() + m_numUnits);
        incrementMove(0);
    }

    size_t maxUnits() const
    {
        return std::max(static_cast<size_t>(Constants::Max_Units), m_moves.size());
    }

    // adds a Move to the unit specified
    void add(const Action & move)
    {
        ensureUnitStorage(move.unit());
        m_moves[move.unit()].push_back(move);

        if (move.unit() + 1 > m_numUnits)
        {
            m_numUnits = move.unit() + 1;
        }

        m_currentMovesIndex[move.unit()] = 0;
        m_currentMoves[move.unit()] = m_moves[move.unit()][0];
    }

    bool validateMoves()
    {
        for (size_t u(0); u<numUnits(); ++u)
        {
            for (size_t m(0); m<numMoves(u); ++m)
            {
                const Action & move(getMove(u, m));

                if (move.unit() >= Constants::Max_Units)
                {
                    printf("Unit Move Incorrect! Something will be wrong\n");
                    return false;
                }
            }
        }

        return true;
    }

    size_t getUnitID(size_t unit)   const { return getMove(unit, 0).unit(); }
    size_t getPlayerID(size_t unit) const { return getMove(unit, 0).player(); }

    void addUnit()
    {
        ensureUnitStorage(m_numUnits);
        m_moves[m_numUnits].clear();
        m_currentMovesIndex[m_numUnits] = 0;
        m_numUnits++;
    }

    size_t numUnits()              const  { return m_numUnits; }
    size_t numUnitsInTuple()       const  { return numUnits(); }
    size_t numMoves(size_t unit)   const  { return m_moves[unit].size(); }
};
}
