/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/flex_animation.hpp"

std::shared_ptr<FlexAnimation> FlexAnimation::Load(std::shared_ptr<VFilePtrInternal> &f)
{
	auto version = f->Read<uint32_t>();
	if(version < 1 || version > FORMAT_VERSION)
		return nullptr;
	auto flexAnim = std::make_shared<FlexAnimation>();
	flexAnim->SetFps(f->Read<float>());

	auto numFlexControllerIds = f->Read<uint32_t>();
	auto &flexControllerIds = flexAnim->GetFlexControllerIds();
	flexControllerIds.resize(numFlexControllerIds);
	f->Read(flexControllerIds.data(),flexControllerIds.size() *sizeof(flexControllerIds.front()));

	auto numFrames = f->Read<uint32_t>();
	auto &frames = flexAnim->GetFrames();
	frames.reserve(numFrames);
	for(auto j=decltype(numFrames){0u};j<numFrames;++j)
	{
		auto &frame = flexAnim->AddFrame();
		auto &values = frame.GetValues();
		f->Read(values.data(),values.size() *sizeof(values.front()));
	}
	return flexAnim;
}
bool FlexAnimation::Save(std::shared_ptr<VFilePtrInternalReal> &f)
{
	f->Write<uint32_t>(FORMAT_VERSION);
	f->Write<float>(GetFps());

	auto &flexControllerIds = GetFlexControllerIds();
	f->Write<uint32_t>(flexControllerIds.size());
	for(auto id : flexControllerIds)
		f->Write<FlexControllerId>(id);

	auto &frames = GetFrames();
	f->Write<uint32_t>(frames.size());
	for(auto &frame : frames)
	{
		auto &values = frame->GetValues();
		assert(values.size() == flexControllerIds.size());
		f->Write(values.data(),flexControllerIds.size() *sizeof(values.front()));
	}
	return true;
}
uint32_t FlexAnimation::AddFlexControllerId(FlexControllerId id)
{
	auto &ids = GetFlexControllerIds();
	auto it = std::find(ids.begin(),ids.end(),id);
	if(it == ids.end())
	{
		ids.push_back(id);
		for(auto &frame : m_frames)
			frame->GetValues().push_back(0.f);
		it = ids.end() -1;
	}
	return it -ids.begin();
}
FlexAnimationFrame &FlexAnimation::AddFrame()
{
	m_frames.push_back(std::make_shared<FlexAnimationFrame>());
	auto &frame = m_frames.back();
	auto &values = frame->GetValues();
	values.resize(m_flexControllerIds.size());
	return *frame;
}
void FlexAnimation::SetFlexControllerIds(std::vector<FlexControllerId> &&ids)
{
	m_flexControllerIds = std::move(ids);
	for(auto &frame : m_frames)
		frame->GetValues().resize(m_flexControllerIds.size());
}
