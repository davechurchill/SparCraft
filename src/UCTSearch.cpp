#include "UCTSearch.h"

using namespace SparCraft;

UCTSearch::UCTSearch(const UCTSearchParameters & params) 
	: m_params(params)
    , m_memoryPool(NULL)
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

void UCTSearch::setMemoryPool(UCTMemoryPool * pool)
{
    m_memoryPool = pool;
}

void UCTSearch::doSearch(GameState & initialState, std::vector<Action> & move)
{
    Timer t;
    t.start();

    m_rootNode = UCTNode(NULL, Players::Player_None, SearchNodeType::RootNode, m_actionVec, m_params.maxChildren(), m_memoryPool ? m_memoryPool->alloc() : NULL);

    // do the required number of traversals
    for (size_t traversals(0); traversals < m_params.maxTraversals(); ++traversals)
    {
        GameState state(initialState);
        traverse(m_rootNode, state);

        if (traversals && (traversals % 5 == 0))
        {
            if (m_params.timeLimit() && (t.elapsedMS() >= m_params.timeLimit()))
            {
                break;
            }
        }

        m_results.traversals++;

        //printSubTree(m_rootNode, initialState, "__uct.txt");
        //system("\"C:\\Program Files (x86)\\Graphviz2.30\\bin\\dot.exe\" < __uct.txt -Tpng > uct.png");
    }

    // choose the move to return
    if (m_params.rootMoveSelectionMethod() == UCTMoveSelect::HighestValue)
    {
        move = m_rootNode.bestUCTValueChild(true, m_params).getMove();
    }
    else if (m_params.rootMoveSelectionMethod() == UCTMoveSelect::MostVisited)
    {
        move = m_rootNode.mostVisitedChild().getMove();
    }

    if (m_params.graphVizFilename().length() > 0)
    {
        //printSubTree(m_rootNode, initialState, m_params.graphVizFilename());
        //system("\"C:\\Program Files (x86)\\Graphviz2.30\\bin\\dot.exe\" < __uct.txt -Tpng > uct.png");
    }

    double ms = t.elapsedMS();
    m_results.timeElapsed = ms;
    //printf("Search took %lf ms\n", ms);
    //printf("Hello\n");
}

const bool UCTSearch::searchTimeOut()
{
	return (m_params.timeLimit() && (m_searchTimer.elapsedMS() >= m_params.timeLimit()));
}

const bool UCTSearch::terminalState(GameState & state, const size_t depth) const
{
	return (depth <= 0 || state.isTerminal());
}

void UCTSearch::generateOrderedMoves(GameState & state, MoveArray & moves, const size_t playerToMove)
{
	m_orderedMoves.clear();

	// if we are using opponent modeling, get the move and then return, we don't want to put any more moves in
    if (m_params.playerModel(playerToMove) != PlayerModels::None)
	{
        // put the vector into the ordered moves array
        m_orderedMoves.add(std::vector<Action>());

        // generate the moves into that vector
		m_playerModels[playerToMove]->getMoves(state, moves, m_orderedMoves[0]);
		
		return;
	}

	// if we are using script move ordering, insert the script moves we want
    if (m_params.moveOrderingMethod() == MoveOrderMethod::ScriptFirst)
    {
        for (size_t s(0); s<m_params.getOrderedMoveScripts().size(); s++)
	    {
            std::vector<Action> moveVec;
		    m_allScripts[playerToMove][s]->getMoves(state, moves, moveVec);
		    m_orderedMoves.add(moveVec);
	    }
    }
	
}
const size_t UCTSearch::getChildNodeType(UCTNode & parent, const GameState & prevState) const
{
    if (!prevState.bothCanMove())
    {
        return SearchNodeType::SoloNode;
    }
    else
    {
        if (parent.getNodeType() == SearchNodeType::RootNode)
        {
            return SearchNodeType::FirstSimNode;
        }
        else if (parent.getNodeType() == SearchNodeType::SoloNode)
        {
            return SearchNodeType::FirstSimNode;
        }
        else if (parent.getNodeType() == SearchNodeType::SecondSimNode)
        {
            return SearchNodeType::FirstSimNode;
        }
        else if (parent.getNodeType() == SearchNodeType::FirstSimNode)
        {
            return SearchNodeType::SecondSimNode;
        }
    }

    return SearchNodeType::Default;
}

const bool UCTSearch::getNextMove(size_t playerToMove, MoveArray & moves, const size_t moveNumber, std::vector<Action> & actionVec)
{
    if (moveNumber > m_params.maxChildren())
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
    }

    actionVec.clear();

	// if this move should be from the ordered list, return it from the list
	if (moveNumber < m_orderedMoves.size())
	{
        actionVec.assign(m_orderedMoves[moveNumber].begin(), m_orderedMoves[moveNumber].end());
        return true;
	}
	// otherwise return the next move vector starting from the beginning
	else
	{
        if (moves.hasMoreMoves())
        {
            moves.getNextMoveVec(actionVec);
            return true;
        }
        else
        {
            return false;
        }
	}
}

