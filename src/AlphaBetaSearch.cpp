#include "AlphaBetaSearch.h"

using namespace SparCraft;

AlphaBetaSearch::AlphaBetaSearch(const AlphaBetaSearchParameters & params, TTPtr TT) 
	: m_params(params)
	, m_currentRootDepth(0)
	, _TT(TT ? TT : TTPtr(new TranspositionTable()))
{
    for (size_t p(0); p<Constants::Num_Players; ++p)
    {
        // set ordered move script player objects
        for (size_t s(0); s<m_params.getOrderedMoveScripts().size(); ++s)
        {
            m_allScripts[p].push_back(AllPlayers::getPlayerPtr(p, m_params.getOrderedMoveScripts()[s]));
        }

        // set player model objects
        if (m_params.playerModel(p) != PlayerModels::None)
        {
            m_playerModels[p] = AllPlayers::getPlayerPtr(p, m_params.playerModel(p));
        }
    }
}

void AlphaBetaSearch::doSearch(GameState & initialState)
{
	m_searchTimer.start();

	StateEvalScore alpha(-10000000, 1000000);
	StateEvalScore beta	( 10000000, 1000000);

	AlphaBetaValue val;

	if (m_params.searchMethod() == SearchMethods::AlphaBeta)
	{
		val = alphaBeta(initialState, m_params.maxDepth(), Players::Player_None, NULL, alpha, beta);
	}
	else if (m_params.searchMethod() == SearchMethods::IDAlphaBeta)
	{
		val = IDAlphaBeta(initialState, m_params.maxDepth());
	}

	m_results.timeElapsed = m_searchTimer.elapsedMS();
}

AlphaBetaValue AlphaBetaSearch::IDAlphaBeta(GameState & initialState, const size_t maxDepth)
{
	AlphaBetaValue val;
	m_results.nodesExpanded = 0;
	m_results.maxDepthReached = 0;

	for (size_t d(1); d < maxDepth; ++d)
	{
		
		StateEvalScore alpha(-10000000, 999999);
		StateEvalScore beta	( 10000000, 999999);
		
		m_results.maxDepthReached = d;
		m_currentRootDepth = d;

		// perform ID-AB until time-out
		try
		{
			val = alphaBeta(initialState, d, Players::Player_None, NULL, alpha, beta);

			m_results.bestMoves = val.abMove().moveVec();
			m_results.abValue = val.score().val();
		}
		// if we do time-out
		catch (int e)
		{
			e += 1;

			// if we didn't finish the first depth, set the move to the best script move
			if (d == 1)
			{
				MoveArray moves;
				const size_t playerToMove(getPlayerToMove(initialState, 1, Players::Player_None, true));
				initialState.generateMoves(moves, playerToMove);
				PlayerPtr bestScript(new Player_NOKDPS(playerToMove));
				bestScript->getMoves(initialState, moves, m_results.bestMoves);
			}

			break;
		}

		long long unsigned nodes = m_results.nodesExpanded;
		double ms = m_searchTimer.elapsedMS();

	}

	return val;
}

// Transposition Table save 
void AlphaBetaSearch::TTsave(	GameState & state, const StateEvalScore & value, const StateEvalScore & alpha, const StateEvalScore & beta, const size_t depth, 
						const size_t firstPlayer, const AlphaBetaMove & bestFirstMove, const AlphaBetaMove & bestSecondMove) 
{
	// IF THE DEPTH OF THE ENTRY IS BIGGER THAN CURRENT DEPTH, DO NOTHING
	TTEntry * entry = _TT->lookupScan(state.calculateHash(0), state.calculateHash(1));
	bool valid = entry && entry->isValid();
	size_t edepth = entry ? entry->getDepth() : 0;

	m_results.ttSaveAttempts++;
	
	if (valid && (edepth > depth)) 
	{
		return;
	}
	
	int type(TTEntry::NONE);

	if      (value <= alpha) type = TTEntry::UPPER;
	else if (value >= beta)  type = TTEntry::LOWER;
	else                     type = TTEntry::ACCURATE;

	// SAVE A NEW ENTRY IN THE TRANSPOSITION TABLE
	_TT->save(state.calculateHash(0), state.calculateHash(1), value, depth, type, firstPlayer, bestFirstMove, bestSecondMove);
}

