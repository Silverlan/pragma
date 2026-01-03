// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :entities.components.vertex_animated;
import :entities.components.flex;
import :entities.components.render;

import :engine;
import :model.model_class;

using namespace pragma;

static std::shared_ptr<prosper::IDynamicResizableBuffer> g_vertexAnimationBuffer = nullptr;
const std::shared_ptr<prosper::IDynamicResizableBuffer> &pragma::get_vertex_animation_buffer() { return g_vertexAnimationBuffer; }
void pragma::initialize_vertex_animation_buffer()
{
	auto alignment = get_cengine()->GetRenderContext().CalcBufferAlignment(prosper::BufferUsageFlags::StorageBufferBit);
	if(alignment > 0 && (sizeof(CVertexAnimatedComponent::VertexAnimationData) % alignment) != 0) {
		Con::CWAR << "Minimum storage buffer alignment is " << alignment << ", but only alignment values of <=" << sizeof(CVertexAnimatedComponent::VertexAnimationData) << " are supported! Morph target animations will be disabled!" << Con::endl;
		return;
	}
	auto instanceSize = sizeof(CVertexAnimatedComponent::VertexAnimationData);
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::HostAccessable | prosper::MemoryFeatureFlags::HostCoherent;
	createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	createInfo.size = 1 * 1'024 * 1'024; // 1 MiB
	createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit | prosper::BufferUsageFlags::TransferDstBit;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
#endif

	g_vertexAnimationBuffer = get_cengine()->GetRenderContext().CreateDynamicResizableBuffer(createInfo, createInfo.size * 5 /* 5 MiB */, 0.05f);
	g_vertexAnimationBuffer->SetDebugName("entity_vertex_anim_bone_buf");
	g_vertexAnimationBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit | prosper::IBuffer::MapFlags::Unsynchronized);
	assert(g_vertexAnimationBuffer->GetAlignment() == 0 || (sizeof(CVertexAnimatedComponent::VertexAnimationData) % g_vertexAnimationBuffer->GetAlignment()) == 0);
}
void pragma::clear_vertex_animation_buffer() { g_vertexAnimationBuffer = nullptr; }
CVertexAnimatedComponent::~CVertexAnimatedComponent() { DestroyVertexAnimationBuffer(); }
void CVertexAnimatedComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CVertexAnimatedComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(cModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		DestroyVertexAnimationBuffer();
		if(static_cast<CEOnModelChanged &>(evData.get()).model == nullptr)
			return;
		auto renderC = GetEntity().GetComponent<CRenderComponent>();
		if(renderC.valid() && renderC->IsRenderBufferValid())
			InitializeVertexAnimationBuffer();
	});
	BindEventUnhandled(cRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED, [this](std::reference_wrapper<ComponentEvent> evData) { InitializeVertexAnimationBuffer(); });
	BindEventUnhandled(cRenderComponent::EVENT_ON_UPDATE_RENDER_DATA_MT, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateVertexAnimationDataMT(); });
	BindEventUnhandled(cRenderComponent::EVENT_ON_UPDATE_RENDER_BUFFERS, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateVertexAnimationBuffer(static_cast<CEOnUpdateRenderBuffers &>(evData.get()).commandBuffer); });
	BindEvent(cRenderComponent::EVENT_UPDATE_INSTANTIABILITY, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		// TODO: Allow instantiability for vertex animated entities
		static_cast<CEUpdateInstantiability &>(evData.get()).instantiable = false;
		return util::EventReply::Handled;
	});
	// TODO: We shouldn't need the animated component, but morph target animations appear broken if it's not there.
	// Find out why and then remove this line!
	GetEntity().AddComponent<CAnimatedComponent>();

	auto whRenderComponent = GetEntity().GetComponent<CRenderComponent>();
	if(whRenderComponent.valid() && whRenderComponent->GetRenderBuffer()) {
		InitializeVertexAnimationBuffer();
		whRenderComponent->UpdateInstantiability();
	}
}
void CVertexAnimatedComponent::InitializeVertexAnimationBuffer()
{
	auto &globalVertAnimBuffer = get_vertex_animation_buffer();
	if(!globalVertAnimBuffer)
		return;
	auto &ent = GetEntity();
	auto whRenderComponent = ent.GetComponent<CRenderComponent>();
	auto &mdl = GetEntity().GetModel();
	auto *buf = whRenderComponent.valid() ? whRenderComponent->GetRenderBuffer() : nullptr;
	auto *ds = whRenderComponent.valid() ? whRenderComponent->GetRenderDescriptorSet() : nullptr;
	if(!buf || mdl == nullptr || !ds)
		return;
	if(m_vertexAnimationBuffer == nullptr) {
		auto &vertAnimations = mdl->GetVertexAnimations();
		m_maxVertexAnimations = 0u;
		for(auto &va : vertAnimations)
			m_maxVertexAnimations += va->GetMeshAnimations().size();
		if(m_maxVertexAnimations == 0u)
			return;
		m_vertexAnimationBuffer = globalVertAnimBuffer->AllocateBuffer(m_maxVertexAnimations * sizeof(VertexAnimationData));
		if(!m_vertexAnimationBuffer) {
			m_maxVertexAnimations = 0;
			return;
		}
		m_vertexAnimationBufferData.resize(m_maxVertexAnimations);
	}

	auto &vertAnimBuffer = static_cast<asset::CModel &>(*mdl).GetVertexAnimationBuffer();
	ds->SetBindingStorageBuffer(*m_vertexAnimationBuffer, math::to_integral(ShaderGameWorldLightingPass::InstanceBinding::VertexAnimationFrameData));
	ds->SetBindingStorageBuffer(*vertAnimBuffer, math::to_integral(ShaderGameWorldLightingPass::InstanceBinding::VertexAnimations));
	ds->Update();
}

