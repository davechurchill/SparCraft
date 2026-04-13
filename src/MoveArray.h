#pragma once

#include "Common.h"
#include "Array.hpp"
#include "Unit.h"
#include "Action.h"

namespace SparCraft
{
class MoveArray
{
	// the array which contains all the moves
	Array2D<Action, Constants::Max_Units, Constants::Max_Moves> m_moves;

	// how many moves each unit has
	Array<size_t, Constants::Max_Units>                         m_numMoves;

    // the current move array, used for the 'iterator'
    //std::vector<Action> m_currentMoves;
    //std::vector<Action>                                             m_currentMovesVec;
    Array<Action, Constants::Max_Units>                         m_currentMoves;
    Array<size_t, Constants::Max_Units>                         m_currentMovesIndex;

	// the number of units that have moves;
	size_t                                                      m_numUnits;
	size_t                                                              m_maxUnits;
    bool                                                                m_hasMoreMoves;

public:

	MoveArray(const size_t maxUnits = 0);

	void clear();

	// returns a given move from a unit
	const Action & getMove(const size_t & unit, const size_t & move) const;

    void printCurrentMoveIndex();

    void incrementMove(const size_t & unit);

    const bool hasMoreMoves() const;

    void resetMoveIterator();

    void getNextMoveVec(std::vector<Action> & moves);

	const size_t maxUnits() const;

	// adds a Move to the unit specified
	void add(const Action & move);
	
	bool validateMoves();

	const size_t getUnitID(const size_t & unit) const;

	const size_t getPlayerID(const size_t & unit) const;

	void addUnit();

    void shuffleMoveActions();

	const size_t & numUnits()						const;
	const size_t & numUnitsInTuple()				const;
	const size_t & numMoves(const size_t & unit)	const;
};
}

