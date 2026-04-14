#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>

namespace SparCraft
{
template <class T, size_t elem>
class Array
{
	size_t	m_size;
	size_t  m_capacity;
	T		m_arr[elem];

public:

	Array()
		: m_size(0)
		, m_capacity(elem)
	{
	}

	Array(const T & fill)
		: m_size(0)
		, m_capacity(elem)
	{
		std::fill(m_arr, m_arr + elem, fill);
	}

	T & get(const size_t index)
	{
		return m_arr[index];
	}

	const T & get(const size_t index) const
	{
		return m_arr[index];
	}

	T & operator [] (const size_t index)
	{
		return get(index);
	}
	
	const T & operator [] (const size_t index) const
	{
		return get(index);
	}

	const bool contains(const T & e) const
	{
		for (size_t i(0); i<m_size; ++i)
		{
			if (get(i) == e)
			{
				return true;
			}
		}

		return false;
	}

	const size_t capacity() const
	{
		return m_capacity;
	}

	const bool containsSize(const T & e) const
	{
		for (size_t i(0); i<m_size; ++i)
		{
			if (get(i) == e)
			{
				return true;
			}
		}

		return false;
	}

	void inc()
	{
		m_size++;
	}		

	void add(const T & e)
	{
		assert(m_size < capacity());
		get(m_size) = e;
		m_size++;
	}

	void addUnique(const T & e)
	{
		if (!contains(e))
		{
			add(e);
		}
	}

	const T & back() const
	{
		assert(m_size > 0);
		return get(m_size - 1);
	}

	void clear()
	{
		m_size = 0;
	}

	const size_t size() const
	{
		return m_size;
	}

	void fill(const T & e)
	{
		std::fill(m_arr, m_arr + elem, e);
	}
};

template <class T, size_t rows, size_t cols>
class Array2D
{
	size_t					m_rows;
	size_t					m_cols;

	Array< Array<T, cols>, rows>	m_arr;

public:

	Array2D()
		: m_rows(rows)
		, m_cols(cols)
	{
	}

	Array2D & operator = (const Array2D<T, rows, cols> & rhs)
	{
		if (this == &rhs)
		{
			return *this;
		}

		for (size_t r(0); r<m_rows; ++r)
		{
			m_arr[r] = rhs.m_arr[r];
		}

		return *this;
	}

	Array2D(const Array2D<T, rows, cols> & rhs)
		: m_rows(rhs.m_rows)
		, m_cols(rhs.m_cols)
		, m_arr(rhs.m_arr)
	{
		
	}

	const size_t getRows() const
	{
		return m_arr.capacity();
	}
	
	Array<T, cols> & operator [] (const size_t index)
	{
		assert(index < m_rows);
		return m_arr[index];
	}
	
	const Array<T, cols> & operator [] (const size_t index) const
	{
		assert(index < m_rows);
		return m_arr[index];
	}

	void fill(const T & e)
	{
		for (size_t i(0); i<m_rows; ++i)
		{
			m_arr[i].fill(e);
		}
	}

};
}
