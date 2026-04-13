#include "MoveArray.h"
#include <algorithm>
#include <random>

using namespace SparCraft;

MoveArray::MoveArray(const size_t maxUnits) 
	: m_numUnits(0)
	, m_maxUnits(Constants::Max_Units)
    , m_hasMoreMoves(true)
{
    //m_currentMovesVec.reserve(Constants::Max_Units);
	m_numMoves.fill(0);
    m_currentMovesIndex.fill(0);
}
    
void MoveArray::clear() 
{
    // only clear things if they need to be cleared
    if (m_numUnits == 0)
    {
        return;
    }

	m_numUnits = 0;
	m_numMoves.fill(0);
    resetMoveIterator();
}

// shuffle the MOVE unit actions to prevent bias in experiments
// this function assumes that all MOVE actions are contiguous in the moves array
// this should be the case unless you change the move generation ordering
void MoveArray::shuffleMoveActions()
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
                moveBegin = a;
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
            std::shuffle(&m_moves[u][moveBegin], &m_moves[u][moveEnd + 1], rng);
            resetMoveIterator();
        }
    }
}

// returns a given move from a unit
const Action & MoveArray::getMove(const size_t unit, const size_t move) const
{
    assert(m_moves[unit][(size_t)move].unit() != 255);

    return m_moves[unit][(size_t)move];
}

void MoveArray::printCurrentMoveIndex()
{
    for (size_t u(0); u<m_numUnits; ++u)
    {
        std::cout << m_currentMovesIndex[u] << " ";
    }

    std::cout << std::endl;
}

void MoveArray::incrementMove(const size_t unit)
{
    // increment the index for this unit
    m_currentMovesIndex[unit] = (m_currentMovesIndex[unit] + 1) % m_numMoves[unit];

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
            // stop
            m_hasMoreMoves = false;
        }
    }

    m_currentMoves[unit] = m_moves[unit][m_currentMovesIndex[unit]];
    //m_currentMovesVec[unit] = m_moves[unit][m_currentMovesIndex[unit]];
}

const bool MoveArray::hasMoreMoves() const
{
    return m_hasMoreMoves;
}

void MoveArray::resetMoveIterator()
{
    m_hasMoreMoves = true;
    m_currentMovesIndex.fill(0);

    for (size_t u(0); u<numUnits(); ++u)
    {
        m_currentMoves[u] = m_moves[u][m_currentMovesIndex[u]];
        //m_currentMovesVec[u] = m_moves[u][m_currentMovesIndex[u]];
    }
}

void MoveArray::getNextMoveVec(std::vector<Action> & moves)
{
    moves.assign(&m_currentMoves[0], &m_currentMoves[m_numUnits]);
    //moves = m_currentMovesVec;
    incrementMove(0);
}

const size_t MoveArray::maxUnits() const
{
	return m_moves.getRows();
}

// adds a Move to the unit specified
void MoveArray::add(const Action & move)
{
	m_moves[move.unit()][m_numMoves[move.unit()]] = move;
	m_numMoves[move.unit()]++;

    m_currentMovesIndex[m_numUnits-1] = 0;
    m_currentMoves[m_numUnits-1] = m_moves[move.unit()][0];
    //m_currentMovesVec.push_back(m_moves[move.unit()][0]);
    //resetMoveIterator();
}
	
bool MoveArray::validateMoves()
{
	for (size_t u(0); u<numUnits(); ++u)
	{
		for (size_t m(0); m<numMoves(u); ++m)
		{
			const Action & move(getMove(u, m));

			if (move.unit() > 200)
			{
				printf("Unit Move Incorrect! Something will be wrong\n");
				return false;
			}
		}
	}
		
	return true;
}

const size_t MoveArray::getUnitID(const size_t unit) const
{
	return getMove(unit, 0).unit();
}

const size_t MoveArray::getPlayerID(const size_t unit) const
{
	return getMove(unit, 0).player();
}

void MoveArray::addUnit() 											{ m_numUnits++; }

const size_t MoveArray::numUnits()						const	{ return m_numUnits; }
const size_t MoveArray::numUnitsInTuple()				const	{ return numUnits(); }
const size_t MoveArray::numMoves(const size_t unit)	const	{ return m_numMoves[unit]; }

