#pragma once

#include "Common.h"
#include "Player.h"
#include "PortfolioGreedySearch.h"

namespace SparCraft
{
class Player_PortfolioGreedySearch : public Player
{
	size_t m_seed;
	size_t m_iterations;
    size_t m_responses;
    size_t m_timeLimit;
public:
	Player_PortfolioGreedySearch (const size_t & playerID);
    Player_PortfolioGreedySearch (const size_t & playerID, const size_t & seed, const size_t & iter, const size_t & responses, const size_t & timeLimit);
	void getMoves(GameState & state, const MoveArray & moves, std::vector<Action> & moveVec);
    size_t getType() { return PlayerModels::PortfolioGreedySearch; }
};
}

