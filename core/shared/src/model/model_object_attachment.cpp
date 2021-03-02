/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/model.h"

const std::vector<ObjectAttachment> &Model::GetObjectAttachments() const {return const_cast<Model*>(this)->GetObjectAttachments();}
std::vector<ObjectAttachment> &Model::GetObjectAttachments() {return m_objectAttachments;}
uint32_t Model::AddObjectAttachment(ObjectAttachment::Type type,const std::string &name,const std::string &attachment,const std::unordered_map<std::string,std::string> &keyValues)
{
	m_objectAttachments.push_back({});
	auto &objAtt = m_objectAttachments.back();
	objAtt.type = type;
	objAtt.name = name;
	objAtt.attachment = attachment;
	objAtt.keyValues = keyValues;
	ustring::to_lower(objAtt.name);
	return m_objectAttachments.size() -1u;
}
uint32_t Model::GetObjectAttachmentCount() const {return m_objectAttachments.size();}
ObjectAttachment *Model::GetObjectAttachment(uint32_t idx)
{
	if(idx >= m_objectAttachments.size())
		return nullptr;
	return &m_objectAttachments.at(idx);
}
bool Model::LookupObjectAttachment(const std::string &name,uint32_t &attId) const
{
	auto lname = name;
	ustring::to_lower(lname);
	auto it = std::find_if(m_objectAttachments.begin(),m_objectAttachments.end(),[&lname](const ObjectAttachment &att) {
		return att.name == lname;
	});
	if(it == m_objectAttachments.end())
		return false;
	return it -m_objectAttachments.begin();
}
bool Model::RemoveObjectAttachment(const std::string &name)
{
	auto attIdx = 0u;
	if(LookupObjectAttachment(name,attIdx) == false)
		return false;
	return RemoveObjectAttachment(attIdx);
}
bool Model::RemoveObjectAttachment(uint32_t idx)
{
	if(idx >= m_objectAttachments.size())
		return false;
	m_objectAttachments.erase(m_objectAttachments.begin() +idx);
	return true;
}
