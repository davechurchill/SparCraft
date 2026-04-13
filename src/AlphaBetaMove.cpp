#include "AlphaBetaMove.h"

using namespace SparCraft;

AlphaBetaMove::AlphaBetaMove()
    : m_isValid(false)
{
}

AlphaBetaMove::AlphaBetaMove(const std::vector<Action> & move,const bool & isValid)
    : m_move(move)
    ,m_isValid(isValid)
{
}

const bool AlphaBetaMove::isValid() const 
{ 
    return m_isValid; 
}

const std::vector<Action> & AlphaBetaMove::moveVec() const 
{ 
    return m_move; 
}

TTBestMove::TTBestMove()
{
}

TTBestMove::TTBestMove(const AlphaBetaMove & first,const AlphaBetaMove & second)
    : m_firstMove(first)
    ,m_secondMove(second)
{
}

const AlphaBetaMove & TTBestMove::firstMove() const 
{ 
    return m_firstMove; 
}

const AlphaBetaMove & TTBestMove::secondMove() const 
{ 
    return m_secondMove; 
}

AlphaBetaValue::AlphaBetaValue()
{
}

AlphaBetaValue::AlphaBetaValue(const StateEvalScore & score,const AlphaBetaMove & abMove)
    : m_score(score)
    ,m_move(abMove)
{
}

const StateEvalScore & AlphaBetaValue::score() const 
{ 
    return m_score; 
}

const AlphaBetaMove & AlphaBetaValue::abMove() const 
{ 
    return m_move; 
}

