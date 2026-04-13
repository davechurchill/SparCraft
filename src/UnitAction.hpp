#pragma once

#include "Common.h"
#include "Position.hpp"

namespace SparCraft
{

namespace UnitActionTypes
{
	enum {NONE, ATTACK, RELOAD, MOVE, PASS, HEAL};
};

class UnitAction 
{

public:

	IDType			m_unit,
					m_player,
					m_moveType,
					m_moveIndex;

    Position        m_p;

	UnitAction()
		: m_unit(255)
		, m_player(255)
		, m_moveType(UnitActionTypes::NONE)
		, m_moveIndex(255)
	{

	}

    UnitAction( const IDType & unitIndex, const IDType & player, const IDType & type, const IDType & moveIndex, const Position & dest)
        : m_unit(unitIndex)
        , m_player(player)
        , m_moveType(type)
        , m_moveIndex(moveIndex)
        , m_p(dest)
    {
        
    }

	UnitAction( const IDType & unitIndex, const IDType & player, const IDType & type, const IDType & moveIndex)
		: m_unit(unitIndex)
		, m_player(player)
		, m_moveType(type)
		, m_moveIndex(moveIndex)
	{
		
	}

	const bool operator == (const UnitAction & rhs)
	{
		return m_unit == rhs.m_unit && m_player == rhs.m_player && m_moveType == rhs.m_moveType && m_moveIndex == rhs.m_moveIndex && m_p == rhs.m_p;
	}

	const IDType & unit()	const	{ return m_unit; }
	const IDType & player() const	{ return m_player; }
	const IDType & type()	const	{ return m_moveType; }
	const IDType & index()	const	{ return m_moveIndex; }
    const Position & pos()  const   { return m_p; }

	const std::string moveString() const
	{
		if (m_moveType == UnitActionTypes::ATTACK) 
		{
			return "ATTACK";
		}
		else if (m_moveType == UnitActionTypes::MOVE)
		{
			return "MOVE";
		}
		else if (m_moveType == UnitActionTypes::RELOAD)
		{
			return "RELOAD";
		}
		else if (m_moveType == UnitActionTypes::PASS)
		{
			return "PASS";
		}
		else if (m_moveType == UnitActionTypes::HEAL)
		{
			return "HEAL";
		}

		return "NONE";
	}

	const Position getDir() const
	{
		return Position(Constants::Move_Dir[m_moveIndex][0], Constants::Move_Dir[m_moveIndex][1]);
	}

    const std::string debugString() const
    {
        std::stringstream ss;
        ss << moveString() << ": (" << (int)unit() << "," << (int)player() << "," << (int)type() << "," << (int)index() << ")  " << "(" << pos().x() << "," << pos().y()   << ")";
        return ss.str();
    }
};


class AlphaBetaMove
{
	std::vector<UnitAction> m_move;
	bool m_isValid;

public:

	AlphaBetaMove()
        : m_isValid(false)
	{
	}

	AlphaBetaMove(const std::vector<UnitAction> & move, const bool & isValid)
		: m_move(move)
		, m_isValid(isValid)
	{
	}

	const bool isValid() const { return m_isValid; }
	const std::vector<UnitAction> & moveVec() const { return m_move; }
};

class TTBestMove
{
	AlphaBetaMove m_firstMove;
	AlphaBetaMove m_secondMove;

public:

	TTBestMove()
	{
	}

	TTBestMove(const AlphaBetaMove & first, const AlphaBetaMove & second)
		: m_firstMove(first)
		, m_secondMove(second)
	{
	}

	const AlphaBetaMove & firstMove() const		{ return m_firstMove; }
	const AlphaBetaMove & secondMove() const	{ return m_secondMove; }
};


class AlphaBetaValue
{	
	StateEvalScore	m_score;
	AlphaBetaMove	m_move;

public:

	AlphaBetaValue()
	{
	}

	AlphaBetaValue(const StateEvalScore & score, const AlphaBetaMove & abMove)
		: m_score(score)
		, m_move(abMove)
	{
	}

	const StateEvalScore & score() const { return m_score; }
	const AlphaBetaMove & abMove() const { return m_move; }
};
}

