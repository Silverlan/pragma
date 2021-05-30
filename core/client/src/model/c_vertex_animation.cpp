/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/model/animation/c_vertex_animation.hpp"

std::shared_ptr<CVertexAnimation> CVertexAnimation::Create()
{
	return std::shared_ptr<CVertexAnimation>(new CVertexAnimation{});
}
std::shared_ptr<CVertexAnimation> CVertexAnimation::Create(const CVertexAnimation &other)
{
	return std::shared_ptr<CVertexAnimation>(new CVertexAnimation{other});
}
std::shared_ptr<CVertexAnimation> CVertexAnimation::Create(const std::string &name)
{
	return std::shared_ptr<CVertexAnimation>(new CVertexAnimation{name});
}
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
std::shared_ptr<VertexAnimation> CVertexAnimation::Copy() const {return CVertexAnimation::Create(*this);}

void CVertexAnimation::UpdateBuffer()
{
	// TODO
}
