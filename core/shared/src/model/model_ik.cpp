/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/model.h"

std::vector<std::shared_ptr<IKController>>::const_iterator Model::FindIKController(const std::string &name) const { return const_cast<Model *>(this)->FindIKController(name); }
std::vector<std::shared_ptr<IKController>>::iterator Model::FindIKController(const std::string &name)
{
	return std::find_if(m_ikControllers.begin(), m_ikControllers.end(), [&name](const std::shared_ptr<IKController> &ikController) { return ustring::compare(ikController->GetEffectorName(), name, false); });
}
const std::vector<std::shared_ptr<IKController>> &Model::GetIKControllers() const { return const_cast<Model *>(this)->GetIKControllers(); }
std::vector<std::shared_ptr<IKController>> &Model::GetIKControllers() { return m_ikControllers; }
const IKController *Model::GetIKController(uint32_t id) const { return const_cast<Model *>(this)->GetIKController(id); }
IKController *Model::GetIKController(uint32_t id) { return (id < m_ikControllers.size()) ? m_ikControllers.at(id).get() : nullptr; }
bool Model::LookupIKController(const std::string &name, uint32_t &id) const
{
	auto it = FindIKController(name);
	if(it == m_ikControllers.end())
		return false;
	id = it - m_ikControllers.begin();
	return true;
}
IKController *Model::AddIKController(const std::string &name, uint32_t chainLength, const std::string &type, util::ik::Method method)
{
	auto it = FindIKController(name);
	if(it != m_ikControllers.end())
		return it->get();
	m_ikControllers.push_back(std::make_shared<IKController>(name, chainLength, type, method));
	return m_ikControllers.back().get();
}
void Model::RemoveIKController(uint32_t id)
{
	if(id >= m_ikControllers.size())
		return;
	m_ikControllers.erase(m_ikControllers.begin() + id);
}
void Model::RemoveIKController(const std::string &name)
{
	auto it = FindIKController(name);
	if(it == m_ikControllers.end())
		return;
	m_ikControllers.erase(it);
}
