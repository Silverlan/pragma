#include "stdafx_client.h"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_flex_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <pragma/model/animation/vertex_animation.hpp>
#include <alsound_buffer.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

luabind::object CVertexAnimatedComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CVertexAnimatedComponentHandleWrapper>(l);}
void CVertexAnimatedComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<ComponentEvent> evData) {
		DestroyVertexAnimationBuffer();
	});
	BindEventUnhandled(CRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED,[this](std::reference_wrapper<ComponentEvent> evData) {
		InitializeVertexAnimationBuffer();
	});
	auto whRenderComponent = GetEntity().GetComponent<CRenderComponent>();
	if(whRenderComponent.valid() && whRenderComponent->GetRenderBuffer().expired() == false)
		InitializeVertexAnimationBuffer();
}
void CVertexAnimatedComponent::InitializeVertexAnimationBuffer()
{
	auto &ent = GetEntity();
	auto whMdlComponent = ent.GetModelComponent();
	auto whRenderComponent = ent.GetComponent<CRenderComponent>();
	auto mdl = whMdlComponent.valid() ? whMdlComponent->GetModel() : nullptr;
	auto wpRenderBuffer = whRenderComponent.valid() ? whRenderComponent->GetRenderBuffer() : std::weak_ptr<prosper::Buffer>{};
	auto *pRenderDescSet = whRenderComponent.valid() ? whRenderComponent->GetRenderDescriptorSet() : nullptr;
	if(wpRenderBuffer.expired() == true || m_vertexAnimationBuffer != nullptr || mdl == nullptr || pRenderDescSet == nullptr)
		return;
	auto &vertAnimations = mdl->GetVertexAnimations();
	m_maxVertexAnimations = 0u;
	for(auto &va : vertAnimations)
		m_maxVertexAnimations += va->GetMeshAnimations().size();
	if(m_maxVertexAnimations == 0u)
		return;
	m_maxVertexAnimations = 500u;
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
	createInfo.size = m_maxVertexAnimations *sizeof(VertexAnimationData);
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
	m_vertexAnimationBuffer = prosper::util::create_buffer(c_engine->GetDevice(),createInfo);

	auto &vertAnimBuffer = static_cast<CModel&>(*mdl).GetVertexAnimationBuffer();
	prosper::util::set_descriptor_set_binding_storage_buffer(*pRenderDescSet,*m_vertexAnimationBuffer,umath::to_integral(pragma::ShaderTextured3DBase::InstanceBinding::VertexAnimationFrameData));
	prosper::util::set_descriptor_set_binding_storage_buffer(*pRenderDescSet,*vertAnimBuffer,umath::to_integral(pragma::ShaderTextured3DBase::InstanceBinding::VertexAnimations));
}

void CVertexAnimatedComponent::DestroyVertexAnimationBuffer()
{
	m_vertexAnimationBuffer = nullptr;
	m_maxVertexAnimations = 0u;
	m_activeVertexAnimations = 0u;
	m_vertexAnimationMeshBufferOffsets.clear();
}

const std::shared_ptr<prosper::Buffer> &CVertexAnimatedComponent::GetVertexAnimationBuffer() const {return m_vertexAnimationBuffer;}