void CVertexAnimatedComponent::DestroyVertexAnimationBuffer()
{
	m_vertexAnimationBuffer = nullptr;
	m_maxVertexAnimations = 0u;
	m_activeVertexAnimations = 0u;
	m_vertexAnimationMeshBufferOffsets.clear();

	auto &ent = GetEntity();
	auto whRenderComponent = ent.GetComponent<CRenderComponent>();
	auto *pRenderDescSet = whRenderComponent.valid() ? whRenderComponent->GetRenderDescriptorSet() : nullptr;
	if(pRenderDescSet) {
		pRenderDescSet->SetBindingStorageBuffer(*get_cengine()->GetRenderContext().GetDummyBuffer(), math::to_integral(ShaderGameWorldLightingPass::InstanceBinding::VertexAnimationFrameData)); // Reset buffer
		pRenderDescSet->Update();
	}
}

const std::shared_ptr<prosper::IBuffer> &CVertexAnimatedComponent::GetVertexAnimationBuffer() const { return m_vertexAnimationBuffer; }

void CVertexAnimatedComponent::UpdateVertexAnimationDataMT()
{
	auto mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC == nullptr || mdlC->GetLOD() > 0)
		return;
	auto &mdl = mdlC->GetModel();
	if(mdl == nullptr)
		return;
	m_activeVertexAnimations = 0u;
	auto &ent = GetEntity();
	if(m_vertexAnimationBuffer == nullptr)
		return;
	// Vertex animation data sorted by mesh
	auto &data = m_vertexAnimationData;
	data.clear();
	m_vertexAnimationSlots.clear();
	auto idx = 0u;
	auto &vertAnims = mdl->GetVertexAnimations();

	auto whFlexComponent = ent.GetComponent<CFlexComponent>();
	if(whFlexComponent.expired())
		return;
	// TODO: This should be in the CFlexComponent component code, not here!
	whFlexComponent->UpdateFlexWeightsMT();
	auto &flexWeights = whFlexComponent->GetFlexWeights();
	auto &flexes = mdl->GetFlexes();
	assert(flexes.size() == flexWeights.size());
	auto numFlexes = math::min(flexes.size(), flexWeights.size());
	for(auto flexId = decltype(numFlexes) {0u}; flexId < numFlexes; ++flexId) {
		auto flexWeight = flexWeights.at(flexId);
		if(flexWeight == 0.f)
			continue;

		auto &flex = flexes.at(flexId);
		auto *va = flex.GetVertexAnimation();
		if(va == nullptr)
			continue;
		auto frameId = flex.GetFrameIndex();
		//auto *ma = flex.GetMeshVertexAnimation();
		//auto *fr = flex.GetMeshVertexFrame();
		auto it = std::find_if(vertAnims.begin(), vertAnims.end(), [va](const std::shared_ptr<animation::VertexAnimation> &vaOther) { return vaOther.get() == va; });
		if(it == vertAnims.end())
			continue;
		auto vaId = it - vertAnims.begin();
		auto &meshAnims = va->GetMeshAnimations();
		for(auto &meshAnim : meshAnims) {
			auto *subMesh = meshAnim->GetSubMesh();
			if(subMesh == nullptr)
				continue;
			auto &frames = meshAnim->GetFrames();
			if(frames.empty() == true)
				continue;
			auto *fr = meshAnim->GetFrame(frameId);
			if(fr == nullptr)
				continue;
			auto cycle = flexWeight * (frames.size() - 1);
			auto fraction = fmodf(cycle, 1.f);
			auto nextFrameId = math::min(static_cast<uint32_t>(frameId + 1), static_cast<uint32_t>(frames.size() - 1));

			uint64_t srcFrameOffset = 0ull;
			uint64_t dstFrameOffset = 0ull;
			if(static_cast<asset::CModel &>(*mdl).GetVertexAnimationBufferFrameOffset(vaId, static_cast<geometry::CModelSubMesh &>(*subMesh), frameId, srcFrameOffset) == false
			  || static_cast<asset::CModel &>(*mdl).GetVertexAnimationBufferFrameOffset(vaId, static_cast<geometry::CModelSubMesh &>(*subMesh), nextFrameId, dstFrameOffset) == false)
				continue;
			if(srcFrameOffset > std::numeric_limits<uint32_t>::max() || dstFrameOffset > std::numeric_limits<uint32_t>::max())
				continue;
			++m_activeVertexAnimations;

			auto it = data.find(static_cast<geometry::CModelSubMesh *>(subMesh));
			if(it == data.end())
				it = data.insert(std::make_pair(static_cast<geometry::CModelSubMesh *>(subMesh), std::vector<VertexAnimationData> {})).first;
			it->second.push_back({});
			auto &vaData = it->second.back();
			vaData.srcFrameOffset = srcFrameOffset;
			vaData.dstFrameOffset = dstFrameOffset;
			//static auto defFraction = 1.f; // TODO
			vaData.blend = flexWeight; //defFraction;//fraction;

			//
			m_vertexAnimationSlots.push_back({});
			auto &info = m_vertexAnimationSlots.back();
			info.vertexAnimationId = vaId;
			info.frameId = frameId;
			info.nextFrameId = nextFrameId;
			info.blend = flexWeight;
			info.mesh = subMesh->shared_from_this();
			//

			if(m_activeVertexAnimations >= m_maxVertexAnimations)
				goto endLoop; // TODO: This should never be reached, but in some cases it is. FIXME
		}
	}
