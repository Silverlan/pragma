// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cinttypes>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <algorithm>
#include <string>

module pragma.shared;

import :model.model;

const std::vector<std::shared_ptr<VertexAnimation>> &pragma::Model::GetVertexAnimations() const { return const_cast<pragma::Model *>(this)->GetVertexAnimations(); }
std::vector<std::shared_ptr<VertexAnimation>> &pragma::Model::GetVertexAnimations() { return m_vertexAnimations; }
std::shared_ptr<VertexAnimation> pragma::Model::CreateVertexAnimation(const std::string &name) const { return VertexAnimation::Create(name); }
std::vector<std::shared_ptr<VertexAnimation>>::iterator pragma::Model::FindVertexAnimation(const std::string &name)
{
	return std::find_if(m_vertexAnimations.begin(), m_vertexAnimations.end(), [&name](const std::shared_ptr<VertexAnimation> &anim) { return ustring::compare(name, anim->GetName(), false); });
}
std::vector<std::shared_ptr<VertexAnimation>>::const_iterator pragma::Model::FindVertexAnimation(const std::string &name) const { return const_cast<pragma::Model *>(this)->FindVertexAnimation(name); }
bool pragma::Model::GetVertexAnimationId(const std::string &name, uint32_t &id) const
{
	auto it = FindVertexAnimation(name);
	if(it == m_vertexAnimations.end())
		return false;
	id = it - m_vertexAnimations.begin();
	return true;
}
const std::shared_ptr<VertexAnimation> *pragma::Model::GetVertexAnimation(uint32_t vaIdx) const { return const_cast<pragma::Model *>(this)->GetVertexAnimation(vaIdx); }
std::shared_ptr<VertexAnimation> *pragma::Model::GetVertexAnimation(uint32_t vaIdx) { return (vaIdx < m_vertexAnimations.size()) ? &m_vertexAnimations.at(vaIdx) : nullptr; }
const std::shared_ptr<VertexAnimation> *pragma::Model::GetVertexAnimation(const std::string &name) const { return const_cast<pragma::Model *>(this)->GetVertexAnimation(name); }
std::shared_ptr<VertexAnimation> *pragma::Model::GetVertexAnimation(const std::string &name)
{
	auto it = FindVertexAnimation(name);
	if(it != m_vertexAnimations.end())
		return &(*it);
	return nullptr;
}
std::shared_ptr<VertexAnimation> pragma::Model::AddVertexAnimation(const std::string &name)
{
	auto it = FindVertexAnimation(name);
	if(it != m_vertexAnimations.end())
		return *it;
	m_vertexAnimations.push_back(CreateVertexAnimation(name));
	return m_vertexAnimations.back();
}
void pragma::Model::RemoveVertexAnimation(const std::string &name)
{
	auto it = FindVertexAnimation(name);
	if(it == m_vertexAnimations.end())
		return;
	m_vertexAnimations.erase(it);
}
