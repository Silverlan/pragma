/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __RENDER_PROCESSOR_HPP__
#define __RENDER_PROCESSOR_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/render_queue.hpp"
#include <mathutil/uvec.h>

class CMaterial;
class CBaseEntity;
class ModelSubMesh;
class CModelSubMesh;
struct RenderPassStats;
class RenderDebugInfo;
namespace util {struct RenderPassDrawInfo;};
namespace pragma
{
	class ShaderGameWorld;
	class CRenderComponent;
	class ShaderTextured3DBase;
	enum class ShaderGameWorldPipeline : uint32_t;
};
enum class DLLCLIENT RenderMode : uint32_t;
using MaterialIndex = uint32_t;
using EntityIndex = uint32_t;
enum class RenderFlags : uint8_t;
namespace prosper {class Shader;};
namespace pragma::rendering
{
	class DLLCLIENT ShaderProcessor
	{
	public:
		ShaderProcessor(prosper::ICommandBuffer &cmdBuffer)
			: m_cmdBuffer{cmdBuffer}
		{}
		bool RecordBindShader(const pragma::CSceneComponent &scene,const pragma::rendering::RasterizationRenderer &renderer,bool view,pragma::ShaderGameWorld &shader,uint32_t pipelineIdx=0u);
		bool RecordBindEntity(CBaseEntity &ent);
		bool RecordBindMaterial(CMaterial &mat);
		bool RecordDraw(CModelSubMesh &mesh,pragma::RenderMeshIndex meshIdx,const pragma::rendering::RenderQueue::InstanceSet *instanceSet=nullptr);

		void SetStats(RenderPassStats *stats) {m_stats = stats;}
	private:
		bool RecordBindScene(const pragma::CSceneComponent &scene,const pragma::rendering::RasterizationRenderer &renderer,const pragma::ShaderGameWorld &referenceShader,bool view);
		bool BindInstanceSet(pragma::ShaderGameWorld &shaderScene,const pragma::rendering::RenderQueue::InstanceSet *instanceSet=nullptr);
		void UpdateSceneFlags(ShaderGameWorld::SceneFlags sceneFlags);

		prosper::ICommandBuffer &m_cmdBuffer;
		std::unique_ptr<prosper::IShaderPipelineLayout> m_currentPipelineLayout = nullptr;
		std::optional<Vector4> m_clipPlane {};
		pragma::CVertexAnimatedComponent *m_vertexAnimC = nullptr;
		pragma::CModelComponent *m_modelC = nullptr;
		pragma::CLightMapReceiverComponent *m_lightMapReceiverC = nullptr;
		ShaderGameWorld::SceneFlags m_sceneFlags = ShaderGameWorld::SceneFlags::None;
		bool m_view = false;

		uint32_t m_materialDescriptorSetIndex = std::numeric_limits<uint32_t>::max();
		uint32_t m_entityInstanceDescriptorSetIndex = std::numeric_limits<uint32_t>::max();
		const RenderQueue::InstanceSet *m_curInstanceSet = nullptr;

		pragma::ShaderGameWorld *m_curShader = nullptr;
		bool m_prepassShader = false;
		uint32_t m_curVertexAnimationOffset = 0;
		RenderPassStats *m_stats = nullptr;

		float m_alphaCutoff = 0.5f;
	};

	class RasterizationRenderer;
	class DLLCLIENT BaseRenderProcessor
	{
	public:
		enum class StateFlags : uint8_t
		{
			None = 0u,
			ShaderBound = 1u,
			MaterialBound = ShaderBound<<1u,
			EntityBound = MaterialBound<<1u,

			CountNonOpaqueMaterialsOnly = EntityBound<<1u
		};
		enum class CameraType : uint8_t
		{
			World = 0,
			View
		};
		BaseRenderProcessor(const util::RenderPassDrawInfo &drawSceneInfo,RenderFlags flags,const Vector4 &drawOrigin);
		~BaseRenderProcessor();
		void SetCameraType(CameraType camType);
		void Set3DSky(bool enabled);
		void SetDrawOrigin(const Vector4 &drawOrigin);
		bool BindShader(prosper::Shader &shader);
		bool BindMaterial(CMaterial &mat);
		bool BindEntity(CBaseEntity &ent);
		void SetDepthBias(float d,float delta);
		bool Render(CModelSubMesh &mesh,pragma::RenderMeshIndex meshIdx,const RenderQueue::InstanceSet *instanceSet=nullptr);
		pragma::ShaderGameWorld *GetCurrentShader();
		void UnbindShader();
		void SetCountNonOpaqueMaterialsOnly(bool b);
		prosper::Extent2D GetExtents() const;
		void RecordViewport();
	protected:
		uint32_t Render(const pragma::rendering::RenderQueue &renderQueue,bool bindShaders,RenderPassStats *optStats=nullptr,std::optional<uint32_t> worldRenderQueueIndex={});
		bool BindInstanceSet(pragma::ShaderGameWorld &shaderScene,const RenderQueue::InstanceSet *instanceSet=nullptr);
		void UnbindMaterial();
		void UnbindEntity();
		prosper::ShaderIndex m_curShaderIndex = std::numeric_limits<prosper::ShaderIndex>::max();
		MaterialIndex m_curMaterialIndex = std::numeric_limits<MaterialIndex>::max();
		EntityIndex m_curEntityIndex = std::numeric_limits<EntityIndex>::max();

		ShaderProcessor m_shaderProcessor;

		prosper::Shader *m_curShader = nullptr;
		pragma::ShaderGameWorld *m_shaderScene = nullptr;
		CMaterial *m_curMaterial = nullptr;
		CBaseEntity *m_curEntity = nullptr;
		pragma::CRenderComponent *m_curRenderC = nullptr;
		std::vector<std::shared_ptr<ModelSubMesh>> *m_curEntityMeshList = nullptr;
		const RenderQueue::InstanceSet *m_curInstanceSet = nullptr;

		CameraType m_camType = CameraType::World;
		const util::RenderPassDrawInfo &m_drawSceneInfo;
		Vector4 m_drawOrigin;
		std::optional<Vector2> m_depthBias {};
		pragma::ShaderGameWorldPipeline m_pipelineType;
		RenderPassStats *m_stats = nullptr;
		const pragma::rendering::RasterizationRenderer *m_renderer = nullptr;
		RenderFlags m_renderFlags;
		uint32_t m_numShaderInvocations = 0;
		StateFlags m_stateFlags = StateFlags::None;
	};

	class DLLCLIENT DepthStageRenderProcessor
		: public pragma::rendering::BaseRenderProcessor
	{
	public:
		DepthStageRenderProcessor(const util::RenderPassDrawInfo &drawSceneInfo,RenderFlags flags,const Vector4 &drawOrigin);
		uint32_t Render(const pragma::rendering::RenderQueue &renderQueue,RenderPassStats *optStats=nullptr,std::optional<uint32_t> worldRenderQueueIndex={});
	};

	class DLLCLIENT LightingStageRenderProcessor
		: public pragma::rendering::BaseRenderProcessor
	{
	public:
		using pragma::rendering::BaseRenderProcessor::BaseRenderProcessor;
		uint32_t Render(const pragma::rendering::RenderQueue &renderQueue,RenderPassStats *optStats=nullptr,std::optional<uint32_t> worldRenderQueueIndex={});
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::BaseRenderProcessor::StateFlags);

#endif
