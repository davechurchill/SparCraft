#pragma once

#include "Common.h"

namespace SparCraft
{
    class AlphaBetaSearchParameters;
}

class SparCraft::AlphaBetaSearchParameters
{											        // DEFAULT				DESCRIPTION
    size_t          m_searchMethod;                  // ID-AB                The Method to use for AB Search
    size_t		    m_maxPlayer;					    // Player_One			The player who will make maximizing moves
    size_t          m_maxDepth;                      // Max_Depth            Maximum depth of AB search to allow

	size_t		    m_timeLimit;					    // 0					Search time limit. 0 means no time limit
    size_t          m_maxChildren;                   // 10                   Max children at each node
    size_t          m_moveOrdering;                  // ScriptFirst          Move ordering method for child generation
	size_t		    m_evalMethod;				    // LTD				Evaluation function type
    size_t          m_simScripts[2];                 // NOKDPS               Policy to use for playouts
	size_t		    m_playerToMoveMethod;		    // Alternate			The player to move policy
	size_t		    m_playerModel[2];                // None                 Player model to use for each player

    std::string     m_graphVizFilename;              // ""                   File name to output graph viz file

    std::vector<size_t> m_orderedMoveScripts;

    std::vector<std::vector<std::string> > m_desc;    // 2-column description vector

public:

	// default constructor
	AlphaBetaSearchParameters() 
        : m_searchMethod         (SearchMethods::IDAlphaBeta)
        , m_maxPlayer            (Players::Player_One)
        , m_maxDepth             (Constants::Max_Search_Depth)
	    , m_timeLimit            (0)
        , m_maxChildren          (10)
        , m_moveOrdering         (MoveOrderMethod::ScriptFirst)
        , m_evalMethod           (SparCraft::EvaluationMethods::Playout)
	    , m_playerToMoveMethod   (SparCraft::PlayerToMove::Alternate)
    {
	    setPlayerModel(Players::Player_One, PlayerModels::None);
	    setPlayerModel(Players::Player_Two, PlayerModels::None);
        setSimScripts(PlayerModels::NOKDPS, PlayerModels::NOKDPS);
    }

    const size_t searchMethod()							    const   { return m_searchMethod; }
    const size_t maxPlayer()							        const   { return m_maxPlayer; }
    const size_t maxDepth()							        const   { return m_maxDepth; }
    const size_t timeLimit()							        const   { return m_timeLimit; }
    const size_t maxChildren()                                const   { return m_maxChildren; }
    const size_t moveOrderingMethod()                         const   { return m_moveOrdering; }
    const size_t evalMethod()						            const   { return m_evalMethod; }
    const size_t simScript(const size_t player)             const   { return m_simScripts[player]; }
    const size_t playerToMoveMethod()				            const   { return m_playerToMoveMethod; }
    const size_t playerModel(const size_t player)	        const   { return m_playerModel[player]; }
    const std::string & graphVizFilename()                      const   { return m_graphVizFilename; }
    const std::vector<size_t> & getOrderedMoveScripts()         const   { return m_orderedMoveScripts; }
	
    void setSearchMethod(const size_t method)                         { m_searchMethod = method; }
    void setMaxPlayer(const size_t player)					        { m_maxPlayer = player; }
    void setMaxDepth(const size_t depth)                              { m_maxDepth = depth; }
    
    void setTimeLimit(const size_t timeLimit)					        { m_timeLimit = timeLimit; }
    void setMaxChildren(const size_t children)                        { m_maxChildren = children; }
    void setMoveOrderingMethod(const size_t method)                   { m_moveOrdering = method; }
    void setEvalMethod(const size_t eval)						        { m_evalMethod = eval; }
    void setSimScripts(const size_t p1, const size_t p2)		    { m_simScripts[0] = p1; m_simScripts[1] = p2; }
    void setPlayerToMoveMethod(const size_t method)				    { m_playerToMoveMethod = method; }
    void setGraphVizFilename(const std::string & filename)              { m_graphVizFilename = filename; }
    void addOrderedMoveScript(const size_t script)                    { m_orderedMoveScripts.push_back(script); }
    void setPlayerModel(const size_t player, const size_t model)	{ m_playerModel[player] = model; }	

    std::vector<std::vector<std::string> > & getDescription()
    {
        if (m_desc.size() == 0)
        {
            m_desc.push_back(std::vector<std::string>());
            m_desc.push_back(std::vector<std::string>());

            std::stringstream ss;

            m_desc[0].push_back("Player Type:");
            m_desc[0].push_back("Time Limit:");
            m_desc[0].push_back("Max Children:");
            m_desc[0].push_back("Move Ordering:");
            m_desc[0].push_back("Player To Move:");
            m_desc[0].push_back("Opponent Model:");

            ss << "AlphaBeta";                                              m_desc[1].push_back(ss.str()); ss.str(std::string());
            ss << timeLimit() << "ms";                                      m_desc[1].push_back(ss.str()); ss.str(std::string());
            ss << maxChildren();                                            m_desc[1].push_back(ss.str()); ss.str(std::string());
            ss << MoveOrderMethod::getName(moveOrderingMethod());             m_desc[1].push_back(ss.str()); ss.str(std::string());
            ss << PlayerToMove::getName(playerToMoveMethod());                m_desc[1].push_back(ss.str()); ss.str(std::string());
            ss << PlayerModels::getName(playerModel((maxPlayer()+1)%2));   m_desc[1].push_back(ss.str()); ss.str(std::string());
        }
        
        return m_desc;
    }
};

