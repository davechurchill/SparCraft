#pragma once

#include <limits>

#include "Timer.hpp"
#include "GameState.h"
#include "Action.h"
#include "GraphViz.hpp"
#include "Array.hpp"
#include "MoveArray.hpp"
#include "UCTSearchParameters.hpp"
#include "UCTSearchResults.hpp"
#include "Player.h"
#include "AllPlayers.h"
#include "UCTNode.h"
#include "GraphViz.hpp"
#include "UCTMemoryPool.hpp"
#include <memory>

namespace SparCraft
{

class Game;
class Player;

class UCTSearch
{
	UCTSearchParameters 	m_params;
    UCTSearchResults        m_results;
	Timer		            m_searchTimer;
    UCTNode                 m_rootNode;
    UCTMemoryPool *         m_memoryPool;

    GameState               m_currentState;

	// we will use these as variables to save stack allocation every time
    std::vector<Action>                 m_actionVec;
	MoveArray                               m_moveArray;
	Array<std::vector<Action>,
		 Constants::Max_Ordered_Moves>      m_orderedMoves;

    std::vector<PlayerPtr>					m_allScripts[Constants::Num_Players];
    PlayerPtr                               m_playerModels[Constants::Num_Players];

public:

	UCTSearch(const UCTSearchParameters & params);

    
    // UCT-specific functions
    UCTNode &       UCTNodeSelect(UCTNode & parent);
    StateEvalScore  traverse(UCTNode & node, GameState & currentState);
	void            uct(GameState & state, size_t depth, const size_t lastPlayerToMove, std::vector<Action> * firstSimMove);

	void            doSearch(GameState & initialState, std::vector<Action> & move);
    
    // Move and Child generation functions
    void            generateChildren(UCTNode & node, GameState & state);
	void            generateOrderedMoves(GameState & state, MoveArray & moves, const size_t playerToMove);
    void            makeMove(UCTNode & node, GameState & state);
	const bool      getNextMove(size_t playerToMove, MoveArray & moves, const size_t moveNumber, std::vector<Action> & actionVec);

    // Utility functions
	const size_t    getPlayerToMove(UCTNode & node, const GameState & state) const;
    const size_t    getChildNodeType(UCTNode & parent, const GameState & prevState) const;
	const bool      searchTimeOut();
	const bool      isRoot(UCTNode & node) const;
	const bool      terminalState(GameState & state, const size_t depth) const;
    const bool      isFirstSimMove(UCTNode & node, GameState & state);
    const bool      isSecondSimMove(UCTNode & node, GameState & state);
    StateEvalScore  performPlayout(GameState & state);
    void            updateState(UCTNode & node, GameState & state, bool isLeaf);
    void            setMemoryPool(UCTMemoryPool * pool);
    UCTSearchResults & getResults();

    // graph printing functions
    void            printSubTree(UCTNode & node, GameState state, std::string filename);
    void            printSubTreeGraphViz(UCTNode & node, GraphViz::Graph & g, GameState state);
    std::string     getNodeIDString(UCTNode & node);
};
}

