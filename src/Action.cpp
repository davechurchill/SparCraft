#include "Action.h"

using namespace SparCraft;

Action::Action()
	: m_unit(255)
	, m_player(255)
	, m_moveType(ActionTypes::NONE)
	, m_moveIndex(255)
{

}

Action::Action( const size_t & unitIndex, const size_t & player, const size_t & type, const size_t & moveIndex, const Position & dest)
    : m_unit(unitIndex)
    , m_player(player)
    , m_moveType(type)
    , m_moveIndex(moveIndex)
    , m_p(dest)
{
        
}

Action::Action( const size_t & unitIndex, const size_t & player, const size_t & type, const size_t & moveIndex)
	: m_unit(unitIndex)
	, m_player(player)
	, m_moveType(type)
	, m_moveIndex(moveIndex)
{
		
}

const bool Action::operator == (const Action & rhs)
{
	return m_unit == rhs.m_unit && m_player == rhs.m_player && m_moveType == rhs.m_moveType && m_moveIndex == rhs.m_moveIndex && m_p == rhs.m_p;
}

const size_t & Action::unit() const	
{ 
    return m_unit; 
}

const size_t & Action::player() const	
{ 
    return m_player; 
}

const size_t & Action::type() const	
{ 
    return m_moveType; 
}

const size_t & Action::index() const	
{ 
    return m_moveIndex; 
}

const Position & Action::pos() const   
{ 
    return m_p; 
}

const std::string Action::moveString() const
{
	if (m_moveType == ActionTypes::ATTACK) 
	{
		return "ATTACK";
	}
	else if (m_moveType == ActionTypes::MOVE)
	{
		return "MOVE";
	}
	else if (m_moveType == ActionTypes::RELOAD)
	{
		return "RELOAD";
	}
	else if (m_moveType == ActionTypes::PASS)
	{
		return "PASS";
	}
	else if (m_moveType == ActionTypes::HEAL)
	{
		return "HEAL";
	}

	return "NONE";
}

const Position Action::getDir() const
{
	return Position(Constants::Move_Dir[m_moveIndex][0], Constants::Move_Dir[m_moveIndex][1]);
}

const std::string Action::debugString() const
{
    std::stringstream ss;
    ss << moveString() << ": (" << (int)unit() << "," << (int)player() << "," << (int)type() << "," << (int)index() << ")  " << "(" << pos().x() << "," << pos().y()   << ")";
    return ss.str();
}