// Transposition Table look up + alpha/beta update
TTLookupValue AlphaBetaSearch::TTlookup(const GameState & state, StateEvalScore & alpha, StateEvalScore & beta, const size_t depth)
{
	TTEntry * entry = _TT->lookupScan(state.calculateHash(0), state.calculateHash(1));
	if (entry && (entry->getDepth() == depth)) 
	{
		// get the value and type of the entry
		StateEvalScore TTvalue = entry->getScore();
		
		// set alpha and beta depending on the type of entry in the TT
		if (entry->getType() == TTEntry::LOWER)
		{
			if (TTvalue > alpha) 
			{
				alpha = TTvalue;
			}
		}
		else if (entry->getType() == TTEntry::UPPER) 
		{
			if (TTvalue < beta)
			{
				beta  = TTvalue;
			}
		} 
		else
		{
			printf("LOL\n");
			alpha = TTvalue;
			beta = TTvalue;
		}
		
		if (alpha >= beta) 
		{
			// this will be a cut
			m_results.ttcuts++;
			return TTLookupValue(true, true, entry);
		}
		else
		{
			// found but no cut
			m_results.ttFoundNoCut++;
			return TTLookupValue(true, false, entry);
		}
	}
	else if (entry)
	{
		m_results.ttFoundLessDepth++;
		return TTLookupValue(true, false, entry);
	}

	return TTLookupValue(false, false, entry);
}

const bool AlphaBetaSearch::searchTimeOut()
{
	return (m_params.timeLimit() && (m_results.nodesExpanded % 50 == 0) && (m_searchTimer.elapsedMS() >= m_params.timeLimit()));
}

const bool AlphaBetaSearch::terminalState(GameState & state, const size_t depth) const
{
	return (depth <= 0 || state.isTerminal());
}

const AlphaBetaMove & AlphaBetaSearch::getAlphaBetaMove(const TTLookupValue & TTval, const size_t playerToMove) const
{
	const size_t enemyPlayer(getEnemy(playerToMove));

	// if we have a valid first move for this player, use it
	if (TTval.entry()->getBestMove(playerToMove).firstMove().isValid())
	{
		return TTval.entry()->getBestMove(playerToMove).firstMove();
	}
	// otherwise return the response to an opponent move, if it doesn't exist it will just be invalid
	else
	{
		return TTval.entry()->getBestMove(enemyPlayer).secondMove();
	}
}

void AlphaBetaSearch::generateOrderedMoves(GameState & state, MoveArray & moves, const TTLookupValue & TTval, const size_t playerToMove, const size_t depth)
{
	// get the array where we will store the moves and clear it
	Array<std::vector<Action>, Constants::Max_Ordered_Moves> & orderedMoves(m_orderedMoves[depth]);
	orderedMoves.clear();

	// if we are using opponent modeling, get the move and then return, we don't want to put any more moves in
	if (m_params.playerModel(playerToMove) != PlayerModels::None)
	{
        // put the vector into the ordered moves array
        orderedMoves.add(std::vector<Action>());

        // generate the moves into that vector
        m_playerModels[playerToMove]->getMoves(state, moves, orderedMoves[0]);
		
		return;
	}

    // if we are using script move ordering, insert the script moves we want
    if (m_params.moveOrderingMethod() == MoveOrderMethod::ScriptFirst)
    {
        for (size_t s(0); s<m_params.getOrderedMoveScripts().size(); s++)
	    {
            std::vector<Action> moveVec;
		    m_allScripts[playerToMove][s]->getMoves(state, moves, moveVec);
		    orderedMoves.add(moveVec);
	    }

        if (orderedMoves.size() < 2)
        {
            int a = 6;
        }
    }
}

