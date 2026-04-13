#pragma once

#include <ctime>
#include <limits>

namespace SparCraft
{
	class RandomInt;
}
	
class SparCraft::RandomInt
{
	int m_seed;
    int m_min;
    int m_max;

public:

	RandomInt(int min, int max, int seed)
        : m_seed(seed)
        , m_min(min)
        , m_max(max)
	{
		srand(seed);
	}

	int nextInt()
	{
		return ( rand() % (m_max-m_min) ) + m_min;
	}
};

