// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.animation.vertex_animation;

pragma::animation::MeshVertexFrame::MeshVertexFrame(const MeshVertexFrame &other) : std::enable_shared_from_this<MeshVertexFrame>(), m_vertices(other.m_vertices), m_normals {other.m_normals}, m_flags {other.m_flags}
{
	static_assert(sizeof(MeshVertexFrame) == 72, "Update this function when making changes to this class!");
}

const std::vector<std::array<uint16_t, 4>> &pragma::animation::MeshVertexFrame::GetVertices() const { return const_cast<MeshVertexFrame *>(this)->GetVertices(); }
std::vector<std::array<uint16_t, 4>> &pragma::animation::MeshVertexFrame::GetVertices() { return m_vertices; }
const std::vector<std::array<uint16_t, 4>> &pragma::animation::MeshVertexFrame::GetNormals() const { return const_cast<MeshVertexFrame *>(this)->GetNormals(); }
std::vector<std::array<uint16_t, 4>> &pragma::animation::MeshVertexFrame::GetNormals() { return m_normals; }
void pragma::animation::MeshVertexFrame::SetVertexCount(uint32_t count)
{
	m_vertices.resize(count, std::array<uint16_t, 4> {0, 0, 0, 0});
	if(IsFlagEnabled(Flags::HasNormals))
		m_normals.resize(count);
}
uint32_t pragma::animation::MeshVertexFrame::GetVertexCount() const { return m_vertices.size(); }
void pragma::animation::MeshVertexFrame::SetDeltaValue(uint32_t vertId, float deltaValue) { SetDeltaValue(vertId, static_cast<uint16_t>(math::float32_to_float16_glm(deltaValue))); }
void pragma::animation::MeshVertexFrame::SetDeltaValue(uint32_t vertId, uint16_t deltaValue)
{
	if(vertId >= m_vertices.size())
		return;
	m_vertices.at(vertId).at(3) = deltaValue;
}
bool pragma::animation::MeshVertexFrame::GetDeltaValue(uint32_t vertId, float &deltaValue) const
{
	if(vertId >= m_vertices.size())
		return false;
	deltaValue = math::float16_to_float32_glm(m_vertices.at(vertId).at(3));
	return true;
}

void pragma::animation::MeshVertexFrame::SetVertexPosition(uint32_t vertId, const Vector3 &pos)
{
	SetVertexPosition(vertId, std::array<uint16_t, 3> {static_cast<uint16_t>(math::float32_to_float16_glm(pos.x)), static_cast<uint16_t>(math::float32_to_float16_glm(pos.y)), static_cast<uint16_t>(math::float32_to_float16_glm(pos.z))});
}
void pragma::animation::MeshVertexFrame::SetVertexPosition(uint32_t vertId, const std::array<uint16_t, 3> &pos)
{
	if(vertId >= m_vertices.size())
		return;
	for(uint8_t i = 0; i < 3; ++i)
		m_vertices.at(vertId).at(i) = pos.at(i);
}
void pragma::animation::MeshVertexFrame::SetVertexPosition(uint32_t vertId, const std::array<uint16_t, 4> &pos)
{
	if(vertId >= m_vertices.size())
		return;
	m_vertices.at(vertId) = pos;
}
bool pragma::animation::MeshVertexFrame::GetVertexPosition(uint32_t vertId, Vector3 &pos) const
{
	if(vertId >= m_vertices.size())
		return false;
	auto &v = m_vertices.at(vertId);
	pos = {math::float16_to_float32_glm(v.at(0)), math::float16_to_float32_glm(v.at(1)), math::float16_to_float32_glm(v.at(2))};
	return true;
}

