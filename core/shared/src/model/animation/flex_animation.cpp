/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/flex_animation.hpp"
#include <udm.hpp>

FlexAnimationFrame::FlexAnimationFrame(const FlexAnimationFrame &frame)
	: m_flexControllerValues{frame.m_flexControllerValues}
{
	static_assert(sizeof(FlexAnimationFrame) == 40,"Update this function when making changes to this class!");
}
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
std::shared_ptr<FlexAnimation> FlexAnimation::Load(const udm::AssetData &data,std::string &outErr)
{
	auto flexAnim = std::make_shared<FlexAnimation>();
	if(flexAnim->LoadFromAssetData(data,outErr) == false)
		return nullptr;
	return flexAnim;
}
bool FlexAnimation::LoadFromAssetData(const udm::AssetData &data,std::string &outErr)
{
	if(data.GetAssetType() != PFLEXANIM_IDENTIFIER)
	{
		outErr = "Incorrect format!";
		return false;
	}

	auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1)
	{
		outErr = "Invalid version!";
		return false;
	}
	// if(version > FORMAT_VERSION)
	// 	return false;
	m_fps = udm["fps"](m_fps);
	m_flexControllerIds = udm["flexControllers"](m_flexControllerIds);
	std::vector<float> blobData;
	if(udm["frameData"].GetBlobData(blobData) != udm::BlobResult::Success)
		return false;
	auto numFrames = !m_flexControllerIds.empty() ? (blobData.size() /m_flexControllerIds.size()) : 0;
	m_frames.resize(numFrames);
	uint32_t offset = 0;
	for(auto &frame : m_frames)
	{
		frame = std::make_shared<FlexAnimationFrame>();
		auto &frameValues = frame->GetValues();
		frameValues.resize(m_flexControllerIds.size());
		memcpy(frameValues.data(),&blobData[offset],frameValues.size() *sizeof(frameValues[0]));
		offset += frameValues.size();
	}
	return true;
}
bool FlexAnimation::Save(udm::AssetData &outData,std::string &outErr)
{
	outData.SetAssetType(PFLEXANIM_IDENTIFIER);
	outData.SetAssetVersion(FORMAT_VERSION);
	auto udm = *outData;

	udm["fps"] = GetFps();
	auto &flexControllerIds = GetFlexControllerIds();
	udm["flexControllers"] = flexControllerIds;

	auto &frames = GetFrames();
	std::vector<float> frameData;
	frameData.resize(frames.size() *flexControllerIds.size());
	uint32_t offset = 0;
	for(auto &frame : frames)
	{
		auto &frameValues = frame->GetValues();
		assert(frameValues.size() == flexControllerIds.size());
		memcpy(&frameData[offset],frameValues.data(),frameValues.size() *sizeof(frameValues[0]));
		offset += frameValues.size();
	}
	udm["frames"] = static_cast<uint32_t>(frames.size());
	udm["frameData"] = udm::compress_lz4_blob(frameData);
	return true;
}
bool FlexAnimation::SaveLegacy(std::shared_ptr<VFilePtrInternalReal> &f)
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
FlexAnimation::FlexAnimation(const FlexAnimation &other)
	: m_flexControllerIds{other.m_flexControllerIds},m_fps{other.m_fps},
	m_frames{other.m_frames}
{
	for(auto &frame : m_frames)
		frame = std::make_shared<FlexAnimationFrame>(*frame);
	static_assert(sizeof(FlexAnimation) == 72,"Update this function when making changes to this class!");
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