void CVertexAnimatedComponent::UpdateVertexAnimationBuffer(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	m_activeVertexAnimations = 0u;
	auto &ent = GetEntity();
	auto whMdlComponent = ent.GetModelComponent();
	auto mdl = whMdlComponent.valid() ? whMdlComponent->GetModel() : nullptr;
	if(mdl == nullptr || m_vertexAnimationBuffer == nullptr)
		return;
	auto &buf = *m_vertexAnimationBuffer;
	// Vertex animation data sorted by mesh
	auto &data = m_vertexAnimationData;
	data.clear();
	m_vertexAnimationSlots.clear();
	auto idx = 0u;
	auto &vertAnims = mdl->GetVertexAnimations();

	/// Test
	//auto flexIdTest = 0u;
	//mdl->GetFlexId("OpenLowerLipL",flexIdTest);
	auto whFlexComponent = ent.GetComponent<CFlexComponent>();
	if(whFlexComponent.expired())
		return;
	for(auto &flex : mdl->GetFlexes())
	{
		auto flexId = idx++;
		//if(flexId == 0u)//flexId != 19u) // TODO; Flex id 19 = "AU1R"
		//	continue;
		auto *va = flex.GetVertexAnimation();
		auto *ma = flex.GetMeshVertexAnimation();
		auto *fr = flex.GetMeshVertexFrame();
		if(va == nullptr || ma == nullptr || fr == nullptr)
			continue;
		auto it = std::find_if(vertAnims.begin(),vertAnims.end(),[va](const std::shared_ptr<VertexAnimation> &vaOther) {
			return vaOther.get() == va;
		});
		auto &frames = ma->GetFrames();
		auto itFrame = std::find_if(frames.begin(),frames.end(),[fr](const std::shared_ptr<MeshVertexFrame> &frameOther) {
			return frameOther.get() == fr;
		});
		if(it == vertAnims.end() || itFrame == frames.end())
			continue;
		auto vaId = it -vertAnims.begin();
		auto frameId = itFrame -frames.begin();

		auto flexVal = 0.f;
		// TODO: This doesn't really belong here and should go into flex component implementation instead!
		if(whFlexComponent->CalcFlexValue(flexId,flexVal) == false)// || flexVal == 0.f)
			continue;
		//if(flexId != flexIdTest)
		//	continue;
		//flexVal = 1.f;
		//if(umath::abs(flexVal) > 0.01f)
		//	Con::cout<<flex.GetName()<<": "<<flexVal<<Con::endl;

		auto &meshAnims = va->GetMeshAnimations();
		for(auto &meshAnim : meshAnims)
		{
			auto *subMesh = meshAnim->GetSubMesh();
			if(subMesh == nullptr)
				continue;
			auto &frames = meshAnim->GetFrames();
			if(frames.empty() == true)
				continue;
			auto cycle = flexVal *(frames.size() -1);
			auto fraction = fmodf(cycle,1.f);
			//auto frameId = umath::min(static_cast<uint32_t>(umath::floor(cycle)),static_cast<uint32_t>(frames.size() -1));
			auto nextFrameId = umath::min(static_cast<uint32_t>(frameId +1),static_cast<uint32_t>(frames.size() -1));

			auto srcFrameOffset = 0ull;
			auto dstFrameOffset = 0ull;
			if(
				static_cast<CModel&>(*mdl).GetVertexAnimationBufferFrameOffset(vaId,static_cast<CModelSubMesh&>(*subMesh),frameId,srcFrameOffset) == false ||
				static_cast<CModel&>(*mdl).GetVertexAnimationBufferFrameOffset(vaId,static_cast<CModelSubMesh&>(*subMesh),nextFrameId,dstFrameOffset) == false
			) continue;
			if(srcFrameOffset > std::numeric_limits<uint32_t>::max() || dstFrameOffset > std::numeric_limits<uint32_t>::max())
				continue;
			++m_activeVertexAnimations;

			auto it = data.find(static_cast<CModelSubMesh*>(subMesh));
			if(it == data.end())
				it = data.insert(std::make_pair(static_cast<CModelSubMesh*>(subMesh),std::vector<VertexAnimationData>{})).first;
			it->second.push_back({});
			auto &vaData = it->second.back();
			vaData.srcFrameOffset = srcFrameOffset;
			vaData.dstFrameOffset = dstFrameOffset;
			//static auto defFraction = 1.f; // TODO
			vaData.blend = flexVal;//defFraction;//fraction;

			//
			m_vertexAnimationSlots.push_back({});
			auto &info = m_vertexAnimationSlots.back();
			info.vertexAnimationId = vaId;
			info.frameId = frameId;
			info.nextFrameId = nextFrameId;
			info.blend = flexVal;
			info.mesh = subMesh->shared_from_this();
			//

			if(m_activeVertexAnimations >= m_maxVertexAnimations)
				goto endLoop;
		}
	}
endLoop:
	m_vertexAnimationMeshBufferOffsets = {};
	//auto bufferData = std::vector<VertexAnimationData>{};
	//bufferData.reserve(m_activeVertexAnimations);

	auto bufferOffset = 0ull;
	for(auto &pair : data)
	{
		m_vertexAnimationMeshBufferOffsets.insert(std::make_pair(pair.first,std::make_pair(bufferOffset,pair.second.size())));
		c_engine->ScheduleRecordUpdateBuffer(buf.shared_from_this(),bufferOffset *sizeof(pair.second.front()),pair.second.size() *sizeof(pair.second.front()),pair.second.data());
		bufferOffset += pair.second.size();

		//for(auto &vaData : pair.second)
		//	bufferData.push_back(vaData);
	}
}

bool CVertexAnimatedComponent::GetVertexAnimationBufferMeshOffset(CModelSubMesh &mesh,uint32_t &offset,uint32_t &animCount) const
{
	auto it = m_vertexAnimationMeshBufferOffsets.find(&mesh);
	if(it == m_vertexAnimationMeshBufferOffsets.end())
		return false;
	offset = it->second.first;
	animCount = it->second.second;
	return true;
}
bool CVertexAnimatedComponent::GetLocalVertexPosition(const ModelSubMesh &subMesh,uint32_t vertexId,Vector3 &pos) const
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return false;
	auto &vaAnims = mdl->GetVertexAnimations();
	for(auto &animSlot : m_vertexAnimationSlots)
	{
		if(animSlot.mesh.expired() || animSlot.mesh.lock().get() != &subMesh || animSlot.vertexAnimationId >= vaAnims.size())
			continue;
		auto &vaAnim = vaAnims.at(animSlot.vertexAnimationId);
		auto *ma = vaAnim->GetMeshAnimation(const_cast<ModelSubMesh&>(subMesh));
		if(ma == nullptr)
			continue;
		auto *frame = ma->GetFrame(animSlot.frameId);
		auto *nextFrame = ma->GetFrame(animSlot.nextFrameId);
		if(frame == nullptr || nextFrame == nullptr)
			continue;
		auto vaPos = Vector3{};
		if(frame->GetVertexPosition(vertexId,vaPos) == false)
			continue;
		pos += vaPos *animSlot.blend;
	}
	return true;
}