void pragma::animation::MeshVertexFrame::SetVertexNormal(uint32_t vertId, const Vector3 &n)
{
	SetVertexNormal(vertId, std::array<uint16_t, 3> {static_cast<uint16_t>(math::float32_to_float16_glm(n.x)), static_cast<uint16_t>(math::float32_to_float16_glm(n.y)), static_cast<uint16_t>(math::float32_to_float16_glm(n.z))});
}
void pragma::animation::MeshVertexFrame::SetVertexNormal(uint32_t vertId, const std::array<uint16_t, 3> &n)
{
	if(vertId >= m_normals.size())
		return;
	for(uint8_t i = 0; i < 3; ++i)
		m_normals.at(vertId).at(i) = n.at(i);
}
void pragma::animation::MeshVertexFrame::SetVertexNormal(uint32_t vertId, const std::array<uint16_t, 4> &n)
{
	if(vertId >= m_normals.size())
		return;
	m_normals.at(vertId) = n;
}
bool pragma::animation::MeshVertexFrame::GetVertexNormal(uint32_t vertId, Vector3 &n) const
{
	if(vertId >= m_normals.size())
		return false;
	auto &v = m_normals.at(vertId);
	n = {math::float16_to_float32_glm(v.at(0)), math::float16_to_float32_glm(v.at(1)), math::float16_to_float32_glm(v.at(2))};
	return true;
}

void pragma::animation::MeshVertexFrame::Rotate(const Quat &rot)
{
	auto numVerts = GetVertexCount();
	for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
		Vector3 pos;
		if(GetVertexPosition(i, pos) == false)
			continue;
		uvec::rotate(&pos, rot);
		SetVertexPosition(i, pos);
	}
}
void pragma::animation::MeshVertexFrame::Scale(const Vector3 &scale)
{
	auto numVerts = GetVertexCount();
	for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
		Vector3 pos;
		if(GetVertexPosition(i, pos) == false)
			continue;
		pos *= scale;
		SetVertexPosition(i, pos);
	}
}
void pragma::animation::MeshVertexFrame::Mirror(Axis axis)
{
	auto transform = asset::get_mirror_transform_vector(axis);
	auto numVerts = GetVertexCount();
	for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
		Vector3 pos;
		if(GetVertexPosition(i, pos)) {
			pos *= transform;
			SetVertexPosition(i, pos);
		}

		Vector3 norm;
		if(GetVertexNormal(i, norm)) {
			norm *= transform;
			SetVertexNormal(i, norm);
		}
	}
}
bool pragma::animation::MeshVertexFrame::operator==(const MeshVertexFrame &other) const
{
	if(m_vertices.size() != other.m_vertices.size() || m_normals.size() != other.m_normals.size())
		return false;
	for(auto i = decltype(m_vertices.size()) {0u}; i < m_vertices.size(); ++i) {
		Vector3 p0, p1;
		if(GetVertexPosition(i, p0) != GetVertexPosition(i, p1) || uvec::cmp(p0, p1) == false)
			return false;
	}
	for(auto i = decltype(m_normals.size()) {0u}; i < m_normals.size(); ++i) {
		Vector3 p0, p1;
		if(GetVertexNormal(i, p0) != GetVertexNormal(i, p1) || uvec::cmp(p0, p1) == false)
			return false;
	}
	static_assert(sizeof(MeshVertexFrame) == 72, "Update this function when making changes to this class!");
	return m_flags == other.m_flags;
}
void pragma::animation::MeshVertexFrame::SetFlags(Flags flags)
{
	m_flags = flags;
	if(IsFlagEnabled(Flags::HasNormals)) {
		if(m_normals.size() != m_vertices.size())
			m_normals.resize(m_vertices.size());
	}
	else
		m_normals.clear();
}
void pragma::animation::MeshVertexFrame::SetFlagEnabled(Flags flags, bool enabled)
{
	auto newFlags = m_flags;
	math::set_flag(newFlags, flags, enabled);
	SetFlags(newFlags);
}
bool pragma::animation::MeshVertexFrame::IsFlagEnabled(Flags flags) const { return math::is_flag_set(m_flags, flags); }
pragma::animation::MeshVertexFrame::Flags pragma::animation::MeshVertexFrame::GetFlags() const { return m_flags; }

/////////////////////

