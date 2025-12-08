// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :model.vertex_animation;

std::shared_ptr<pragma::animation::CVertexAnimation> pragma::animation::CVertexAnimation::Create() { return std::shared_ptr<pragma::animation::CVertexAnimation>(new pragma::animation::CVertexAnimation {}); }
std::shared_ptr<pragma::animation::CVertexAnimation> pragma::animation::CVertexAnimation::Create(const pragma::animation::CVertexAnimation &other) { return std::shared_ptr<pragma::animation::CVertexAnimation>(new pragma::animation::CVertexAnimation {other}); }
std::shared_ptr<pragma::animation::CVertexAnimation> pragma::animation::CVertexAnimation::Create(const std::string &name) { return std::shared_ptr<pragma::animation::CVertexAnimation>(new pragma::animation::CVertexAnimation {name}); }
pragma::animation::CVertexAnimation::CVertexAnimation() : VertexAnimation() {}
pragma::animation::CVertexAnimation::CVertexAnimation(const pragma::animation::CVertexAnimation &other) : VertexAnimation(other)
{
	//m_buffer = other.m_buffer; // prosper TODO
}
pragma::animation::CVertexAnimation::CVertexAnimation(const std::string &name) : VertexAnimation(name) {}
std::shared_ptr<pragma::animation::VertexAnimation> pragma::animation::CVertexAnimation::Copy() const { return pragma::animation::CVertexAnimation::Create(*this); }

void pragma::animation::CVertexAnimation::UpdateBuffer()
{
	// TODO
}
