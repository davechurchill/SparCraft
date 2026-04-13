#include "Player_AlphaBeta.h"

using namespace SparCraft;

Player_AlphaBeta::Player_AlphaBeta (const size_t & playerID)
    : alphaBeta(nullptr)
{
	m_playerID = playerID;
}

Player_AlphaBeta::Player_AlphaBeta (const size_t & playerID, const AlphaBetaSearchParameters & params, TTPtr table)
    : alphaBeta(nullptr)
{
	m_playerID = playerID;
	m_params = params;
	TT = table;

    alphaBeta = new AlphaBetaSearch(m_params, TT);
}

Player_AlphaBeta::~Player_AlphaBeta()
{
    delete alphaBeta;
}

AlphaBetaSearchResults & Player_AlphaBeta::results()
{
	return alphaBeta->getResults();
}

AlphaBetaSearchParameters & Player_AlphaBeta::getParams()
{
	return m_params;
}

void Player_AlphaBeta::setParameters(AlphaBetaSearchParameters & p)
{
	m_params = p;
}

void Player_AlphaBeta::setTranspositionTable(TTPtr table)
{
	TT = table ? table : TTPtr(new TranspositionTable());
    delete alphaBeta;
    alphaBeta = new AlphaBetaSearch(m_params, TT);
}

void Player_AlphaBeta::getMoves(GameState & state, const MoveArray & moves, std::vector<Action> & moveVec)
{
    moveVec.clear();
	alphaBeta->doSearch(state);
    moveVec.assign(alphaBeta->getResults().bestMoves.begin(), alphaBeta->getResults().bestMoves.end());
}