endLoop:
	m_vertexAnimationMeshBufferOffsets = {};
	m_bufferUpdateRequired = true;

	auto bufferOffset = 0ull;
	for(auto &pair : data) {
		m_vertexAnimationMeshBufferOffsets.insert(std::make_pair(pair.first, std::make_pair(bufferOffset, pair.second.size())));
		memcpy(reinterpret_cast<uint8_t *>(m_vertexAnimationBufferData.data()) + bufferOffset * sizeof(pair.second.front()), pair.second.data(), pair.second.size() * sizeof(pair.second.front()));
		bufferOffset += pair.second.size();
	}
	m_vertexAnimationBufferDataCount = bufferOffset;
	//auto bufferData = std::vector<pragma::rendering::VertexAnimationData>{};
	//bufferData.reserve(m_activeVertexAnimations);
}

void CVertexAnimatedComponent::UpdateVertexAnimationBuffer(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	if(!m_vertexAnimationBuffer)
		return;
	auto &buf = *m_vertexAnimationBuffer;
	if(m_vertexAnimationBufferDataCount > 0 && m_bufferUpdateRequired) {
		constexpr auto pipelineStages = prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit | prosper::PipelineStageFlags::GeometryShaderBit;
		drawCmd->RecordBufferBarrier(buf, pipelineStages, prosper::PipelineStageFlags::TransferBit, prosper::AccessFlags::ShaderReadBit, prosper::AccessFlags::TransferWriteBit);
		drawCmd->RecordUpdateBuffer(buf, 0ull, m_vertexAnimationBufferDataCount * sizeof(m_vertexAnimationBufferData.front()), m_vertexAnimationBufferData.data());
		drawCmd->RecordBufferBarrier(buf, prosper::PipelineStageFlags::TransferBit, pipelineStages, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
	}
}

bool CVertexAnimatedComponent::GetVertexAnimationBufferMeshOffset(geometry::CModelSubMesh &mesh, uint32_t &offset, uint32_t &animCount) const
{
	auto it = m_vertexAnimationMeshBufferOffsets.find(&mesh);
	if(it == m_vertexAnimationMeshBufferOffsets.end())
		return false;
	offset = it->second.first;
	animCount = it->second.second;
	return true;
}
bool CVertexAnimatedComponent::GetLocalVertexPosition(const geometry::ModelSubMesh &subMesh, uint32_t vertexId, Vector3 &pos, Vector3 *optOutNormal, float *optOutDelta) const
{
	pos = {};
	if(optOutNormal)
		*optOutNormal = {};
	if(optOutDelta)
		*optOutDelta = 0.f;

	auto mdlComponent = GetEntity().GetModelComponent();
	auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return false;
	auto &vaAnims = mdl->GetVertexAnimations();
	for(auto &animSlot : m_vertexAnimationSlots) {
		if(animSlot.mesh.expired() || animSlot.mesh.lock().get() != &subMesh || animSlot.vertexAnimationId >= vaAnims.size())
			continue;
		auto &vaAnim = vaAnims.at(animSlot.vertexAnimationId);
		auto *ma = vaAnim->GetMeshAnimation(const_cast<geometry::ModelSubMesh &>(subMesh));
		if(ma == nullptr)
			continue;
		auto *frame = ma->GetFrame(animSlot.frameId);
		auto *nextFrame = ma->GetFrame(animSlot.nextFrameId);
		if(frame == nullptr || nextFrame == nullptr)
			continue;
		Vector3 vaPos;
		if(frame->GetVertexPosition(vertexId, vaPos) == false)
			continue;
		pos += vaPos * animSlot.blend;
		if(optOutNormal) {
			Vector3 n;
			if(frame->GetVertexNormal(vertexId, n) == false)
				continue;
			*optOutNormal += n * animSlot.blend;
		}
		if(optOutDelta) {
			float dtVal;
			if(frame->GetDeltaValue(vertexId, dtVal))
				*optOutDelta += dtVal * animSlot.blend;
		}
	}
	return true;
}

void CVertexAnimatedComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	auto defCVertexAnimated = pragma::LuaCore::create_entity_component_class<CVertexAnimatedComponent, BaseEntityComponent>("VertexAnimatedComponent");
	defCVertexAnimated.def("UpdateVertexAnimationBuffer", static_cast<void (*)(lua::State *, CVertexAnimatedComponent &, std::shared_ptr<prosper::ICommandBuffer> &)>([](lua::State *l, CVertexAnimatedComponent &hAnim, std::shared_ptr<prosper::ICommandBuffer> &drawCmd) {
		if(drawCmd->IsPrimary() == false)
			return;
		hAnim.UpdateVertexAnimationBuffer(std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(drawCmd));
	}));
	defCVertexAnimated.def("GetVertexAnimationBuffer", &CVertexAnimatedComponent::GetVertexAnimationBuffer);
	defCVertexAnimated.def("GetVertexAnimationBufferMeshOffset",
	  static_cast<Lua::opt<luabind::mult<uint32_t, uint32_t>> (*)(lua::State *, CVertexAnimatedComponent &, std::shared_ptr<geometry::ModelSubMesh> &)>(
	    [](lua::State *l, CVertexAnimatedComponent &hAnim, std::shared_ptr<geometry::ModelSubMesh> &subMesh) -> Lua::opt<luabind::mult<uint32_t, uint32_t>> {
		    uint32_t offset;
		    uint32_t animCount;
		    auto b = hAnim.GetVertexAnimationBufferMeshOffset(static_cast<geometry::CModelSubMesh &>(*subMesh), offset, animCount);
		    if(b == false)
			    return luabind::object {};
		    return luabind::mult<uint32_t, uint32_t> {l, offset, animCount};
	    }));
	defCVertexAnimated.def("GetLocalVertexPosition",
	  static_cast<Lua::opt<luabind::mult<Vector3, Vector3>> (*)(lua::State *, CVertexAnimatedComponent &, std::shared_ptr<geometry::ModelSubMesh> &, uint32_t)>(
	    [](lua::State *l, CVertexAnimatedComponent &hAnim, std::shared_ptr<geometry::ModelSubMesh> &subMesh, uint32_t vertexId) -> Lua::opt<luabind::mult<Vector3, Vector3>> {
		    Vector3 pos, n;
		    auto b = hAnim.GetLocalVertexPosition(static_cast<geometry::CModelSubMesh &>(*subMesh), vertexId, pos, &n);
		    if(b == false)
			    return luabind::object {};
		    return luabind::mult<Vector3, Vector3> {l, pos, n};
	    }));
	modEnts[defCVertexAnimated];
}
