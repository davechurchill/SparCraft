#pragma once

#include "Common.h"


namespace SparCraft
{

    // type definitions for storing data
    typedef 	int					PositionType;
    typedef 	int					TimeType;
    typedef		short				HealthType;
    typedef		int					ScoreType;
    typedef     size_t              IDType;
    typedef		unsigned int		HashType;
    typedef     int                 UCTValue;

    class StateEvalScore
    {
        ScoreType	m_val;
        int			m_numMoves;

    public:

        StateEvalScore()
            : m_val(0)
            ,m_numMoves(0)
        {
        }

        StateEvalScore(const ScoreType val,const int numMoves)
            : m_val(val)
            ,m_numMoves(numMoves)
        {
        }

        const bool operator < (const StateEvalScore & rhs) const
        {
            if (m_val < rhs.m_val)
            {
                return true;
            }
            else if (m_val == rhs.m_val)
            {
                return m_numMoves > rhs.m_numMoves;
            }
            else
            {
                return false;
            }
        }

        const bool operator > (const StateEvalScore & rhs) const
        {
            if (m_val > rhs.m_val)
            {
                return true;
            }
            else if (m_val == rhs.m_val)
            {
                return m_numMoves < rhs.m_numMoves;
            }
            else
            {
                return false;
            }
        }

        const bool operator <= (const StateEvalScore & rhs) const
        {
            if (m_val < rhs.m_val)
            {
                return true;
            }
            else if (m_val == rhs.m_val)
            {
                return m_numMoves >= rhs.m_numMoves;
            }
            else
            {
                return false;
            }
        }

        const bool operator >= (const StateEvalScore & rhs) const
        {
            if (m_val > rhs.m_val)
            {
                return true;
            }
            else if (m_val == rhs.m_val)
            {
                return m_numMoves <= rhs.m_numMoves;
            }
            else
            {
                return false;
            }
        }

        const bool operator == (const StateEvalScore & rhs) const
        {
            return (m_val == rhs.m_val) && (m_numMoves == rhs.m_numMoves);
        }

        const ScoreType val() const { return m_val; }
        const TimeType numMoves() const { return m_numMoves; }
    };



}

