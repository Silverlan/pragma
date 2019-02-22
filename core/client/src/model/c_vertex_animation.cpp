#include "stdafx_client.h"
#include "pragma/model/animation/c_vertex_animation.hpp"

CVertexAnimation::CVertexAnimation()
	: VertexAnimation()
{}
CVertexAnimation::CVertexAnimation(const CVertexAnimation &other)
	: VertexAnimation(other)
{
	//m_buffer = other.m_buffer; // prosper TODO
}
CVertexAnimation::CVertexAnimation(const std::string &name)
	: VertexAnimation(name)
{}
std::unique_ptr<VertexAnimation> CVertexAnimation::Copy() const {return std::unique_ptr<VertexAnimation>(new CVertexAnimation(*this));}

void CVertexAnimation::UpdateBuffer()
{
	// TODO
}
