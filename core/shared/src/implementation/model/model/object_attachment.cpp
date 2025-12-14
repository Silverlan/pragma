// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.model;

const std::vector<pragma::asset::ObjectAttachment> &pragma::asset::Model::GetObjectAttachments() const { return const_cast<Model *>(this)->GetObjectAttachments(); }
std::vector<pragma::asset::ObjectAttachment> &pragma::asset::Model::GetObjectAttachments() { return m_objectAttachments; }
uint32_t pragma::asset::Model::AddObjectAttachment(ObjectAttachment::Type type, const std::string &name, const std::string &attachment, const std::unordered_map<std::string, std::string> &keyValues)
{
	m_objectAttachments.push_back({});
	auto &objAtt = m_objectAttachments.back();
	objAtt.type = type;
	objAtt.name = name;
	objAtt.attachment = attachment;
	objAtt.keyValues = keyValues;
	string::to_lower(objAtt.name);
	return m_objectAttachments.size() - 1u;
}
uint32_t pragma::asset::Model::GetObjectAttachmentCount() const { return m_objectAttachments.size(); }
pragma::asset::ObjectAttachment *pragma::asset::Model::GetObjectAttachment(uint32_t idx)
{
	if(idx >= m_objectAttachments.size())
		return nullptr;
	return &m_objectAttachments.at(idx);
}
bool pragma::asset::Model::LookupObjectAttachment(const std::string &name, uint32_t &attId) const
{
	auto lname = name;
	string::to_lower(lname);
	auto it = std::find_if(m_objectAttachments.begin(), m_objectAttachments.end(), [&lname](const ObjectAttachment &att) { return att.name == lname; });
	if(it == m_objectAttachments.end())
		return false;
	return it - m_objectAttachments.begin();
}
bool pragma::asset::Model::RemoveObjectAttachment(const std::string &name)
{
	auto attIdx = 0u;
	if(LookupObjectAttachment(name, attIdx) == false)
		return false;
	return RemoveObjectAttachment(attIdx);
}
bool pragma::asset::Model::RemoveObjectAttachment(uint32_t idx)
{
	if(idx >= m_objectAttachments.size())
		return false;
	m_objectAttachments.erase(m_objectAttachments.begin() + idx);
	return true;
}