pragma::animation::MeshVertexAnimation::MeshVertexAnimation(const MeshVertexAnimation &other) : std::enable_shared_from_this<MeshVertexAnimation>(), m_wpMesh(other.m_wpMesh), m_wpSubMesh(other.m_wpSubMesh)
{
	m_frames.reserve(other.m_frames.size());
	for(auto &frame : m_frames)
		m_frames.push_back(pragma::util::make_shared<MeshVertexFrame>(*frame));
	static_assert(sizeof(MeshVertexAnimation) == 72, "Update this function when making changes to this class!");
}
pragma::geometry::ModelMesh *pragma::animation::MeshVertexAnimation::GetMesh() const { return m_wpMesh.lock().get(); }
pragma::geometry::ModelSubMesh *pragma::animation::MeshVertexAnimation::GetSubMesh() const { return m_wpSubMesh.lock().get(); }
void pragma::animation::MeshVertexAnimation::SetMesh(geometry::ModelMesh &mesh, geometry::ModelSubMesh &subMesh)
{
	m_wpMesh = mesh.shared_from_this();
	m_wpSubMesh = subMesh.shared_from_this();
}
const std::vector<std::shared_ptr<pragma::animation::MeshVertexFrame>> &pragma::animation::MeshVertexAnimation::GetFrames() const { return const_cast<MeshVertexAnimation *>(this)->GetFrames(); }
std::vector<std::shared_ptr<pragma::animation::MeshVertexFrame>> &pragma::animation::MeshVertexAnimation::GetFrames() { return m_frames; }
const pragma::animation::MeshVertexFrame *pragma::animation::MeshVertexAnimation::GetFrame(uint32_t frameId) const { return const_cast<MeshVertexAnimation *>(this)->GetFrame(frameId); }
pragma::animation::MeshVertexFrame *pragma::animation::MeshVertexAnimation::GetFrame(uint32_t frameId) { return (frameId < m_frames.size()) ? m_frames.at(frameId).get() : nullptr; }
std::shared_ptr<pragma::animation::MeshVertexFrame> pragma::animation::MeshVertexAnimation::AddFrame()
{
	if(m_wpSubMesh.expired() == true)
		return nullptr;
	m_frames.push_back(pragma::util::make_shared<MeshVertexFrame>());
	m_frames.back()->SetVertexCount(m_wpSubMesh.lock()->GetVertexCount());
	return m_frames.back();
}
void pragma::animation::MeshVertexAnimation::Rotate(const Quat &rot)
{
	for(auto &frame : m_frames)
		frame->Rotate(rot);
}
void pragma::animation::MeshVertexAnimation::Scale(const Vector3 &scale)
{
	for(auto &frame : m_frames)
		frame->Scale(scale);
}
void pragma::animation::MeshVertexAnimation::Mirror(Axis axis)
{
	for(auto &frame : m_frames)
		frame->Mirror(axis);
}
bool pragma::animation::MeshVertexAnimation::operator==(const MeshVertexAnimation &other) const
{
	if(m_frames.size() != other.m_frames.size() || m_wpMesh.expired() != other.m_wpMesh.expired() || m_wpSubMesh.expired() != other.m_wpSubMesh.expired())
		return false;
	for(auto i = decltype(m_frames.size()) {0u}; i < other.m_frames.size(); ++i) {
		if(*m_frames[i] != *other.m_frames[i])
			return false;
	}
	static_assert(sizeof(MeshVertexAnimation) == 72, "Update this function when making changes to this class!");
	return true;
}

/////////////////////

std::shared_ptr<pragma::animation::VertexAnimation> pragma::animation::VertexAnimation::Create() { return std::shared_ptr<VertexAnimation>(new VertexAnimation {}); }
std::shared_ptr<pragma::animation::VertexAnimation> pragma::animation::VertexAnimation::Create(const VertexAnimation &other) { return std::shared_ptr<VertexAnimation>(new VertexAnimation {other}); }
std::shared_ptr<pragma::animation::VertexAnimation> pragma::animation::VertexAnimation::Create(const std::string &name) { return std::shared_ptr<VertexAnimation>(new VertexAnimation {name}); }
pragma::animation::VertexAnimation::VertexAnimation(const std::string &name) : m_name(name) {}

pragma::animation::VertexAnimation::VertexAnimation(const VertexAnimation &other) : m_name(other.m_name)
{
	m_meshAnims.reserve(other.m_meshAnims.size());
	for(auto &anim : m_meshAnims)
		m_meshAnims.push_back(pragma::util::make_shared<MeshVertexAnimation>(*anim));
	static_assert(layout_version == 1, "Update this function when making changes to this class!");
}