bool AlphaBetaSearch::getNextMoveVec(size_t playerToMove, MoveArray & moves, const size_t moveNumber, const TTLookupValue & TTval, const size_t depth, std::vector<Action> & moveVec) const
{
    if (m_params.maxChildren() && (moveNumber >= m_params.maxChildren()))
    {
        return false;
    }

    // if this move is beyond the first, check to see if we are only using a single move
    if (moveNumber == 1)
    {
        // if we are player modeling, we should have only generated the first move
        if (m_params.playerModel(playerToMove) != PlayerModels::None)
	    {
            // so return false
		    return false;
	    }

	    // if there is a transposition table entry for this state
	    if (TTval.found())
	    {
		    // if there was a valid move found with higher depth, just do that one
		    const AlphaBetaMove & abMove = getAlphaBetaMove(TTval, playerToMove);
		    if ((TTval.entry()->getDepth() >= depth) && abMove.isValid())
		    {
                // so return false
			    return false;
		    }
	    }
    }

	const Array<std::vector<Action>, Constants::Max_Ordered_Moves> & orderedMoves(m_orderedMoves[depth]);
    moveVec.clear();
   
	// if this move should be from the ordered list, return it from the list
	if (moveNumber < orderedMoves.size())
	{
        moveVec.assign(orderedMoves[moveNumber].begin(), orderedMoves[moveNumber].end());
        return true;
	}
	// otherwise return the next move vector starting from the beginning
	else
	{
        if (moves.hasMoreMoves())
        {
            moves.getNextMoveVec(moveVec);
            return true;
        }
        else
        {
            return false;
        }
	}
}

const size_t AlphaBetaSearch::getPlayerToMove(GameState & state, const size_t depth, const size_t lastPlayerToMove, const bool isFirstSimMove) const
{
	const size_t whoCanMove(state.whoCanMove());

	// if both players can move
	if (whoCanMove == Players::Player_Both)
	{
		// no matter what happens, the 2nd player to move is always the enemy of the first
		if (!isFirstSimMove)
		{
			return getEnemy(lastPlayerToMove);
		}

		// pick the first move based on our policy
		const size_t policy(m_params.playerToMoveMethod());
		const size_t maxPlayer(m_params.maxPlayer());

		if (policy == SparCraft::PlayerToMove::Alternate)
		{
			return isRoot(depth) ? maxPlayer : getEnemy(lastPlayerToMove);
		}
		else if (policy == SparCraft::PlayerToMove::Not_Alternate)
		{
			return isRoot(depth) ? maxPlayer : lastPlayerToMove;
		}
		else if (policy == SparCraft::PlayerToMove::Random)
		{
			// srand(state.calculateHash(0));
			return isRoot(depth) ? maxPlayer : rand() % 2;
		}

		// we should never get to this state
		System::FatalError("AlphaBeta Error: Nobody can move for some reason");
		return Players::Player_None;
	}
	else
	{
		return whoCanMove;
	}
}

const bool AlphaBetaSearch::isTranspositionLookupState(GameState & state, const std::vector<Action> * firstSimMove) const
{
	return !state.bothCanMove() || (state.bothCanMove() && !firstSimMove);
}