const size_t UCTSearch::getPlayerToMove(UCTNode & node, const GameState & state) const
{
	const size_t whoCanMove(state.whoCanMove());

	// if both players can move
	if (whoCanMove == Players::Player_Both)
	{
        // pick the first move based on our policy
		const size_t policy(m_params.playerToMoveMethod());
		const size_t maxPlayer(m_params.maxPlayer());

        // the max player always chooses at the root
        if (isRoot(node))
        {
            return maxPlayer;
        }

        // the type of node this is
        const size_t nodeType = node.getNodeType();

        // the 2nd player in a sim move is always the enemy of the first
        if (nodeType == SearchNodeType::FirstSimNode)
        {
            return state.getEnemy(node.getPlayer());
        }
        // otherwise use our policy to see who goes first in a sim move state
        else
        {
		    if (policy == SparCraft::PlayerToMove::Alternate)
		    {
			    return state.getEnemy(node.getPlayer());
		    }
		    else if (policy == SparCraft::PlayerToMove::Not_Alternate)
		    {
			    return node.getPlayer();
		    }
		    else if (policy == SparCraft::PlayerToMove::Random)
		    {
			    return rand() % 2;
		    }

            // we should never get to this state
		    System::FatalError("UCT Error: Nobody can move for some reason");
		    return Players::Player_None;
        }
	}
	else
	{
		return whoCanMove;
	}
}

UCTNode & UCTSearch::UCTNodeSelect(UCTNode & parent)
{
    UCTNode *   bestNode    = NULL;
    bool        maxPlayer   = isRoot(parent) || (parent.getChild(0).getPlayer() == m_params.maxPlayer());
    double      bestVal     = maxPlayer ? std::numeric_limits<double>::min() : std::numeric_limits<double>::max();
         
    // loop through each child to find the best node
    for (size_t c(0); c < parent.numChildren(); ++c)
    {
        UCTNode & child = parent.getChild(c);

		double currentVal(0);
	
        // if we have visited this node already, get its UCT value
		if (child.numVisits() > 0)
		{
			double winRate    = (double)child.numWins() / (double)child.numVisits();
            double uctVal     = m_params.cValue() * sqrt( log( (double)parent.numVisits() ) / ( child.numVisits() ) );
			currentVal        = maxPlayer ? (winRate + uctVal) : (winRate - uctVal);
            
            child.setUCTVal(currentVal);
		}
		else
		{
            // if we haven't visited it yet, return it and visit immediately
			return child;
		}

        // choose the best node depending on max or min player
        if (maxPlayer)
        {
            if (currentVal > bestVal)
            {
                bestVal             = currentVal;
			    bestNode            = &child;
            }
        }
        else if (currentVal < bestVal)
        {
            bestVal             = currentVal;
			bestNode            = &child;
        }
	}

    return *bestNode;
}

void UCTSearch::updateState(UCTNode & node, GameState & state, bool isLeaf)
{
    const bool secondSimNode = (node.getNodeType() == SearchNodeType::SecondSimNode);

    // if it's the first sim move with children, or the root node
    if ((node.getNodeType() != SearchNodeType::FirstSimNode) || isLeaf)
    {
        // if this is a second sim node
        if (secondSimNode)
        {
            // make the parent's moves on the state because they haven't been done yet
            state.makeMoves(node.getParent()->getMove());
        }

        // do the current node moves and call finished moving
        state.makeMoves(node.getMove(), secondSimNode);
        state.finishedMoving();
    }
}

StateEvalScore UCTSearch::traverse(UCTNode & node, GameState & currentState)
{
    StateEvalScore playoutVal;

    m_results.totalVisits++;

    // if we haven't visited this node yet, do a playout
    if (node.numVisits() == 0)
    {
        // update the status of the current state with this node's moves
        //updateState(node, currentState, !node.hasChildren());
        updateState(node, currentState, true);

        // do the playout
        playoutVal = currentState.eval(m_params.maxPlayer(), m_params.evalMethod(), m_params.simScript(Players::Player_One), m_params.simScript(Players::Player_Two));

        m_results.nodesVisited++;
    }
    // otherwise we have seen this node before
    else
    {
        // update the state for a non-leaf node
        updateState(node, currentState, false);

        if (currentState.isTerminal())
        {
            playoutVal = currentState.eval(m_params.maxPlayer(), EvaluationMethods::LTD2);
        }
        else
        {
            // if the children haven't been generated yet
            if (!node.hasChildren())
            {
                generateChildren(node, currentState);
            }

            UCTNode & next = UCTNodeSelect(node);
            playoutVal = traverse(next, currentState);
        }
    }

    node.incVisits();
    
    if (playoutVal.val() > 0)
    {
        node.addWins(1);
    }
    else if (playoutVal.val() == 0)
    {
        node.addWins(0.5);
    }

    return playoutVal;
}

