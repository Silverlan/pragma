// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __INDEXMAP_H__
#define __INDEXMAP_H__

#include <vector>
#include <queue>

template<class T>
class IndexMap {
  public:
	size_t Insert(const T &val);
	void Remove(size_t idx);
	size_t Size() const;
	T *operator[](const size_t idx);
  private:
	std::vector<T> m_values;
	std::queue<size_t> m_freeIndices;
};

template<class T>
size_t IndexMap<T>::Insert(const T &val)
{
	if(m_freeIndices.empty()) {
		m_values.push_back(val);
		return m_values.size() - 1;
	}
	auto idx = m_freeIndices.front();
	m_values.at(idx) = val;
	m_freeIndices.pop();
	return idx;
}

template<class T>
size_t IndexMap<T>::Size() const
{
	return m_values.size();
}

template<class T>
void IndexMap<T>::Remove(size_t idx)
{
	if(idx >= m_values.size() || idx < 0)
		return;
	if(idx == m_values.size() - 1) {
		m_values.pop_back();
		return;
	}
	m_values.at(idx) = {};
	m_freeIndices.push(idx);
}

template<class T>
T *IndexMap<T>::operator[](const size_t idx)
{
	if(idx >= m_values.size() || idx < 0)
		return nullptr;
	return &m_values.at(idx);
}

#endif
