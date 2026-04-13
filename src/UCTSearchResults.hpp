#pragma once

#include <vector>
#include "Action.h"

namespace SparCraft
{
class UCTSearchResults
{

public:

	unsigned long long          nodesExpanded;	// number of nodes expanded in the search
	double                      timeElapsed;	// time elapsed in milliseconds

    int                         traversals;
    int                         traverseCalls;
    int                         nodesVisited;
    int                         totalVisits;
    int                         nodesCreated;

    std::vector<Action>     bestMoves;
	ScoreType                   abValue;
	
    std::vector<std::vector<std::string> > m_desc;    // 2-column description vector

	UCTSearchResults() 
		: nodesExpanded         (0)
		, timeElapsed           (0)
        , traversals            (0)
        , traverseCalls         (0)
        , nodesVisited          (0)
        , totalVisits           (0)
        , nodesCreated          (0)
		, abValue               (0)
	{
	}

    std::vector<std::vector<std::string> > & getDescription()
    {
        m_desc.clear();
        m_desc.push_back(std::vector<std::string>());
        m_desc.push_back(std::vector<std::string>());

        std::stringstream ss;

        m_desc[0].push_back("Traversals: ");
        m_desc[0].push_back("Nodes Visited: ");
        m_desc[0].push_back("Total Visits: ");
        m_desc[0].push_back("Nodes Created: ");

        ss << traversals;       m_desc[1].push_back(ss.str()); ss.str(std::string());
        ss << nodesVisited;     m_desc[1].push_back(ss.str()); ss.str(std::string());
        ss << totalVisits;      m_desc[1].push_back(ss.str()); ss.str(std::string());
        ss << nodesCreated;     m_desc[1].push_back(ss.str()); ss.str(std::string());
        
        return m_desc;
    }
};
}

