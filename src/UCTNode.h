#pragma once

#include "Common.h"
#include "Action.h"

namespace SparCraft
{


class UCTNode
{
    // uct stat counting variables
    size_t                      m_numVisits;         // total visits to this node
    double                      m_numWins;           // wins from this node
    double                      m_uctVal;            // previous computed UCT value
            
    // game specific variables
    size_t                      m_player;            // the player who made a move to generate this node
    size_t                      m_nodeType;
    std::vector<Action>     m_move;              // the ove that generated this node

    // holds children
    std::vector<UCTNode>        m_children;

    // nodes for traversing the tree
    UCTNode *                   m_parent;
    
public:

    UCTNode ()
        : m_numVisits            (0)
        , m_numWins              (0)
        , m_uctVal               (0)
        , m_player               (Players::Player_None)
        , m_nodeType             (SearchNodeType::Default)
        , m_parent               (NULL)
    {

    }

    UCTNode (UCTNode * parent, const size_t player, const size_t nodeType, const std::vector<Action> & move, const size_t maxChildren, std::vector<UCTNode> * fromPool = NULL)
        : m_numVisits            (0)
        , m_numWins              (0)
        , m_uctVal               (0)
        , m_player               (player)
        , m_nodeType             (nodeType)
        , m_move                 (move)
        , m_parent               (parent)
    {
        m_children.reserve(maxChildren);
    }

    const size_t    numVisits()                 const           { return m_numVisits; }
    const double    numWins()                   const           { return m_numWins; }
    const size_t    numChildren()               const           { return m_children.size(); }
    const double    getUCTVal()                 const           { return m_uctVal; }
    const bool      hasChildren()               const           { return numChildren() > 0; }
    const size_t    getNodeType()               const           { return m_nodeType; }
    const size_t    getPlayer()                 const           { return m_player; }

    UCTNode *       getParent()                 const           { return m_parent; }
    UCTNode &       getChild(const size_t c)                  { return m_children[c]; }

    void            setUCTVal(double val)                       { m_uctVal = val; }
    void            incVisits()                                 { m_numVisits++; }
    void            addWins(double val)                         { m_numWins += val; }

    std::vector<UCTNode> & getChildren()                        { return m_children; }

    const std::vector<Action> & getMove() const
    {
        return m_move;
    }

    void setMove(const std::vector<Action> & move)
    {
        m_move = move;
    }

    void addChild(UCTNode * parent, const size_t player, const size_t nodeType, const std::vector<Action> & move, const size_t maxChildren, std::vector<UCTNode> * fromPool = NULL)
    {
        m_children.push_back(UCTNode(parent, player, nodeType, move, maxChildren));
    }

    UCTNode & mostVisitedChild() 
    {
        UCTNode * mostVisitedChild = NULL;
        size_t mostVisits = 0;

       for (size_t c(0); c < numChildren(); ++c)
       {
           UCTNode & child = getChild(c);

           if (!mostVisitedChild || (child.numVisits() > mostVisits))
           {
               mostVisitedChild = &child;
               mostVisits = child.numVisits();
           }
       }

       return *mostVisitedChild;
    }

    UCTNode & bestUCTValueChild(const bool maxPlayer, const UCTSearchParameters & params) 
    {
        UCTNode * bestChild = NULL;
        double bestVal = maxPlayer ? std::numeric_limits<double>::min() : std::numeric_limits<double>::max();

        for (size_t c(0); c < numChildren(); ++c)
        {
            UCTNode & child = getChild(c);
       
            double winRate      = (double)child.numWins() / (double)child.numVisits();
            double uctVal       = params.cValue() * sqrt( log( (double)numVisits() ) / ( child.numVisits() ) );
			double currentVal   = maxPlayer ? (winRate + uctVal) : (winRate - uctVal);

            if (maxPlayer)
            {
                if (currentVal > bestVal)
                {
                    bestVal             = currentVal;
			        bestChild           = &child;
                }
            }
            else if (currentVal < bestVal)
            {
                bestVal             = currentVal;
		        bestChild           = &child;
            }
        }

        return *bestChild;
    }
};
}

