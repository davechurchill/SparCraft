#pragma once

#include "Common.h"
#include "UCTNode.h"

namespace SparCraft
{
    class UCTMemoryPool;
}

class SparCraft::UCTMemoryPool
{	
    std::vector< std::vector<UCTNode> > m_pool;

    const size_t    m_poolSize;
    const size_t    m_maxChildren;
    
    size_t    m_currentIndex;

public:

	UCTMemoryPool(const size_t poolSize, const size_t maxChildren)
        : m_pool             (poolSize, std::vector<UCTNode>())
        , m_poolSize         (poolSize)
        , m_maxChildren      (maxChildren)
        , m_currentIndex     (0)
    {
        for (size_t s(0); s<poolSize; ++s)
        {
            m_pool[s].reserve(maxChildren);
        }
    }

    std::vector<UCTNode> * alloc()
    {
        std::vector<UCTNode> & ret(m_pool[m_currentIndex]);
        if (ret.size() > 0)
        {
            ret.clear();
        }

        m_currentIndex = (m_currentIndex + 1) % m_poolSize;
        return &ret;
    }

    void clearPool()
    {
        for (size_t i(0); i<m_poolSize; ++i)
        {
            m_pool[i].clear();
        }

        m_currentIndex = 0;
    }
};

