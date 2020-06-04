/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/vertex_animation.hpp"
#include "pragma/model/modelmesh.h"

MeshVertexFrame::MeshVertexFrame(const MeshVertexFrame &other)
	: std::enable_shared_from_this<MeshVertexFrame>(),
	m_vertices(other.m_vertices),m_normals{other.m_normals},
	m_flags{other.m_flags}
{}

const std::vector<std::array<uint16_t,4>> &MeshVertexFrame::GetVertices() const {return const_cast<MeshVertexFrame*>(this)->GetVertices();}
std::vector<std::array<uint16_t,4>> &MeshVertexFrame::GetVertices() {return m_vertices;}
const std::vector<std::array<uint16_t,4>> &MeshVertexFrame::GetNormals() const {return const_cast<MeshVertexFrame*>(this)->GetNormals();}
std::vector<std::array<uint16_t,4>> &MeshVertexFrame::GetNormals() {return m_normals;}
void MeshVertexFrame::SetVertexCount(uint32_t count)
{
	m_vertices.resize(count,std::array<uint16_t,4>{0,0,0,0});
	if(IsFlagEnabled(Flags::HasNormals))
		m_normals.resize(count);
}
uint32_t MeshVertexFrame::GetVertexCount() const {return m_vertices.size();}
void MeshVertexFrame::SetDeltaValue(uint32_t vertId,float deltaValue) {SetDeltaValue(vertId,static_cast<uint16_t>(umath::float32_to_float16_glm(deltaValue)));}
void MeshVertexFrame::SetDeltaValue(uint32_t vertId,uint16_t deltaValue)
{
	if(vertId >= m_vertices.size())
		return;
	m_vertices.at(vertId).at(3) = deltaValue;
}
bool MeshVertexFrame::GetDeltaValue(uint32_t vertId,float &deltaValue) const
{
	if(vertId >= m_vertices.size())
		return false;
	deltaValue = umath::float16_to_float32_glm(m_vertices.at(vertId).at(3));
	return true;
}

void MeshVertexFrame::SetVertexPosition(uint32_t vertId,const Vector3 &pos) {SetVertexPosition(vertId,std::array<uint16_t,3>{static_cast<uint16_t>(umath::float32_to_float16_glm(pos.x)),static_cast<uint16_t>(umath::float32_to_float16_glm(pos.y)),static_cast<uint16_t>(umath::float32_to_float16_glm(pos.z))});}
void MeshVertexFrame::SetVertexPosition(uint32_t vertId,const std::array<uint16_t,3> &pos)
{
	if(vertId >= m_vertices.size())
		return;
	for(uint8_t i=0;i<3;++i)
		m_vertices.at(vertId).at(i) = pos.at(i);
}
void MeshVertexFrame::SetVertexPosition(uint32_t vertId,const std::array<uint16_t,4> &pos)
{
	if(vertId >= m_vertices.size())
		return;
	m_vertices.at(vertId) = pos;
}
bool MeshVertexFrame::GetVertexPosition(uint32_t vertId,Vector3 &pos) const
{
	if(vertId >= m_vertices.size())
		return false;
	auto &v = m_vertices.at(vertId);
	pos = {umath::float16_to_float32_glm(v.at(0)),umath::float16_to_float32_glm(v.at(1)),umath::float16_to_float32_glm(v.at(2))};
	return true;
}

void MeshVertexFrame::SetVertexNormal(uint32_t vertId,const Vector3 &n) {SetVertexNormal(vertId,std::array<uint16_t,3>{static_cast<uint16_t>(umath::float32_to_float16_glm(n.x)),static_cast<uint16_t>(umath::float32_to_float16_glm(n.y)),static_cast<uint16_t>(umath::float32_to_float16_glm(n.z))});}
void MeshVertexFrame::SetVertexNormal(uint32_t vertId,const std::array<uint16_t,3> &n)
{
	if(vertId >= m_normals.size())
		return;
	for(uint8_t i=0;i<3;++i)
		m_normals.at(vertId).at(i) = n.at(i);
}
void MeshVertexFrame::SetVertexNormal(uint32_t vertId,const std::array<uint16_t,4> &n)
{
	if(vertId >= m_normals.size())
		return;
	m_normals.at(vertId) = n;
}
bool MeshVertexFrame::GetVertexNormal(uint32_t vertId,Vector3 &n) const
{
	if(vertId >= m_normals.size())
		return false;
	auto &v = m_normals.at(vertId);
	n = {umath::float16_to_float32_glm(v.at(0)),umath::float16_to_float32_glm(v.at(1)),umath::float16_to_float32_glm(v.at(2))};
	return true;
}

