#include "Player_UCT.h"

using namespace SparCraft;

Player_UCT::Player_UCT (const size_t & playerID, const UCTSearchParameters & params) 
{
	m_playerID = playerID;
    m_params = params;
}

void Player_UCT::getMoves(GameState & state, const MoveArray & moves, std::vector<Action> & moveVec)
{
    moveVec.clear();
    
    UCTSearch uct(m_params);

    uct.doSearch(state, moveVec);
    m_prevResults = uct.getResults();
}

UCTSearchParameters & Player_UCT::getParams()
{
    return m_params;
}

UCTSearchResults & Player_UCT::getResults()
{
    return m_prevResults;
}

