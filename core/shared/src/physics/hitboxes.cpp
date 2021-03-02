/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/model.h"

void Model::AddHitbox(uint32_t boneId,HitGroup group,const Vector3 &min,const Vector3 &max)
{
	AddHitbox(boneId,Hitbox(group,min,max));
}
void Model::AddHitbox(uint32_t boneId,const Hitbox &hitbox)
{
	auto it = m_hitboxes.find(boneId);
	if(it != m_hitboxes.end())
		it->second = hitbox;
	else
		m_hitboxes.insert(decltype(m_hitboxes)::value_type(boneId,hitbox));
}
uint32_t Model::GetHitboxCount() const {return static_cast<uint32_t>(m_hitboxes.size());}
const std::unordered_map<uint32_t,Hitbox> &Model::GetHitboxes() const {return const_cast<Model*>(this)->GetHitboxes();}
std::unordered_map<uint32_t,Hitbox> &Model::GetHitboxes() {return m_hitboxes;}
const Hitbox *Model::GetHitbox(uint32_t boneId) const
{
	auto it = m_hitboxes.find(boneId);
	if(it == m_hitboxes.end())
		return nullptr;
	return &it->second;
}
HitGroup Model::GetHitboxGroup(uint32_t boneId) const
{
	auto *hitbox = GetHitbox(boneId);
	if(hitbox == nullptr)
		return HitGroup::Invalid;
	return hitbox->group;
}
bool Model::GetHitboxBounds(uint32_t boneId,Vector3 &min,Vector3 &max) const
{
	auto *hitbox = GetHitbox(boneId);
	if(hitbox == nullptr)
	{
		min = Vector3{0.f,0.f,0.f};
		max = Vector3{0.f,0.f,0.f};
		return false;
	}
	min = hitbox->min;
	max = hitbox->max;
	return true;
}
std::vector<uint32_t> Model::GetHitboxBones(HitGroup group) const
{
	std::vector<uint32_t> bones;
	GetHitboxBones(group,bones);
	return bones;
}
void Model::GetHitboxBones(HitGroup group,std::vector<uint32_t> &boneIds) const
{
	boneIds.reserve(boneIds.size() +m_hitboxes.size());
	for(auto &it : m_hitboxes)
	{
		if(it.second.group == group)
			boneIds.push_back(it.first);
	}
}
std::vector<uint32_t> Model::GetHitboxBones() const
{
	std::vector<uint32_t> bones;
	GetHitboxBones(bones);
	return bones;
}
void Model::GetHitboxBones(std::vector<uint32_t> &boneIds) const
{
	boneIds.reserve(boneIds.size() +m_hitboxes.size());
	for(auto &it : m_hitboxes)
		boneIds.push_back(it.first);
}