void MeshVertexFrame::Rotate(const Quat &rot)
{
	auto numVerts = GetVertexCount();
	for(auto i=decltype(numVerts){0u};i<numVerts;++i)
	{
		Vector3 pos;
		if(GetVertexPosition(i,pos) == false)
			continue;
		uvec::rotate(&pos,rot);
		SetVertexPosition(i,pos);
	}
}
void MeshVertexFrame::Scale(const Vector3 &scale)
{
	auto numVerts = GetVertexCount();
	for(auto i=decltype(numVerts){0u};i<numVerts;++i)
	{
		Vector3 pos;
		if(GetVertexPosition(i,pos) == false)
			continue;
		pos *= scale;
		SetVertexPosition(i,pos);
	}
}
void MeshVertexFrame::SetFlags(Flags flags)
{
	m_flags = flags;
	if(IsFlagEnabled(Flags::HasNormals))
	{
		if(m_normals.size() != m_vertices.size())
			m_normals.resize(m_vertices.size());
	}
	else
		m_normals.clear();
}
void MeshVertexFrame::SetFlagEnabled(Flags flags,bool enabled)
{
	auto newFlags = m_flags;
	umath::set_flag(newFlags,flags,enabled);
	SetFlags(newFlags);
}
bool MeshVertexFrame::IsFlagEnabled(Flags flags) const {return umath::is_flag_set(m_flags,flags);}
MeshVertexFrame::Flags MeshVertexFrame::GetFlags() const {return m_flags;}

/////////////////////

MeshVertexAnimation::MeshVertexAnimation(const MeshVertexAnimation &other)
	: std::enable_shared_from_this<MeshVertexAnimation>(),m_wpMesh(other.m_wpMesh),m_wpSubMesh(other.m_wpSubMesh)
{
	m_frames.reserve(other.m_frames.size());
	for(auto &frame : m_frames)
		m_frames.push_back(std::make_shared<MeshVertexFrame>(*frame));
}
ModelMesh *MeshVertexAnimation::GetMesh() const {return m_wpMesh.lock().get();}
ModelSubMesh *MeshVertexAnimation::GetSubMesh() const {return m_wpSubMesh.lock().get();}
void MeshVertexAnimation::SetMesh(ModelMesh &mesh,ModelSubMesh &subMesh)
{
	m_wpMesh = mesh.shared_from_this();
	m_wpSubMesh = subMesh.shared_from_this();
}
const std::vector<std::shared_ptr<MeshVertexFrame>> &MeshVertexAnimation::GetFrames() const {return const_cast<MeshVertexAnimation*>(this)->GetFrames();}
std::vector<std::shared_ptr<MeshVertexFrame>> &MeshVertexAnimation::GetFrames() {return m_frames;}
const MeshVertexFrame *MeshVertexAnimation::GetFrame(uint32_t frameId) const {return const_cast<MeshVertexAnimation*>(this)->GetFrame(frameId);}
MeshVertexFrame *MeshVertexAnimation::GetFrame(uint32_t frameId) {return (frameId < m_frames.size()) ? m_frames.at(frameId).get() : nullptr;}
std::shared_ptr<MeshVertexFrame> MeshVertexAnimation::AddFrame()
{
	if(m_wpSubMesh.expired() == true)
		return nullptr;
	m_frames.push_back(std::make_shared<MeshVertexFrame>());
	m_frames.back()->SetVertexCount(m_wpSubMesh.lock()->GetVertexCount());
	return m_frames.back();
}
void MeshVertexAnimation::Rotate(const Quat &rot)
{
	for(auto &frame : m_frames)
		frame->Rotate(rot);
}
void MeshVertexAnimation::Scale(const Vector3 &scale)
{
	for(auto &frame : m_frames)
		frame->Scale(scale);
}

