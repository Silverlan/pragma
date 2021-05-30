/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/model.h"
#include "pragma/model/animation/vertex_animation.hpp"
#include <sharedutils/util_string.h>

const std::vector<std::shared_ptr<VertexAnimation>> &Model::GetVertexAnimations() const {return const_cast<Model*>(this)->GetVertexAnimations();}
std::vector<std::shared_ptr<VertexAnimation>> &Model::GetVertexAnimations() {return m_vertexAnimations;}
std::shared_ptr<VertexAnimation> Model::CreateVertexAnimation(const std::string &name) const {return VertexAnimation::Create(name);}
std::vector<std::shared_ptr<VertexAnimation>>::iterator Model::FindVertexAnimation(const std::string &name)
{
	return std::find_if(m_vertexAnimations.begin(),m_vertexAnimations.end(),[&name](const std::shared_ptr<VertexAnimation> &anim) {
		return ustring::compare(name,anim->GetName(),false);
	});
}
std::vector<std::shared_ptr<VertexAnimation>>::const_iterator Model::FindVertexAnimation(const std::string &name) const {return const_cast<Model*>(this)->FindVertexAnimation(name);}
bool Model::GetVertexAnimationId(const std::string &name,uint32_t &id) const
{
	auto it = FindVertexAnimation(name);
	if(it == m_vertexAnimations.end())
		return false;
	id = it -m_vertexAnimations.begin();
	return true;
}
const std::shared_ptr<VertexAnimation> *Model::GetVertexAnimation(uint32_t vaIdx) const {return const_cast<Model*>(this)->GetVertexAnimation(vaIdx);}
std::shared_ptr<VertexAnimation> *Model::GetVertexAnimation(uint32_t vaIdx) {return (vaIdx < m_vertexAnimations.size()) ? &m_vertexAnimations.at(vaIdx) : nullptr;}
const std::shared_ptr<VertexAnimation> *Model::GetVertexAnimation(const std::string &name) const {return const_cast<Model*>(this)->GetVertexAnimation(name);}
std::shared_ptr<VertexAnimation> *Model::GetVertexAnimation(const std::string &name)
{
	auto it = FindVertexAnimation(name);
	if(it != m_vertexAnimations.end())
		return &(*it);
	return nullptr;
}
std::shared_ptr<VertexAnimation> Model::AddVertexAnimation(const std::string &name)
{
	auto it = FindVertexAnimation(name);
	if(it != m_vertexAnimations.end())
		return *it;
	m_vertexAnimations.push_back(CreateVertexAnimation(name));
	return m_vertexAnimations.back();
}
void Model::RemoveVertexAnimation(const std::string &name)
{
	auto it = FindVertexAnimation(name);
	if(it == m_vertexAnimations.end())
		return;
	m_vertexAnimations.erase(it);
}

