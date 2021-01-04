/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_LUA_HPP__
#define __C_SHADER_LUA_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"
#include <shader/prosper_shader_base_image_processing.hpp>
#include <pragma/lua/luaobjectbase.h>
#include <wgui/shaders/wishader_textured.hpp>

namespace pragma
{
	struct LuaDescriptorSetBinding;
	struct LuaDescriptorSetInfo;
	struct LuaVertexBinding;
	struct LuaVertexAttribute;
};

lua_registercheck(DescriptorSetBinding,pragma::LuaDescriptorSetBinding);
lua_registercheck(DescriptorSetInfo,pragma::LuaDescriptorSetInfo);
lua_registercheck(VertexBinding,pragma::LuaVertexBinding);
lua_registercheck(VertexAttribute,pragma::LuaVertexAttribute);
namespace pragma
{
	class DLLCLIENT LuaShaderManager
	{
	private:
		struct ShaderInfo
		{
			luabind::object luaClassObject;
			::util::WeakHandle<prosper::Shader> whShader;
		};
		std::unordered_map<std::string,ShaderInfo> m_shaders;
	public:
		~LuaShaderManager();
		void RegisterShader(std::string className,luabind::object &o);
		luabind::object *GetClassObject(std::string className);
	};

	struct LuaVertexBinding
	{
		LuaVertexBinding()=default;
		LuaVertexBinding(uint32_t inputRate,uint32_t stride=std::numeric_limits<uint32_t>::max())
			: stride(stride),inputRate(static_cast<prosper::VertexInputRate>(inputRate))
		{}
		uint32_t stride = 0u;
		prosper::VertexInputRate inputRate = prosper::VertexInputRate::Vertex;
	};

	struct LuaVertexAttribute
	{
		LuaVertexAttribute()=default;
		LuaVertexAttribute(uint32_t format,uint32_t location=std::numeric_limits<uint32_t>::max(),uint32_t offset=std::numeric_limits<uint32_t>::max())
			: location(location),format(static_cast<prosper::Format>(format)),offset(offset)
		{}
		uint32_t location = 0u;
		prosper::Format format = prosper::Format::R8G8B8A8_UNorm;
		uint32_t offset = 0u;
	};

	struct LuaDescriptorSetBinding
	{
		LuaDescriptorSetBinding()=default;
		LuaDescriptorSetBinding(uint32_t type,uint32_t shaderStages,uint32_t bindingIndex=std::numeric_limits<uint32_t>::max(),uint32_t descriptorArraySize=1u)
			: type(static_cast<prosper::DescriptorType>(type)),shaderStages(static_cast<prosper::ShaderStageFlags>(shaderStages)),bindingIndex(bindingIndex),descriptorArraySize(descriptorArraySize)
		{}
		prosper::DescriptorType type = {};
		prosper::ShaderStageFlags shaderStages = prosper::ShaderStageFlags::All;
		uint32_t bindingIndex = std::numeric_limits<uint32_t>::max();
		uint32_t descriptorArraySize = 1u;
	};

	struct LuaDescriptorSetInfo
	{
		LuaDescriptorSetInfo()=default;
		LuaDescriptorSetInfo(luabind::object lbindings,uint32_t setIndex=std::numeric_limits<uint32_t>::max())
			: setIndex(setIndex)
		{
			::Lua::get_table_values<LuaDescriptorSetBinding>(lbindings.interpreter(),2u,bindings,[](lua_State *l,int32_t idx) -> LuaDescriptorSetBinding {
				return *::Lua::CheckDescriptorSetBinding(l,idx);
			});
		}
		uint32_t setIndex = 0u;
		std::vector<LuaDescriptorSetBinding> bindings;
	};
	prosper::DescriptorSetInfo to_prosper_descriptor_set_info(const LuaDescriptorSetInfo &ldescSetInfo);

	//////////////////

	class DLLCLIENT LuaShaderBase
		: public LuaObjectBase
	{
	public:
		static LuaShaderBase *GetShader(prosper::BasePipelineCreateInfo &pipelineInfo);

		LuaShaderBase(prosper::Shader &shader);
		LuaShaderBase(const LuaShaderBase&)=delete;
		LuaShaderBase &operator=(const LuaShaderBase&)=delete;
		void Initialize(const luabind::object &o);
		void ClearLuaObject();
		bool AttachDescriptorSetInfo(const pragma::LuaDescriptorSetInfo &descSetInfo);

		prosper::Shader &GetShader() const;

		virtual void SetIdentifier(const std::string &identifier)=0;
		virtual void SetPipelineCount(uint32_t pipelineCount)=0;
		virtual prosper::ICommandBuffer *GetCurrentCommandBuffer()=0;

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) {}
		static void Lua_default_InitializePipeline(lua_State *l,LuaShaderBase *shader,prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) {shader->Lua_InitializePipeline(pipelineInfo,pipelineIdx);}

		void Lua_OnInitialized() {}
		static void Lua_default_OnInitialized(lua_State *l,LuaShaderBase *shader) {shader->Lua_OnInitialized();}

