#include "SparCraftException.h"

using namespace SparCraft;

SparCraftException::SparCraftException(std::string ss) 
    : m_s(ss) 
    , m_hasState(false)
{
}

SparCraftException::SparCraftException(std::string ss, const GameState * state) 
    : m_s(ss) 
    , m_hasState(false)
{
    if (state != nullptr)
    {
        m_state = *state;
        m_hasState = true;
    }
}

SparCraftException::~SparCraftException() throw () 
{
} 

const char* SparCraftException::what() const throw() 
{ 
    return m_s.c_str(); 
}

bool SparCraftException::hasState() const 
{ 
    return m_hasState; 
}

const GameState & SparCraftException::getState() const 
{ 
    return m_state; 
}

