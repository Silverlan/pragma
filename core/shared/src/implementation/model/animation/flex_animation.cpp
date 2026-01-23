// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :model.animation.enums;
import :model.animation.flex_animation;

FlexAnimationFrame::FlexAnimationFrame(const FlexAnimationFrame &frame) : m_flexControllerValues {frame.m_flexControllerValues} { static_assert(sizeof(FlexAnimationFrame) == 40, "Update this function when making changes to this class!"); }
bool FlexAnimationFrame::operator==(const FlexAnimationFrame &other) const
{
	static_assert(sizeof(FlexAnimationFrame) == 40, "Update this function when making changes to this class!");
	return m_flexControllerValues == other.m_flexControllerValues;
}
std::shared_ptr<FlexAnimation> FlexAnimation::Load(ufile::IFile &f)
{
	auto version = f.Read<uint32_t>();
	if(version < 1 || version > FORMAT_VERSION)
		return nullptr;
	auto flexAnim = pragma::util::make_shared<FlexAnimation>();
	flexAnim->SetFps(f.Read<float>());

	auto numFlexControllerIds = f.Read<uint32_t>();
	auto &flexControllerIds = flexAnim->GetFlexControllerIds();
	flexControllerIds.resize(numFlexControllerIds);
	f.Read(flexControllerIds.data(), flexControllerIds.size() * sizeof(flexControllerIds.front()));

	auto numFrames = f.Read<uint32_t>();
	auto &frames = flexAnim->GetFrames();
	frames.reserve(numFrames);
	for(auto j = decltype(numFrames) {0u}; j < numFrames; ++j) {
		auto &frame = flexAnim->AddFrame();
		auto &values = frame.GetValues();
		f.Read(values.data(), values.size() * sizeof(values.front()));
	}
	return flexAnim;
}
std::shared_ptr<FlexAnimation> FlexAnimation::Load(const udm::AssetData &data, std::string &outErr)
{
	auto flexAnim = pragma::util::make_shared<FlexAnimation>();
	if(flexAnim->LoadFromAssetData(data, outErr) == false)
		return nullptr;
	return flexAnim;
}
bool FlexAnimation::LoadFromAssetData(const udm::AssetData &data, std::string &outErr)
{
	if(data.GetAssetType() != PFLEXANIM_IDENTIFIER) {
		outErr = "Incorrect format!";
		return false;
	}

	auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return false;
	}
	// if(version > FORMAT_VERSION)
	// 	return false;
	//udm["fps"](m_fps);

	auto udmNodes = udm["nodes"];
	auto udmNode = udmNodes[0];
	std::string type;
	udmNode["type"](type);
	if(type == "flexController")
		udmNode["set"](m_flexControllerIds);

	udm["fps"](m_fps);
	std::vector<float> times {};
	auto udmChannels = udm["channels"];
	auto numChannels = udmChannels.GetSize();
	for(auto i = decltype(numChannels) {0u}; i < numChannels; ++i) {
		auto udmChannel = udmChannels[i];
		if(i == 0) {
			udmChannel["times"](times);
			m_frames.resize(times.size());
			for(auto &frame : m_frames) {
				frame = pragma::util::make_shared<FlexAnimationFrame>();
				frame->GetValues().resize(m_flexControllerIds.size());
			}
		}

		uint16_t nodeIdx = 0;
		udmChannel["node"](nodeIdx);
		std::string property;
		udmChannel["property"](property);
		std::vector<float> values;
		udmChannel["values"](values);

		for(auto j = decltype(values.size()) {0u}; j < values.size(); ++j)
			m_frames[j]->GetValues()[nodeIdx] = values[j];
	}
	return true;
}
bool FlexAnimation::Save(udm::AssetDataArg outData, std::string &outErr)
{
	outData.SetAssetType(PFLEXANIM_IDENTIFIER);
	outData.SetAssetVersion(FORMAT_VERSION);
	auto udm = *outData;

	auto udmNodes = udm.AddArray("nodes", 1);
	auto udmNode = udmNodes[0];
	udmNode["type"] = "flexController";
	udmNode["set"] = m_flexControllerIds;

	std::vector<float> times {};
	std::vector<std::vector<float>> values;
	times.resize(m_frames.size());
	values.resize(m_flexControllerIds.size());
	for(auto &v : values)
		v.resize(m_frames.size());
	for(auto i = decltype(m_frames.size()) {0u}; i < m_frames.size(); ++i) {
		times[i] = i / m_fps;

		auto &frame = *m_frames[i];
		auto &frameValues = frame.GetValues();
		for(auto j = decltype(frameValues.size()) {0u}; j < frameValues.size(); ++j)
			values[j][i] = frameValues[j];
	}

	udm["fps"] = GetFps();
	auto udmChannels = udm.AddArray("channels", m_flexControllerIds.size());
	for(auto i = decltype(m_flexControllerIds.size()) {0u}; i < m_flexControllerIds.size(); ++i) {
		auto udmChannel = udmChannels[i];
		auto flexControllerId = m_flexControllerIds[i];
		udmChannel["node"] = static_cast<uint16_t>(i);

		udmChannel["times"] = times;
		udmChannel["property"] = "value";
		udmChannel.AddArray("values", values[i], udm::ArrayType::Compressed);
	}
	return true;
}
bool FlexAnimation::SaveLegacy(std::shared_ptr<pragma::fs::VFilePtrInternalReal> &f)
{
	f->Write<uint32_t>(FORMAT_VERSION);
	f->Write<float>(GetFps());

	auto &flexControllerIds = GetFlexControllerIds();
	f->Write<uint32_t>(flexControllerIds.size());
	for(auto id : flexControllerIds)
		f->Write<pragma::animation::FlexControllerId>(id);

	auto &frames = GetFrames();
	f->Write<uint32_t>(frames.size());
	for(auto &frame : frames) {
		auto &values = frame->GetValues();
		assert(values.size() == flexControllerIds.size());
		f->Write(values.data(), flexControllerIds.size() * sizeof(values.front()));
	}
	return true;
}
FlexAnimation::FlexAnimation(const FlexAnimation &other) : m_flexControllerIds {other.m_flexControllerIds}, m_fps {other.m_fps}, m_frames {other.m_frames}
{
	for(auto &frame : m_frames)
		frame = pragma::util::make_shared<FlexAnimationFrame>(*frame);
	static_assert(sizeof(FlexAnimation) == 72, "Update this function when making changes to this class!");
}
uint32_t FlexAnimation::AddFlexControllerId(pragma::animation::FlexControllerId id)
{
	auto &ids = GetFlexControllerIds();
	auto it = std::find(ids.begin(), ids.end(), id);
	if(it == ids.end()) {
		ids.push_back(id);
		for(auto &frame : m_frames)
			frame->GetValues().push_back(0.f);
		it = ids.end() - 1;
	}
	return it - ids.begin();
}
FlexAnimationFrame &FlexAnimation::AddFrame()
{
	m_frames.push_back(pragma::util::make_shared<FlexAnimationFrame>());
	auto &frame = m_frames.back();
	auto &values = frame->GetValues();
	values.resize(m_flexControllerIds.size());
	return *frame;
}
bool FlexAnimation::operator==(const FlexAnimation &other) const
{
	static_assert(sizeof(FlexAnimation) == 72, "Update this function when making changes to this class!");
	if(!(m_frames.size() == other.m_frames.size() && m_flexControllerIds == other.m_flexControllerIds && pragma::math::abs(m_fps - other.m_fps) < 0.001f))
		return false;
	for(auto i = decltype(m_frames.size()) {0u}; i < m_frames.size(); ++i) {
		if(*m_frames[i] != *other.m_frames[i])
			return false;
	}
	return true;
}
void FlexAnimation::SetFlexControllerIds(std::vector<pragma::animation::FlexControllerId> &&ids)
{
	m_flexControllerIds = std::move(ids);
	for(auto &frame : m_frames)
		frame->GetValues().resize(m_flexControllerIds.size());
}