AlphaBetaValue AlphaBetaSearch::alphaBeta(GameState & state, size_t depth, const size_t lastPlayerToMove, std::vector<Action> * prevSimMove, StateEvalScore alpha, StateEvalScore beta)
{
	// update statistics
	m_results.nodesExpanded++;

	if (searchTimeOut())
	{
		throw 1;
	}
    
	if (terminalState(state, depth))
	{
		// return the value, but the move will not be valid since none was performed
        StateEvalScore evalScore = state.eval(m_params.maxPlayer(), m_params.evalMethod(), m_params.simScript(Players::Player_One), m_params.simScript(Players::Player_Two));
		
		return AlphaBetaValue(StateEvalScore(evalScore.val(), state.getNumMovements(m_params.maxPlayer()) + evalScore.numMoves() ), AlphaBetaMove());
	}

	// figure out which player is to move
	const size_t playerToMove(getPlayerToMove(state, depth, lastPlayerToMove, !prevSimMove));

	// is the player to move the max player?
	bool maxPlayer = (playerToMove == m_params.maxPlayer());

	// Transposition Table Logic
	TTLookupValue TTval;
	if (isTranspositionLookupState(state, prevSimMove))
	{
		TTval = TTlookup(state, alpha, beta, depth);

		// if this is a TT cut, return the proper value
		if (TTval.cut())
		{
			return AlphaBetaValue(TTval.entry()->getScore(), getAlphaBetaMove(TTval, playerToMove));
		}
	}

	bool bestMoveSet(false);

	// move generation
	MoveArray & moves = m_allMoves[depth];
	state.generateMoves(moves, playerToMove);
    moves.shuffleMoveActions();
	generateOrderedMoves(state, moves, TTval, playerToMove, depth);

	// while we have more simultaneous moves
	AlphaBetaMove bestMove, bestSimResponse;
	    
    size_t moveNumber(0);
    std::vector<Action> moveVec;

    // for each child
    while (getNextMoveVec(playerToMove, moves, moveNumber, TTval, depth, moveVec))
	{
        // the value of the recursive AB we will call
		AlphaBetaValue val;
		
		// generate the child state
		GameState child(state);

		bool firstMove = true;

		// if this is the first player in a simultaneous move state
		if (state.bothCanMove() && !prevSimMove && (depth != 1))
		{
			firstMove = true;
			// don't generate a child yet, just pass on the move we are investigating
			val = alphaBeta(state, depth-1, playerToMove, &moveVec, alpha, beta);
		}
		else
		{
			firstMove = false;

			// if this is the 2nd move of a simultaneous move state
				if (prevSimMove)
				{
					// do the previous move selected by the first player to move during this state
	                child.makeMoves(*prevSimMove);
				}

				// do the moves of the current player
	            child.makeMoves(moveVec, prevSimMove != NULL);
				child.finishedMoving();

			// get the alpha beta value
			val = alphaBeta(child, depth-1, playerToMove, NULL, alpha, beta);
		}

		// set alpha or beta based on maxplayer
		if (maxPlayer && (val.score() > alpha)) 
		{
			alpha = val.score();
			bestMove = AlphaBetaMove(moveVec, true);
			bestMoveSet = true;

			if (state.bothCanMove() && !prevSimMove)
			{
				bestSimResponse = val.abMove();
			}

			// if this is depth 1 of the first try at depth 1, store the best in results
		}
		else if (!maxPlayer && (val.score() < beta))
		{
			beta = val.score();
			bestMove = AlphaBetaMove(moveVec, true);
			bestMoveSet = true;

			if (state.bothCanMove() && prevSimMove)
			{
				bestSimResponse = val.abMove();
			}
		}

		if (alpha.val() == -10000000 && beta.val() == 10000000)
		{
			fprintf(stderr, "\n\nALPHA BETA ERROR, NO VALUE SET\n\n");
		}

		// alpha-beta cut
		if (alpha >= beta) 
		{ 
			break; 
		}

        moveNumber++;
	}
	
	if (isTranspositionLookupState(state, prevSimMove))
	{
		TTsave(state, maxPlayer ? alpha : beta, alpha, beta, depth, playerToMove, bestMove, bestSimResponse);
	}

	return maxPlayer ? AlphaBetaValue(alpha, bestMove) : AlphaBetaValue(beta, bestMove);
}


AlphaBetaSearchResults & AlphaBetaSearch::getResults()
{
	return m_results;
}

const size_t AlphaBetaSearch::getEnemy(const size_t player) const
{
	return (player + 1) % 2;
}

const bool AlphaBetaSearch::isRoot(const size_t depth) const
{
	return depth == m_currentRootDepth;
}