// generate the children of state 'node'
// state is the GameState after node's moves have been performed
void UCTSearch::generateChildren(UCTNode & node, GameState & state)
{
    // figure out who is next to move in the game
    const size_t playerToMove(getPlayerToMove(node, state));

    // generate all the moves possible from this state
	state.generateMoves(m_moveArray, playerToMove);
    m_moveArray.shuffleMoveActions();

    // generate the 'ordered moves' for move ordering
    generateOrderedMoves(state, m_moveArray, playerToMove);

    // for each child of this state, add a child to the current node
    for (size_t child(0); (child < m_params.maxChildren()) && getNextMove(playerToMove, m_moveArray, child, m_actionVec); ++child)
    {
        // add the child to the tree
        node.addChild(&node, playerToMove, getChildNodeType(node, state), m_actionVec, m_params.maxChildren(), m_memoryPool ? m_memoryPool->alloc() : NULL);
        m_results.nodesCreated++;
    }
}

StateEvalScore UCTSearch::performPlayout(GameState & state)
{
    GameState copy(state);
    copy.finishedMoving();

    return copy.eval(m_params.maxPlayer(), m_params.evalMethod(), m_params.simScript(Players::Player_One), m_params.simScript(Players::Player_Two));
}

const bool UCTSearch::isRoot(UCTNode & node) const
{
    return &node == &m_rootNode;
}

void UCTSearch::printSubTree(UCTNode & node, GameState s, std::string filename)
{
    std::ofstream out(filename.c_str());

    GraphViz::Graph G("g");
    G.set("bgcolor", "#ffffff");

    printSubTreeGraphViz(node, G, s);

    G.print(out);
}

void UCTSearch::printSubTreeGraphViz(UCTNode & node, GraphViz::Graph & g, GameState state)
{
    const bool secondSimNode = (node.getNodeType() == SearchNodeType::SecondSimNode);

    if (node.getNodeType() == SearchNodeType::FirstSimNode && node.hasChildren())
    {
        // don't make any moves if it is a first simnode
    }
    else
    {
        if (secondSimNode)
        {
            state.makeMoves(node.getParent()->getMove());
        }

        state.makeMoves(node.getMove(), secondSimNode);
        state.finishedMoving();
    }

    std::stringstream label;
    std::stringstream move;

    for (size_t a(0); a<node.getMove().size(); ++a)
    {
        move << node.getMove()[a].moveString() << "\\n";
    }

    if (node.getMove().size() == 0)
    {
        move << "root";
    }

    std::string firstSim = SearchNodeType::getName(node.getNodeType());

    Unit p1 = state.getUnit(0,0);
    Unit p2 = state.getUnit(1,0);

    label   << move.str() 
            << "\\nVal: "       << node.getUCTVal() 
            << "\\nWins: "      << node.numWins() 
            << "\\nVisits: "    << node.numVisits() 
            << "\\nChildren: "  << node.numChildren() 
            << "\\n"            << firstSim
            << "\\nPtr: "       << &node
            << "\\n---------------"
            << "\\nFrame: " << state.getTime()
            << "\\nHP: " << p1.currentHP() << "  " << p2.currentHP()
            << "\\nAtk: " << p1.nextAttackActionTime() << "  " << p2.nextAttackActionTime()
            << "\\nMove: " << p1.nextMoveActionTime() << "  " << p2.nextMoveActionTime()
            << "\\nPrev: " << p1.previousActionTime() << "  " << p2.previousActionTime();
    
    std::string fillcolor       ("#aaaaaa");

    if (node.getPlayer() == Players::Player_One)
    {
        fillcolor = "#ff0000";
    }
    else if (node.getPlayer() == Players::Player_Two)
    {
        fillcolor = "#00ff00";
    }
    
    GraphViz::Node n(getNodeIDString(node));
    n.set("label",      label.str());
    n.set("fillcolor",  fillcolor);
    n.set("color",      "#000000");
    n.set("fontcolor",  "#000000");
    n.set("style",      "filled,bold");
    n.set("shape",      "box");
    g.addNode(n);

    // recurse for each child
    for (size_t c(0); c<node.numChildren(); ++c)
    {
        UCTNode & child = node.getChild(c);
        if (child.numVisits() > 0)
        {
            GraphViz::Edge edge(getNodeIDString(node), getNodeIDString(child));
            g.addEdge(edge);
            printSubTreeGraphViz(child, g, state);
        }
    }
}
 
std::string UCTSearch::getNodeIDString(UCTNode & node)
{
    std::stringstream ss;
    ss << (unsigned long long)&node;
    return ss.str();
}

UCTSearchResults & UCTSearch::getResults()
{
    return m_results;
}

