#pragma once

#include <vector>
#include "Action.h"

namespace SparCraft
{
class AlphaBetaSearchResults
{

public:

	bool 				solved,			// whether ot not a solution was found
						timedOut;		// did the search time-out?
	
	unsigned long long 	nodesExpanded;	// number of nodes expanded in the search
	
	double 				timeElapsed,	// time elapsed in milliseconds
						avgBranch;		// avg branching factor

    std::vector<Action>   bestMoves;
	ScoreType			abValue;
	unsigned long long  ttcuts;
	size_t				maxDepthReached;	

	size_t				ttMoveOrders;
	size_t				ttFoundButNoMove;
	size_t				ttFoundNoCut;
	size_t				ttFoundCheck;
	size_t				ttFoundLessDepth;
	size_t				ttSaveAttempts;

    std::vector<std::vector<std::string> > m_desc;    // 2-column description vector
	
	AlphaBetaSearchResults() 
		: solved(false)
		, timedOut(false)
		, nodesExpanded(0)
		, timeElapsed(0)
		, avgBranch(0)
		, abValue(0)
		, ttcuts(0)
		, maxDepthReached(0)
		, ttMoveOrders(0)
		, ttFoundButNoMove(0)
		, ttFoundNoCut(0)
		, ttFoundCheck(0)
		, ttFoundLessDepth(0)
		, ttSaveAttempts(0)
	{
	}

    std::vector<std::vector<std::string> > & getDescription()
    {
        m_desc.clear();
        m_desc.push_back(std::vector<std::string>());
        m_desc.push_back(std::vector<std::string>());

        std::stringstream ss;

        m_desc[0].push_back("Nodes Searched: ");
        m_desc[0].push_back("AB Value: ");
        m_desc[0].push_back("Max Depth: ");

        ss << nodesExpanded;       m_desc[1].push_back(ss.str()); ss.str(std::string());
        ss << abValue;              m_desc[1].push_back(ss.str()); ss.str(std::string());
        ss << maxDepthReached;     m_desc[1].push_back(ss.str()); ss.str(std::string());
        
        return m_desc;
    }
};
}