		void Lua_OnPipelinesInitialized() {}
		static void Lua_default_OnPipelinesInitialized(lua_State *l,LuaShaderBase *shader) {shader->Lua_OnPipelinesInitialized();}

		void Lua_OnPipelineInitialized(uint32_t pipelineIdx) {}
		static void Lua_default_OnPipelineInitialized(lua_State *l,LuaShaderBase *shader,uint32_t pipelineIdx) {shader->Lua_OnPipelineInitialized(pipelineIdx);}
	protected:
		void OnPipelineInitialized(uint32_t pipelineIdx);
		void OnInitialized();
		void OnPipelinesInitialized();
		void InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
		prosper::Shader &m_shader;

		prosper::BasePipelineCreateInfo *m_currentPipelineInfo = nullptr;
	private:
		uint32_t m_currentDescSetIndex = 0u;
	};

	class DLLCLIENT LuaShaderGraphicsBase
		: public LuaShaderBase
	{
	public:
		bool AttachVertexAttribute(const pragma::LuaVertexBinding &binding,const std::vector<pragma::LuaVertexAttribute> &attributes);
	protected:
		LuaShaderGraphicsBase(prosper::ShaderGraphics &shader);
		void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
		void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx);
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)=0;
	private:
		uint32_t m_currentVertexAttributeLocation = 0u;
		uint32_t m_currentVertexBinding = 0u;
	};

	class DLLCLIENT LuaShaderComputeBase
		: public LuaShaderBase
	{
	protected:
		LuaShaderComputeBase(prosper::ShaderCompute &shader);
		void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
	};

	template<class TBaseShader,class TLuaBaseShader>
		class TLuaShaderBase
			: public TBaseShader,public TLuaBaseShader
	{
	public:
		virtual prosper::ICommandBuffer *GetCurrentCommandBuffer() override;
	protected:
		template<typename... TARGS>
			TLuaShaderBase(prosper::IPrContext &context,TARGS ...args)
				: TBaseShader(context,std::forward<TARGS>(args)...),TLuaBaseShader(static_cast<TBaseShader&>(*this))
		{}
		virtual void SetIdentifier(const std::string &identifier) override;
		virtual void SetPipelineCount(uint32_t pipelineCount) override;
		virtual void OnPipelineInitialized(uint32_t pipelineIdx) override;
		virtual void OnInitialized() override;
		virtual void OnPipelinesInitialized() override;
	};

	class DLLCLIENT LuaShaderGraphics
		: public TLuaShaderBase<prosper::ShaderGraphics,LuaShaderGraphicsBase>
	{
	public:
		LuaShaderGraphics();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};

	class DLLCLIENT LuaShaderCompute
		: public TLuaShaderBase<prosper::ShaderCompute,LuaShaderComputeBase>
	{
	public:
		LuaShaderCompute();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	protected:
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};

	class DLLCLIENT LuaShaderGUITextured
		: public TLuaShaderBase<wgui::ShaderTextured,LuaShaderGraphicsBase>
	{
	public:
		LuaShaderGUITextured();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};

	class DLLCLIENT LuaShaderGUIParticle2D
		: public TLuaShaderBase<pragma::ShaderParticle2DBase,LuaShaderGraphicsBase>
	{
	public:
		LuaShaderGUIParticle2D();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;

		Vector3 Lua_CalcVertexPosition(
			lua_State *l,CParticleSystemHandle &hPtC,uint32_t ptIdx,uint32_t localVertIdx,const Vector3 &camPos,const Vector3 &camUpWs,const Vector3 &camRightWs,float nearZ,float farZ
		);
		static Vector3 Lua_default_CalcVertexPosition(
			lua_State *l,LuaShaderGUIParticle2D &shader,CParticleSystemHandle &hPtC,uint32_t ptIdx,uint32_t localVertIdx,
			const Vector3 &camPos,const Vector3 &camUpWs,const Vector3 &camRightWs,float nearZ,float farZ
		) {
			return shader.Lua_CalcVertexPosition(l,hPtC,ptIdx,localVertIdx,camPos,camUpWs,camRightWs,nearZ,farZ);
		}
	protected:
		virtual Vector3 DoCalcVertexPosition(
			const pragma::CParticleSystemComponent &ptc,uint32_t ptIdx,uint32_t localVertIdx,
			const Vector3 &camPos,const Vector3 &camUpWs,const Vector3 &camRightWs,float nearZ,float farZ
		) const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};

	class DLLCLIENT LuaShaderImageProcessing
		: public TLuaShaderBase<prosper::ShaderBaseImageProcessing,LuaShaderGraphicsBase>
	{
	public:
		LuaShaderImageProcessing();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};

	class DLLCLIENT LuaShaderPostProcessing
		: public TLuaShaderBase<ShaderPPBase,LuaShaderGraphicsBase>
	{
	public:
		LuaShaderPostProcessing();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};

	class DLLCLIENT LuaShaderTextured3D
		: public TLuaShaderBase<ShaderTextured3DBase,LuaShaderGraphicsBase>
	{
	public:
		LuaShaderTextured3D();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		// virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override; // TODO: ShaderTexturedBase

		bool Lua_BindMaterialParameters(Material &mat);
		static bool Lua_default_BindMaterialParameters(lua_State *l,LuaShaderTextured3D &shader,Material &mat) {return shader.Lua_BindMaterialParameters(mat);}

		void Lua_InitializeGfxPipelineVertexAttributes(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
		static void Lua_default_InitializeGfxPipelineVertexAttributes(lua_State *l,LuaShaderTextured3D &shader,prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) {shader.Lua_InitializeGfxPipelineVertexAttributes(pipelineInfo,pipelineIdx);}

		void Lua_InitializeGfxPipelinePushConstantRanges(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
		static void Lua_default_InitializeGfxPipelinePushConstantRanges(lua_State *l,LuaShaderTextured3D &shader,prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) {shader.Lua_InitializeGfxPipelinePushConstantRanges(pipelineInfo,pipelineIdx);}

		void Lua_InitializeGfxPipelineDescriptorSets(prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
		static void Lua_default_InitializeGfxPipelineDescriptorSets(lua_State *l,LuaShaderTextured3D &shader,prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) {shader.Lua_InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);}
	
		void Lua_OnBindMaterial(Material &mat);
		static void Lua_default_OnBindMaterial(lua_State *l,LuaShaderTextured3D &shader,Material &mat) {shader.Lua_OnBindMaterial(mat);}

		int32_t Lua_OnDraw(ModelSubMesh &mesh);
		static int32_t Lua_default_OnDraw(lua_State *l,LuaShaderTextured3D &shader,ModelSubMesh &mesh) {return shader.Lua_OnDraw(mesh);}

		void Lua_OnBindEntity(EntityHandle &hEnt);
		static void Lua_default_OnBindEntity(lua_State *l,LuaShaderTextured3D &shader,EntityHandle &hEnt) {shader.Lua_OnBindEntity(hEnt);}

		void Lua_OnBindScene(rendering::RasterizationRenderer &renderer,bool bView);
		static void Lua_default_OnBindScene(lua_State *l,LuaShaderTextured3D &shader,rendering::RasterizationRenderer &renderer,bool bView) {shader.Lua_OnBindScene(renderer,bView);}

		void Lua_OnBeginDraw(prosper::ICommandBuffer &drawCmd,const Vector4 &clipPlane,uint32_t pipelineIdx,uint32_t recordFlags);
		static void Lua_default_OnBeginDraw(lua_State *l,LuaShaderTextured3D &shader,prosper::ICommandBuffer &drawCmd,const Vector4 &clipPlane,uint32_t pipelineIdx,uint32_t recordFlags) {shader.Lua_OnBeginDraw(drawCmd,clipPlane,pipelineIdx,recordFlags);}

		void Lua_OnEndDraw();
		static void Lua_default_OnEndDraw(lua_State *l,LuaShaderTextured3D &shader) {shader.Lua_OnEndDraw();}

		virtual bool BindMaterial(CMaterial &mat) override;
		virtual bool Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount=1) override;
		virtual bool BindEntity(CBaseEntity &ent) override;
		virtual bool BindVertexAnimationOffset(uint32_t offset) override;
		virtual bool BindScene(pragma::CSceneComponent &scene,rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool BeginDraw(
			const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f},ShaderGameWorldPipeline pipelineIdx=ShaderGameWorldPipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		) override;
		virtual void EndDraw() override;
	protected:
		virtual bool BindMaterialParameters(CMaterial &mat) override;
		virtual void InitializeGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};
};
template<class TBaseShader,class TLuaBaseShader>
	prosper::ICommandBuffer *pragma::TLuaShaderBase<TBaseShader,TLuaBaseShader>::GetCurrentCommandBuffer() {return TBaseShader::GetCurrentCommandBuffer();}
