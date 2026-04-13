#pragma once

#include "Common.h"
#include "Position.hpp"

namespace SparCraft
{

namespace ActionTypes
{
	enum {NONE, ATTACK, RELOAD, MOVE, PASS, HEAL};
};

class Action 
{
	size_t  m_unit;
	size_t	m_player;
	size_t	m_moveType;
	size_t	m_moveIndex;

    Position m_p;

public:


	Action();

    Action( const size_t & unitIndex, const size_t & player, const size_t & type, const size_t & moveIndex, const Position & dest);

	Action( const size_t & unitIndex, const size_t & player, const size_t & type, const size_t & moveIndex);

	const bool operator == (const Action & rhs);

	const size_t & unit()	const;
	const size_t & player() const;
	const size_t & type()	const;
	const size_t & index()	const;
    const Position & pos()  const;

	const std::string moveString() const;

	const Position getDir() const;

    const std::string debugString() const;

};

}

