#pragma once

#include <limits>

#include "AllPlayers.h"
#include "Timer.hpp"
#include "GameState.h"
#include "Action.h"
#include "Array.hpp"
#include "MoveArray.hpp"
#include "TranspositionTable.h"
#include "Player.h"

#include "AlphaBetaSearchResults.hpp"
#include "AlphaBetaSearchParameters.hpp"
#include "GraphViz.hpp"

namespace SparCraft
{

class Game;
class AlphaBetaSearchParameters;
class Player;


class AlphaBetaSearch
{
	AlphaBetaSearchParameters               m_params;
	AlphaBetaSearchResults                  m_results;
	SparCraft::Timer                        m_searchTimer;

	size_t                                  m_currentRootDepth;

	Array<MoveArray, 
          Constants::Max_Search_Depth + 1>      m_allMoves;

	Array2D<std::vector<Action>, 
			Constants::Max_Search_Depth + 1, 
			Constants::Max_Ordered_Moves>   m_orderedMoves;

    std::vector<PlayerPtr>					m_allScripts[Constants::Num_Players];
    PlayerPtr                               m_playerModels[Constants::Num_Players];

	TTPtr                                   _TT;

public:

	AlphaBetaSearch(const AlphaBetaSearchParameters & params, TTPtr TT = TTPtr((TranspositionTable *)NULL));

	void doSearch(GameState & initialState);

	// search functions
	AlphaBetaValue IDAlphaBeta(GameState & initialState, const size_t maxDepth);
	AlphaBetaValue alphaBeta(GameState & state, size_t depth, const size_t lastPlayerToMove, std::vector<Action> * firstSimMove, StateEvalScore alpha, StateEvalScore beta);

	// Transposition Table
	TTLookupValue TTlookup(const GameState & state, StateEvalScore & alpha, StateEvalScore & beta, const size_t depth);
	void TTsave(GameState & state, const StateEvalScore & value, const StateEvalScore & alpha, const StateEvalScore & beta, const size_t depth, 
				const size_t firstPlayer, const AlphaBetaMove & bestFirstMove, const AlphaBetaMove & bestSecondMove);

	// Transposition Table look up + alpha/beta update

	// get the results from the search
	AlphaBetaSearchResults & getResults();
    	
	void generateOrderedMoves(GameState & state, MoveArray & moves, const TTLookupValue & TTval, const size_t playerToMove, const size_t depth);
	const size_t getEnemy(const size_t player) const;
	const size_t getPlayerToMove(GameState & state, const size_t depth, const size_t lastPlayerToMove, const bool isFirstSimMove) const;
	bool getNextMoveVec(size_t playerToMove, MoveArray & moves, const size_t moveNumber, const TTLookupValue & TTval, const size_t depth, std::vector<Action> & moveVec) const;
	const AlphaBetaMove & getAlphaBetaMove(const TTLookupValue & TTval, const size_t playerToMove) const;
	const bool searchTimeOut();
	const bool isRoot(const size_t depth) const;
	const bool terminalState(GameState & state, const size_t depth) const;
	const bool isTranspositionLookupState(GameState & state, const std::vector<Action> * firstSimMove) const;

};
}
