// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.client;

import :model.vertex_animation;

std::shared_ptr<CVertexAnimation> CVertexAnimation::Create() { return std::shared_ptr<CVertexAnimation>(new CVertexAnimation {}); }
std::shared_ptr<CVertexAnimation> CVertexAnimation::Create(const CVertexAnimation &other) { return std::shared_ptr<CVertexAnimation>(new CVertexAnimation {other}); }
std::shared_ptr<CVertexAnimation> CVertexAnimation::Create(const std::string &name) { return std::shared_ptr<CVertexAnimation>(new CVertexAnimation {name}); }
CVertexAnimation::CVertexAnimation() : VertexAnimation() {}
CVertexAnimation::CVertexAnimation(const CVertexAnimation &other) : VertexAnimation(other)
{
	//m_buffer = other.m_buffer; // prosper TODO
}
CVertexAnimation::CVertexAnimation(const std::string &name) : VertexAnimation(name) {}
std::shared_ptr<VertexAnimation> CVertexAnimation::Copy() const { return CVertexAnimation::Create(*this); }

void CVertexAnimation::UpdateBuffer()
{
	// TODO
}