template<class TBaseShader,class TLuaBaseShader>
	void pragma::TLuaShaderBase<TBaseShader,TLuaBaseShader>::SetIdentifier(const std::string &identifier) {return TBaseShader::SetIdentifier(identifier);}
template<class TBaseShader,class TLuaBaseShader>
	void pragma::TLuaShaderBase<TBaseShader,TLuaBaseShader>::SetPipelineCount(uint32_t pipelineCount) {return TBaseShader::SetPipelineCount(pipelineCount);}
template<class TBaseShader,class TLuaBaseShader>
	void pragma::TLuaShaderBase<TBaseShader,TLuaBaseShader>::OnInitialized() {TBaseShader::OnInitialized(); TLuaBaseShader::OnInitialized();}
template<class TBaseShader,class TLuaBaseShader>
	void pragma::TLuaShaderBase<TBaseShader,TLuaBaseShader>::OnPipelinesInitialized() {TBaseShader::OnPipelinesInitialized(); TLuaBaseShader::OnPipelinesInitialized();}
template<class TBaseShader,class TLuaBaseShader>
	void pragma::TLuaShaderBase<TBaseShader,TLuaBaseShader>::OnPipelineInitialized(uint32_t pipelineIdx) {TBaseShader::OnPipelineInitialized(pipelineIdx); TLuaBaseShader::OnPipelineInitialized(pipelineIdx);}

#endif