bool pragma::animation::VertexAnimation::operator==(const VertexAnimation &other) const
{
	if(m_name != other.m_name || m_meshAnims.size() != other.m_meshAnims.size())
		return false;
	for(auto i = decltype(m_meshAnims.size()) {0u}; i < m_meshAnims.size(); ++i) {
		if(*m_meshAnims[i] != *other.m_meshAnims[i])
			return false;
	}
	static_assert(layout_version == 1, "Update this function when making changes to this class!");
	return true;
}

std::shared_ptr<pragma::animation::VertexAnimation> pragma::animation::VertexAnimation::Load(asset::Model &mdl, const udm::AssetData &data, std::string &outErr)
{
	auto morphAnim = Create();
	if(morphAnim->LoadFromAssetData(mdl, data, outErr) == false)
		return nullptr;
	return morphAnim;
}
bool pragma::animation::VertexAnimation::Save(asset::Model &mdl, udm::AssetDataArg outData, std::string &outErr)
{
	outData.SetAssetType(PMORPHANI_IDENTIFIER);
	outData.SetAssetVersion(FORMAT_VERSION);
	auto udm = *outData;
	udm["name"] = GetName();

	auto writeFlag = [](auto udm, auto flag, const std::string &name, auto flags) {
		if(math::is_flag_set(flags, flag) == false)
			return;
		udm["flags"][name] = true;
	};

	auto strctHalfVector4 = udm::StructDescription::Define<udm::Half, udm::Half, udm::Half, udm::Half>({"x", "y", "z", "w"});
	auto &meshAnims = GetMeshAnimations();
	auto udmMeshAnims = udm.AddArray("meshAnimations", meshAnims.size());
	uint32_t meshAnimIdx = 0;
	for(auto &ma : meshAnims) {
		auto *mesh = ma->GetMesh();
		auto *subMesh = ma->GetSubMesh();
		if(mesh == nullptr || subMesh == nullptr)
			continue;
		auto udmMa = udmMeshAnims[meshAnimIdx++];
		uint32_t groupIdx, meshIdx, subMeshIdx;
		if(mdl.FindSubMeshIndex(nullptr, mesh, subMesh, groupIdx, meshIdx, subMeshIdx) == false)
			continue;
		udmMa["meshGroup"] = groupIdx;
		udmMa["mesh"] = meshIdx;
		udmMa["subMesh"] = subMeshIdx;

		auto &frames = ma->GetFrames();
		auto udmFrames = udmMa.AddArray("frames", frames.size());
		uint32_t frameIdx = 0;
		for(auto &frame : frames) {
			auto udmFrame = udmFrames[frameIdx++];
			writeFlag(udmFrame, MeshVertexFrame::Flags::HasDeltaValues, "hasDeltaValues", frame->GetFlags());
			writeFlag(udmFrame, MeshVertexFrame::Flags::HasNormals, "hasNormals", frame->GetFlags());
			static_assert(math::to_integral(MeshVertexFrame::Flags::Count) == 2, "Update this list when new flags have been added!");

			auto flags = frame->GetFlags();
			struct Attribute {
				Attribute(const std::string &name, const std::vector<std::array<uint16_t, 4>> &vertexData) : name {name}, vertexData {vertexData} {}
				std::string name;
				const std::vector<std::array<uint16_t, 4>> &vertexData;
			};
			std::vector<Attribute> attributes {};
			attributes.push_back(Attribute {"position", frame->GetVertices()});
			if(math::is_flag_set(flags, MeshVertexFrame::Flags::HasNormals))
				attributes.push_back(Attribute {"normal", frame->GetNormals()});
			std::set<uint16_t> usedVertIndicesSet {};
			std::vector<uint16_t> usedVertIndices {};
			for(auto &attr : attributes) {
				auto vertIdx = 0u;
				auto &vdata = attr.vertexData.at(vertIdx);
				usedVertIndices.reserve(vdata.size());
				for(auto &vdata : attr.vertexData) {
					auto itUsed = std::find_if(vdata.begin(), vdata.end(), [](const uint16_t &v) { return v != 0; });
					if(itUsed != vdata.end()) {
						if(usedVertIndicesSet.find(vertIdx) == usedVertIndicesSet.end()) {
							usedVertIndicesSet.insert(vertIdx);
							usedVertIndices.push_back(vertIdx);
						}
					}
					++vertIdx;
				}
			}

			udmFrame.AddArray("vertexIndices", usedVertIndices, udm::ArrayType::Compressed);
			auto udmAttributes = udmFrame.AddArray("attributes", attributes.size());
			for(auto i = decltype(attributes.size()) {0u}; i < attributes.size(); ++i) {
				auto &attr = attributes[i];
				auto udmAttribute = udmAttributes[i];
				std::vector<std::array<uint16_t, 4>> usedVertexData;
				usedVertexData.reserve(attr.vertexData.size());
				for(auto idx : usedVertIndices)
					usedVertexData.push_back(attr.vertexData[idx]);
				static_assert(sizeof(udm::Half) == sizeof(uint16_t));
				udmAttribute["property"] = attr.name;
				udmAttribute.AddArray("values", strctHalfVector4, usedVertexData, udm::ArrayType::Compressed);
			}
		}
	}
	return true;
}
bool pragma::animation::VertexAnimation::LoadFromAssetData(asset::Model &mdl, const udm::AssetData &data, std::string &outErr)
{
	if(data.GetAssetType() != PMORPHANI_IDENTIFIER) {
		outErr = "Incorrect format!";
		return false;
	}

	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return false;
	}
	// if(version > FORMAT_VERSION)
	// 	return false;

	auto udm = *data;
	udm["name"](m_name);

	auto readFlag = [this](auto udm, auto flag, const std::string &name, auto &outFlags) {
		auto udmFlags = udm["flags"];
		if(!udmFlags)
			return;
		math::set_flag(outFlags, flag, udmFlags[name](false));
	};

	auto &meshAnims = GetMeshAnimations();
	auto udmMeshAnims = udm["meshAnimations"];
	auto numMeshAnims = udmMeshAnims.GetSize();
	meshAnims.resize(numMeshAnims);
	for(auto i = decltype(numMeshAnims) {0u}; i < numMeshAnims; ++i) {
		auto &ma = meshAnims[i];
		ma = pragma::util::make_shared<MeshVertexAnimation>();
		auto udmMa = udmMeshAnims[i];
		auto groupIdx = std::numeric_limits<uint32_t>::max();
		auto meshIdx = std::numeric_limits<uint32_t>::max();
		auto subMeshIdx = std::numeric_limits<uint32_t>::max();
		udmMa["meshGroup"](groupIdx);
		udmMa["mesh"](meshIdx);
		udmMa["subMesh"](subMeshIdx);

		auto *mesh = mdl.GetMesh(groupIdx, meshIdx);
		auto *subMesh = mdl.GetSubMesh(groupIdx, meshIdx, subMeshIdx);
		if(mesh && subMesh)
			ma->SetMesh(*mesh, *subMesh);

		auto &frames = ma->GetFrames();
		auto udmFrames = udmMa["frames"];
		auto numFrames = udmFrames.GetSize();
		frames.resize(numFrames);
		for(auto frameIdx = decltype(numFrames) {0u}; frameIdx < numFrames; ++frameIdx) {
			auto &meshFrame = frames[frameIdx];
			meshFrame = pragma::util::make_shared<MeshVertexFrame>();
			auto udmFrame = udmFrames[frameIdx];
			auto flags = meshFrame->GetFlags();
			readFlag(udmFrame, MeshVertexFrame::Flags::HasDeltaValues, "hasDeltaValues", flags);
			readFlag(udmFrame, MeshVertexFrame::Flags::HasNormals, "hasNormals", flags);
			static_assert(math::to_integral(MeshVertexFrame::Flags::Count) == 2, "Update this list when new flags have been added!");
			meshFrame->SetFlags(flags);
			if(subMesh)
				meshFrame->SetVertexCount(subMesh->GetVertexCount());

			std::vector<uint16_t> usedVertIndices {};
			udmFrame["vertexIndices"](usedVertIndices);

			auto udmAttributes = udmFrame["attributes"];
			for(auto udmAttr : udmAttributes) {
				std::string property;
				udmAttr["property"](property);
				if(property == "position") {
					std::vector<std::array<uint16_t, 4>> positionData;
					udmAttr["values"](positionData);
					if(positionData.size() == usedVertIndices.size()) {
						for(auto i = decltype(usedVertIndices.size()) {0u}; i < usedVertIndices.size(); ++i) {
							auto idx = usedVertIndices[i];
							meshFrame->SetVertexPosition(idx, positionData[i]);
						}
					}
				}
				else if(property == "normal") {
					std::vector<std::array<uint16_t, 4>> normalData;
					udmAttr["values"](normalData);
					if(normalData.size() == usedVertIndices.size()) {
						for(auto i = decltype(usedVertIndices.size()) {0u}; i < usedVertIndices.size(); ++i) {
							auto idx = usedVertIndices[i];
							meshFrame->SetVertexNormal(idx, normalData[i]);
						}
					}
				}
			}
		}
	}
	return true;
}

