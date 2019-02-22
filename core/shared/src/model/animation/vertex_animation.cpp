#include "stdafx_shared.h"
#include "pragma/model/animation/vertex_animation.hpp"
#include "pragma/model/modelmesh.h"

MeshVertexFrame::MeshVertexFrame(const MeshVertexFrame &other)
	: std::enable_shared_from_this<MeshVertexFrame>(),
	m_vertices(other.m_vertices)
{}

const std::vector<std::array<uint16_t,3>> &MeshVertexFrame::GetVertices() const {return const_cast<MeshVertexFrame*>(this)->GetVertices();}
std::vector<std::array<uint16_t,3>> &MeshVertexFrame::GetVertices() {return m_vertices;}
void MeshVertexFrame::SetVertexCount(uint32_t count) {m_vertices.resize(count,std::array<uint16_t,3>{0,0,0});}
void MeshVertexFrame::SetVertexPosition(uint32_t vertId,const Vector3 &pos) {SetVertexPosition(vertId,std::array<uint16_t,3>{umath::float32_to_float16(pos.x),umath::float32_to_float16(pos.y),umath::float32_to_float16(pos.z)});}
void MeshVertexFrame::SetVertexPosition(uint32_t vertId,const std::array<uint16_t,3> &pos)
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
	pos = {umath::float16_to_float32(v.at(0)),umath::float16_to_float32(v.at(1)),umath::float16_to_float32(v.at(2))};
	return true;
}

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

/////////////////////

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

std::unique_ptr<VertexAnimation> VertexAnimation::Copy() const {return std::make_unique<VertexAnimation>(*this);}

void VertexAnimation::SetName(const std::string &name) {m_name = name;}
const std::string &VertexAnimation::GetName() const {return m_name;}

const std::vector<std::shared_ptr<MeshVertexAnimation>> &VertexAnimation::GetMeshAnimations() const {return const_cast<VertexAnimation*>(this)->GetMeshAnimations();}
std::vector<std::shared_ptr<MeshVertexAnimation>> &VertexAnimation::GetMeshAnimations() {return m_meshAnims;}
