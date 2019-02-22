#ifndef __INDEXMAP_H__
#define __INDEXMAP_H__

#include <vector>
#include <deque>
template<class T>
	class IndexMap
{
private:
	std::vector<T*> m_values;
	std::deque<unsigned int> m_indices;
public:
	unsigned int Insert(T* val);
	void Remove(unsigned int idx);
	unsigned int Size();
	T *operator[](const unsigned int idx);
};

template<class T>
	unsigned int IndexMap<T>::Insert(T* val)
{
	if(m_indices.empty())
	{
		m_values.push_back(val);
		return CUInt32(m_values.size()) -1;
	}
	unsigned int idx = m_indices[0];
	m_values[idx] = val;
	m_indices.pop_front();
	return idx;
}

template<class T>
	unsigned int IndexMap<T>::Size() {return CUInt32(m_values.size());}

template<class T>
	void IndexMap<T>::Remove(unsigned int idx)
{
	if(idx >= m_values.size() || idx < 0)
		return;
	if(idx == m_values.size() -1)
	{
		m_values.pop_back();
		return;
	}
	m_values[idx] = NULL;
	m_indices.push_back(idx);
}

template<class T>
	T *IndexMap<T>::operator[](const unsigned int idx)
{
	if(idx >= m_values.size() || idx < 0)
		return NULL;
	return m_values[idx];
}

#endif