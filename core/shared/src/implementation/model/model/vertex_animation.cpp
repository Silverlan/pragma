// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.model;

const std::vector<std::shared_ptr<pragma::animation::VertexAnimation>> &pragma::asset::Model::GetVertexAnimations() const { return const_cast<Model *>(this)->GetVertexAnimations(); }
std::vector<std::shared_ptr<pragma::animation::VertexAnimation>> &pragma::asset::Model::GetVertexAnimations() { return m_vertexAnimations; }
std::shared_ptr<pragma::animation::VertexAnimation> pragma::asset::Model::CreateVertexAnimation(const std::string &name) const { return animation::VertexAnimation::Create(name); }
std::vector<std::shared_ptr<pragma::animation::VertexAnimation>>::iterator pragma::asset::Model::FindVertexAnimation(const std::string &name)
{
	return std::find_if(m_vertexAnimations.begin(), m_vertexAnimations.end(), [&name](const std::shared_ptr<animation::VertexAnimation> &anim) { return string::compare(name, anim->GetName(), false); });
}
std::vector<std::shared_ptr<pragma::animation::VertexAnimation>>::const_iterator pragma::asset::Model::FindVertexAnimation(const std::string &name) const { return const_cast<Model *>(this)->FindVertexAnimation(name); }
bool pragma::asset::Model::GetVertexAnimationId(const std::string &name, uint32_t &id) const
{
	auto it = FindVertexAnimation(name);
	if(it == m_vertexAnimations.end())
		return false;
	id = it - m_vertexAnimations.begin();
	return true;
}
const std::shared_ptr<pragma::animation::VertexAnimation> *pragma::asset::Model::GetVertexAnimation(uint32_t vaIdx) const { return const_cast<Model *>(this)->GetVertexAnimation(vaIdx); }
std::shared_ptr<pragma::animation::VertexAnimation> *pragma::asset::Model::GetVertexAnimation(uint32_t vaIdx) { return (vaIdx < m_vertexAnimations.size()) ? &m_vertexAnimations.at(vaIdx) : nullptr; }
const std::shared_ptr<pragma::animation::VertexAnimation> *pragma::asset::Model::GetVertexAnimation(const std::string &name) const { return const_cast<Model *>(this)->GetVertexAnimation(name); }
std::shared_ptr<pragma::animation::VertexAnimation> *pragma::asset::Model::GetVertexAnimation(const std::string &name)
{
	auto it = FindVertexAnimation(name);
	if(it != m_vertexAnimations.end())
		return &(*it);
	return nullptr;
}
std::shared_ptr<pragma::animation::VertexAnimation> pragma::asset::Model::AddVertexAnimation(const std::string &name)
{
	auto it = FindVertexAnimation(name);
	if(it != m_vertexAnimations.end())
		return *it;
	m_vertexAnimations.push_back(CreateVertexAnimation(name));
	return m_vertexAnimations.back();
}
void pragma::asset::Model::RemoveVertexAnimation(const std::string &name)
{
	auto it = FindVertexAnimation(name);
	if(it == m_vertexAnimations.end())
		return;
	m_vertexAnimations.erase(it);
}