bool pragma::animation::VertexAnimation::GetMeshAnimationId(geometry::ModelSubMesh &subMesh, uint32_t &id) const
{
	auto it = std::find_if(m_meshAnims.begin(), m_meshAnims.end(), [&subMesh](const std::shared_ptr<MeshVertexAnimation> &anim) { return (anim->GetSubMesh() == &subMesh) ? true : false; });
	if(it == m_meshAnims.end())
		return false;
	id = it - m_meshAnims.begin();
	return true;
}
const pragma::animation::MeshVertexAnimation *pragma::animation::VertexAnimation::GetMeshAnimation(geometry::ModelSubMesh &subMesh) const { return const_cast<VertexAnimation *>(this)->GetMeshAnimation(subMesh); }
pragma::animation::MeshVertexAnimation *pragma::animation::VertexAnimation::GetMeshAnimation(geometry::ModelSubMesh &subMesh)
{
	auto frameId = 0u;
	if(GetMeshAnimationId(subMesh, frameId) == false)
		return nullptr;
	return m_meshAnims.at(frameId).get();
}

std::shared_ptr<pragma::animation::MeshVertexFrame> pragma::animation::VertexAnimation::AddMeshFrame(geometry::ModelMesh &mesh, geometry::ModelSubMesh &subMesh)
{
	auto *anim = GetMeshAnimation(subMesh);
	if(anim == nullptr) {
		auto meshAnim = pragma::util::make_shared<MeshVertexAnimation>();
		meshAnim->SetMesh(mesh, subMesh);
		m_meshAnims.push_back(meshAnim);
		anim = meshAnim.get();
	}
	return anim->AddFrame();
}

