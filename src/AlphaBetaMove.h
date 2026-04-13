#pragma once

#include "Common.h"
#include "Position.hpp"
#include "Action.h"

namespace SparCraft
{

class AlphaBetaMove
{
	std::vector<Action> m_move;
	bool m_isValid;

public:

	AlphaBetaMove();

	AlphaBetaMove(const std::vector<Action> & move, const bool & isValid);

	const bool isValid() const;
	const std::vector<Action> & moveVec() const;
};

class TTBestMove
{
	AlphaBetaMove m_firstMove;
	AlphaBetaMove m_secondMove;

public:

	TTBestMove();

	TTBestMove(const AlphaBetaMove & first, const AlphaBetaMove & second);

	const AlphaBetaMove & firstMove() const;
	const AlphaBetaMove & secondMove() const;
};


class AlphaBetaValue
{	
	StateEvalScore	m_score;
	AlphaBetaMove	m_move;

public:

	AlphaBetaValue();

	AlphaBetaValue(const StateEvalScore & score, const AlphaBetaMove & abMove);

	const StateEvalScore & score() const;
	const AlphaBetaMove & abMove() const;
};
}

