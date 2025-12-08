// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.model;

std::vector<std::shared_ptr<IKController>>::const_iterator pragma::asset::Model::FindIKController(const std::string &name) const { return const_cast<pragma::asset::Model *>(this)->FindIKController(name); }
std::vector<std::shared_ptr<IKController>>::iterator pragma::asset::Model::FindIKController(const std::string &name)
{
	return std::find_if(m_ikControllers.begin(), m_ikControllers.end(), [&name](const std::shared_ptr<IKController> &ikController) { return ustring::compare(ikController->GetEffectorName(), name, false); });
}
const std::vector<std::shared_ptr<IKController>> &pragma::asset::Model::GetIKControllers() const { return const_cast<pragma::asset::Model *>(this)->GetIKControllers(); }
std::vector<std::shared_ptr<IKController>> &pragma::asset::Model::GetIKControllers() { return m_ikControllers; }
const IKController *pragma::asset::Model::GetIKController(uint32_t id) const { return const_cast<pragma::asset::Model *>(this)->GetIKController(id); }
IKController *pragma::asset::Model::GetIKController(uint32_t id) { return (id < m_ikControllers.size()) ? m_ikControllers.at(id).get() : nullptr; }
bool pragma::asset::Model::LookupIKController(const std::string &name, uint32_t &id) const
{
	auto it = FindIKController(name);
	if(it == m_ikControllers.end())
		return false;
	id = it - m_ikControllers.begin();
	return true;
}
IKController *pragma::asset::Model::AddIKController(const std::string &name, uint32_t chainLength, const std::string &type, util::ik::Method method)
{
	auto it = FindIKController(name);
	if(it != m_ikControllers.end())
		return it->get();
	m_ikControllers.push_back(::util::make_shared<IKController>(name, chainLength, type, method));
	return m_ikControllers.back().get();
}
void pragma::asset::Model::RemoveIKController(uint32_t id)
{
	if(id >= m_ikControllers.size())
		return;
	m_ikControllers.erase(m_ikControllers.begin() + id);
}
void pragma::asset::Model::RemoveIKController(const std::string &name)
{
	auto it = FindIKController(name);
	if(it == m_ikControllers.end())
		return;
	m_ikControllers.erase(it);
}
