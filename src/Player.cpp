#include "Player.h"

using namespace SparCraft;


void Player::getMoves(GameState & state, const MoveArray & moves, std::vector<Action> & moveVec)
{
	// not implemented
}

const size_t Player::ID() 
{ 
	return m_playerID; 
}

void Player::setID(const size_t playerID)
{
	m_playerID = playerID;
}

