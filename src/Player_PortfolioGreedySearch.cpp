#include "Player_PortfolioGreedySearch.h"

using namespace SparCraft;

Player_PortfolioGreedySearch::Player_PortfolioGreedySearch (const size_t & playerID) 
{
	m_playerID = playerID;
	m_iterations = 1;
    m_responses = 0;
	m_seed = PlayerModels::NOKDPS;
}

Player_PortfolioGreedySearch::Player_PortfolioGreedySearch (const size_t & playerID, const size_t & seed, const size_t & iter, const size_t & responses, const size_t & timeLimit)
{
	m_playerID = playerID;
	m_iterations = iter;
    m_responses = responses;
	m_seed = seed;
    m_timeLimit = timeLimit;
}

void Player_PortfolioGreedySearch::getMoves(GameState & state, const MoveArray & moves, std::vector<Action> & moveVec)
{
    moveVec.clear();
	PortfolioGreedySearch pgs(m_playerID, m_seed, m_iterations, m_responses, m_timeLimit);

	moveVec = pgs.search(m_playerID, state);
}