/////////////////////

std::shared_ptr<VertexAnimation> VertexAnimation::Create()
{
	return std::shared_ptr<VertexAnimation>(new VertexAnimation{});
}
std::shared_ptr<VertexAnimation> VertexAnimation::Create(const VertexAnimation &other)
{
	return std::shared_ptr<VertexAnimation>(new VertexAnimation{other});
}
std::shared_ptr<VertexAnimation> VertexAnimation::Create(const std::string &name)
{
	return std::shared_ptr<VertexAnimation>(new VertexAnimation{name});
}
VertexAnimation::VertexAnimation(const std::string &name)
	: m_name(name)
{}

VertexAnimation::VertexAnimation(const VertexAnimation &other)
	: m_name(other.m_name)
{
	m_meshAnims.reserve(other.m_meshAnims.size());
	for(auto &anim : m_meshAnims)
		m_meshAnims.push_back(std::make_shared<MeshVertexAnimation>(*anim));
}

bool VertexAnimation::GetMeshAnimationId(ModelSubMesh &subMesh,uint32_t &id) const
{
	auto it = std::find_if(m_meshAnims.begin(),m_meshAnims.end(),[&subMesh](const std::shared_ptr<MeshVertexAnimation> &anim) {
		return (anim->GetSubMesh() == &subMesh) ? true : false;
	});
	if(it == m_meshAnims.end())
		return false;
	id = it -m_meshAnims.begin();
	return true;
}
const MeshVertexAnimation *VertexAnimation::GetMeshAnimation(ModelSubMesh &subMesh) const {return const_cast<VertexAnimation*>(this)->GetMeshAnimation(subMesh);}
MeshVertexAnimation *VertexAnimation::GetMeshAnimation(ModelSubMesh &subMesh)
{
	auto frameId = 0u;
	if(GetMeshAnimationId(subMesh,frameId) == false)
		return nullptr;
	return m_meshAnims.at(frameId).get();
}

std::shared_ptr<MeshVertexFrame> VertexAnimation::AddMeshFrame(ModelMesh &mesh,ModelSubMesh &subMesh)
{
	auto *anim = GetMeshAnimation(subMesh);
	if(anim == nullptr)
	{
		auto meshAnim = std::make_shared<MeshVertexAnimation>();
		meshAnim->SetMesh(mesh,subMesh);
		m_meshAnims.push_back(meshAnim);
		anim = meshAnim.get();
	}
	return anim->AddFrame();
}

const MeshVertexFrame *VertexAnimation::GetMeshFrame(ModelSubMesh &subMesh,uint32_t frameId) const {return const_cast<VertexAnimation*>(this)->GetMeshFrame(subMesh,frameId);}
MeshVertexFrame *VertexAnimation::GetMeshFrame(ModelSubMesh &subMesh,uint32_t frameId)
{
	auto animId = 0u;
	if(GetMeshAnimationId(subMesh,animId) == false)
		return nullptr;
	return m_meshAnims.at(animId)->GetFrame(frameId);
}

std::shared_ptr<VertexAnimation> VertexAnimation::Copy() const {return VertexAnimation::Create(*this);}

void VertexAnimation::SetName(const std::string &name) {m_name = name;}
const std::string &VertexAnimation::GetName() const {return m_name;}

const std::vector<std::shared_ptr<MeshVertexAnimation>> &VertexAnimation::GetMeshAnimations() const {return const_cast<VertexAnimation*>(this)->GetMeshAnimations();}
std::vector<std::shared_ptr<MeshVertexAnimation>> &VertexAnimation::GetMeshAnimations() {return m_meshAnims;}

void VertexAnimation::Rotate(const Quat &rot)
{
	for(auto &meshAnim : m_meshAnims)
		meshAnim->Rotate(rot);
}
void VertexAnimation::Scale(const Vector3 &scale)
{
	for(auto &meshAnim : m_meshAnims)
		meshAnim->Scale(scale);
}