const pragma::animation::MeshVertexFrame *pragma::animation::VertexAnimation::GetMeshFrame(geometry::ModelSubMesh &subMesh, uint32_t frameId) const { return const_cast<VertexAnimation *>(this)->GetMeshFrame(subMesh, frameId); }
pragma::animation::MeshVertexFrame *pragma::animation::VertexAnimation::GetMeshFrame(geometry::ModelSubMesh &subMesh, uint32_t frameId)
{
	auto animId = 0u;
	if(GetMeshAnimationId(subMesh, animId) == false)
		return nullptr;
	return m_meshAnims.at(animId)->GetFrame(frameId);
}

std::shared_ptr<pragma::animation::VertexAnimation> pragma::animation::VertexAnimation::Copy() const { return Create(*this); }

void pragma::animation::VertexAnimation::SetName(const std::string &name) { m_name = name; }
const std::string &pragma::animation::VertexAnimation::GetName() const { return m_name; }

const std::vector<std::shared_ptr<pragma::animation::MeshVertexAnimation>> &pragma::animation::VertexAnimation::GetMeshAnimations() const { return const_cast<VertexAnimation *>(this)->GetMeshAnimations(); }
std::vector<std::shared_ptr<pragma::animation::MeshVertexAnimation>> &pragma::animation::VertexAnimation::GetMeshAnimations() { return m_meshAnims; }

void pragma::animation::VertexAnimation::Rotate(const Quat &rot)
{
	for(auto &meshAnim : m_meshAnims)
		meshAnim->Rotate(rot);
}
void pragma::animation::VertexAnimation::Scale(const Vector3 &scale)
{
	for(auto &meshAnim : m_meshAnims)
		meshAnim->Scale(scale);
}
void pragma::animation::VertexAnimation::Mirror(Axis axis)
{
	for(auto &meshAnim : m_meshAnims)
		meshAnim->Mirror(axis);
}
