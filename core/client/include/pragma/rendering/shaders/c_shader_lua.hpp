/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_LUA_HPP__
#define __C_SHADER_LUA_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"
#include <shader/prosper_shader_base_image_processing.hpp>
#include <prosper_prepared_command_buffer.hpp>
#include <pragma/lua/luaobjectbase.h>
#include <wgui/shaders/wishader_textured.hpp>

namespace pragma {
	struct LuaDescriptorSetBinding;
	struct LuaDescriptorSetInfo;
	struct LuaVertexBinding;
	struct LuaVertexAttribute;
};

lua_registercheck(DescriptorSetBinding, pragma::LuaDescriptorSetBinding);
lua_registercheck(DescriptorSetInfo, pragma::LuaDescriptorSetInfo);
lua_registercheck(VertexBinding, pragma::LuaVertexBinding);
lua_registercheck(VertexAttribute, pragma::LuaVertexAttribute);
namespace pragma {
	class DLLCLIENT LuaShaderManager {
	  private:
		struct ShaderInfo {
			luabind::object luaClassObject;
			::util::WeakHandle<prosper::Shader> whShader;
		};
		std::unordered_map<std::string, ShaderInfo> m_shaders;
	  public:
		~LuaShaderManager();
		void RegisterShader(std::string className, luabind::object &o);
		luabind::object *GetClassObject(std::string className);
	};

	struct LuaVertexBinding {
		LuaVertexBinding() = default;
		LuaVertexBinding(uint32_t inputRate, uint32_t stride = std::numeric_limits<uint32_t>::max()) : stride(stride), inputRate(static_cast<prosper::VertexInputRate>(inputRate)) {}
		uint32_t stride = 0u;
		prosper::VertexInputRate inputRate = prosper::VertexInputRate::Vertex;
	};

	struct LuaVertexAttribute {
		LuaVertexAttribute() = default;
		LuaVertexAttribute(uint32_t format, uint32_t location = std::numeric_limits<uint32_t>::max(), uint32_t offset = std::numeric_limits<uint32_t>::max()) : location(location), format(static_cast<prosper::Format>(format)), offset(offset) {}
		uint32_t location = 0u;
		prosper::Format format = prosper::Format::R8G8B8A8_UNorm;
		uint32_t offset = 0u;
	};

	struct LuaDescriptorSetBinding {
		LuaDescriptorSetBinding() = default;
		LuaDescriptorSetBinding(const std::string &name, uint32_t type, uint32_t shaderStages, uint32_t bindingIndex = std::numeric_limits<uint32_t>::max(), uint32_t descriptorArraySize = 1u)
		    : name {name}, type(static_cast<prosper::DescriptorType>(type)), shaderStages(static_cast<prosper::ShaderStageFlags>(shaderStages)), bindingIndex(bindingIndex), descriptorArraySize(descriptorArraySize)
		{
		}
		LuaDescriptorSetBinding(uint32_t type, uint32_t shaderStages, uint32_t bindingIndex = std::numeric_limits<uint32_t>::max(), uint32_t descriptorArraySize = 1u) : LuaDescriptorSetBinding {"", type, shaderStages, bindingIndex, descriptorArraySize} {}
		prosper::DescriptorType type = {};
		std::string name;
		prosper::ShaderStageFlags shaderStages = prosper::ShaderStageFlags::All;
		uint32_t bindingIndex = std::numeric_limits<uint32_t>::max();
		uint32_t descriptorArraySize = 1u;
	};

	struct LuaDescriptorSetInfo {
		LuaDescriptorSetInfo() = default;
		LuaDescriptorSetInfo(uint32_t tableIndex, const std::string &name, luabind::object lbindings, uint32_t setIndex = std::numeric_limits<uint32_t>::max());
		LuaDescriptorSetInfo(const std::string &name, luabind::object lbindings, uint32_t setIndex = std::numeric_limits<uint32_t>::max());
		LuaDescriptorSetInfo(luabind::object lbindings, uint32_t setIndex = std::numeric_limits<uint32_t>::max());
		uint32_t setIndex = 0u;
		std::string name;
		std::vector<LuaDescriptorSetBinding> bindings;
	};
	prosper::DescriptorSetInfo to_prosper_descriptor_set_info(const LuaDescriptorSetInfo &ldescSetInfo);

	//////////////////

	struct LShaderBase;
	class DLLCLIENT LuaShaderWrapperBase : public LuaObjectBase {
	  public:
		static LuaShaderWrapperBase *GetShader(prosper::BasePipelineCreateInfo &pipelineInfo);

		LuaShaderWrapperBase();
		LuaShaderWrapperBase(const LuaShaderWrapperBase &) = delete;
		LuaShaderWrapperBase &operator=(const LuaShaderWrapperBase &) = delete;
		virtual void Initialize(const luabind::object &o);
		void OnInitializationComplete();
		void ClearLuaObject();

		void SetStageSourceFilePath(prosper::ShaderStage shaderStage, const std::string &fpath);
		void SetPipelineCount(uint32_t pipelineCount);

		prosper::Shader &GetShader() const;
		void SetShader(prosper::Shader *shader);
		virtual LShaderBase *CreateShader() const = 0;

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) {}
		static void Lua_default_InitializePipeline(lua_State *l, LuaShaderWrapperBase *shader, prosper::BasePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { shader->Lua_InitializePipeline(pipelineInfo, pipelineIdx); }

		virtual void Lua_InitializeShaderResources() {}
		static void Lua_default_InitializeShaderResources(lua_State *l, LuaShaderWrapperBase *shader) { shader->Lua_InitializeShaderResources(); }

		void Lua_OnInitialized() {}
		static void Lua_default_OnInitialized(lua_State *l, LuaShaderWrapperBase *shader) { shader->Lua_OnInitialized(); }

		void Lua_OnPipelinesInitialized() {}
		static void Lua_default_OnPipelinesInitialized(lua_State *l, LuaShaderWrapperBase *shader) { shader->Lua_OnPipelinesInitialized(); }

		void Lua_OnPipelineInitialized(uint32_t pipelineIdx) {}
		static void Lua_default_OnPipelineInitialized(lua_State *l, LuaShaderWrapperBase *shader, uint32_t pipelineIdx) { shader->Lua_OnPipelineInitialized(pipelineIdx); }

		// For internal use only!
		uint32_t GetCurrentPipelineIndex() const { return m_curPipelineIdx; }
		void OnPipelineInitialized(uint32_t pipelineIdx);
		void OnInitialized();
		void OnPipelinesInitialized();
	  protected:
		void InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx);
		void InitializeShaderResources();
		prosper::Shader *m_shader = nullptr;
		uint32_t m_curPipelineIdx = std::numeric_limits<uint32_t>::max();

		prosper::BasePipelineCreateInfo *m_currentPipelineInfo = nullptr;
	  private:
		uint32_t m_currentDescSetIndex = 0u;
	};

	DLLCLIENT void reset_lua_shaders();
	struct DLLCLIENT LShaderBase {
		virtual ~LShaderBase() = default;
		void SetLuaShader(luabind::object wrapperObject, LuaShaderWrapperBase *wrapper)
		{
			m_wrapper = wrapper;
			m_wrapperObject = wrapperObject;
		}
		LuaShaderWrapperBase *GetWrapper() { return m_wrapper; }
		const luabind::object &GetWrapperObject() { return m_wrapperObject; }
		virtual void SetIdentifier(const std::string &identifier) = 0;
		virtual void SetPipelineCount(uint32_t count) = 0;
	  protected:
		luabind::object m_wrapperObject;
		LuaShaderWrapperBase *m_wrapper = nullptr;
	};

	template<class TShader>
	luabind::object create_shader_object(lua_State *l, LShaderBase &shader)
	{
		Lua::PushRaw<TShader *>(l, static_cast<TShader *>(&shader));
		auto o = luabind::object {luabind::from_stack(l, -1)};
		Lua::Pop(l, 1);
		return o;
	}

	struct ShaderGraphicsWrapper;
	struct ShaderGuiWrapper;
	struct ShaderGuiTexturedWrapper;
	class DLLCLIENT LuaShaderWrapperGraphicsBase : public LuaShaderWrapperBase {
	  public:
		bool AttachVertexAttribute(const pragma::LuaVertexBinding &binding, const std::vector<pragma::LuaVertexAttribute> &attributes);
	  protected:
		friend ShaderGraphicsWrapper;
		friend ShaderGuiWrapper;
		friend ShaderGuiTexturedWrapper;
		LuaShaderWrapperGraphicsBase();
		void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx);
		void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx);
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) = 0;
	  private:
		struct VertexBindingData {
			prosper::ShaderGraphics::VertexBinding binding;
			std::vector<prosper::ShaderGraphics::VertexAttribute> attributes;
		};
		std::vector<std::unique_ptr<VertexBindingData>> m_vertexBindingData;
	};

	struct ShaderComputeWrapper;
	class DLLCLIENT LuaShaderWrapperComputeBase : public LuaShaderWrapperBase {
	  protected:
		LuaShaderWrapperComputeBase();
		friend ShaderComputeWrapper;
		void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx);
	};

	template<typename TShader>
	class TLShaderBase : public LShaderBase, public TShader {
	  public:
		using TBaseShader = prosper::ShaderGraphics;
		using TShader::TShader;
	  protected:
		virtual void OnInitialized() override
		{
			TShader::OnInitialized();
			auto *wrapper = GetWrapper();
			if(!wrapper)
				return;
			wrapper->OnInitialized();
		}
		virtual void OnPipelinesInitialized() override
		{
			TShader::OnPipelinesInitialized();
			auto *wrapper = GetWrapper();
			if(!wrapper)
				return;
			wrapper->OnPipelinesInitialized();
			wrapper->OnInitializationComplete();
		}
		virtual void OnPipelineInitialized(uint32_t pipelineIdx) override
		{
			TShader::OnPipelineInitialized(pipelineIdx);
			auto *wrapper = GetWrapper();
			if(!wrapper)
				return;
			wrapper->OnPipelineInitialized(pipelineIdx);
		}
	};

	class LuaShaderWrapperGraphics;
	struct DLLCLIENT LShaderGraphics : public TLShaderBase<prosper::ShaderGraphics> {
	  public:
		friend LuaShaderWrapperGraphics;
		LShaderGraphics();

		virtual void SetIdentifier(const std::string &identifier) override { prosper::ShaderGraphics::SetIdentifier(identifier); }
		virtual void SetPipelineCount(uint32_t count) override { prosper::ShaderGraphics::SetPipelineCount(count); }
	  protected:
		void BaseInitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { TBaseShader::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
		void BaseInitializeShaderResources() { TBaseShader::InitializeShaderResources(); }
		void BaseInitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) { TBaseShader::InitializeRenderPass(outRenderPass, pipelineIdx); }

		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx);
	};
	class DLLCLIENT LuaShaderWrapperGraphics : public LuaShaderWrapperGraphicsBase {
	  public:
		using TShader = LShaderGraphics;
		friend TShader;
		LuaShaderWrapperGraphics();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void Lua_InitializeShaderResources() override;
		virtual LShaderBase *CreateShader() const override { return new TShader {}; }
	  protected:
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};

	class LuaShaderWrapperCompute;
	struct DLLCLIENT LShaderCompute : public TLShaderBase<prosper::ShaderCompute> {
	  public:
		using TBaseShader = prosper::ShaderCompute;
		LShaderCompute();

		virtual void SetIdentifier(const std::string &identifier) override { prosper::ShaderCompute::SetIdentifier(identifier); }
		virtual void SetPipelineCount(uint32_t count) override { prosper::ShaderCompute::SetPipelineCount(count); }
	  protected:
		friend LuaShaderWrapperCompute;
		void BaseInitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { TBaseShader::InitializeComputePipeline(pipelineInfo, pipelineIdx); }
		void BaseInitializeShaderResources() { TBaseShader::InitializeShaderResources(); }
		virtual void InitializeComputePipeline(prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
	};
	class DLLCLIENT LuaShaderWrapperCompute : public LuaShaderWrapperComputeBase {
	  public:
		using TShader = LShaderCompute;
		friend TShader;
		LuaShaderWrapperCompute();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void Lua_InitializeShaderResources() override;
		virtual LShaderBase *CreateShader() const override { return new TShader {}; }
	};

	class LuaShaderWrapperGUI;
	struct DLLCLIENT LShaderGui : public TLShaderBase<wgui::Shader> {
	  public:
		using TBaseShader = wgui::Shader;
		LShaderGui();

		virtual void SetIdentifier(const std::string &identifier) override { wgui::Shader::SetIdentifier(identifier); }
		virtual void SetPipelineCount(uint32_t count) override { wgui::Shader::SetPipelineCount(count); }

		bool RecordBeginDraw(prosper::ShaderBindState &bindState, wgui::DrawState &drawState, uint32_t width, uint32_t height, wgui::StencilPipeline pipelineIdx, bool msaa, uint32_t testStencilLevel) const;
	  protected:
		friend LuaShaderWrapperGUI;
		void BaseInitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { TBaseShader::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
		void BaseInitializeShaderResources() { TBaseShader::InitializeShaderResources(); }
		void BaseInitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) { TBaseShader::InitializeRenderPass(outRenderPass, pipelineIdx); }
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx);
	};
	class DLLCLIENT LuaShaderWrapperGUI : public LuaShaderWrapperGraphicsBase {
	  public:
		using TShader = LShaderGui;
		friend TShader;
		LuaShaderWrapperGUI();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void Lua_InitializeShaderResources() override;
		virtual LShaderBase *CreateShader() const override { return new TShader {}; }
	  protected:
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};

	class LuaShaderWrapperGUITextured;
	struct DLLCLIENT LShaderGuiTextured : public TLShaderBase<wgui::ShaderTextured> {
	  public:
		using TBaseShader = wgui::ShaderTextured;
		LShaderGuiTextured();

		virtual void SetIdentifier(const std::string &identifier) override { wgui::ShaderTextured::SetIdentifier(identifier); }
		virtual void SetPipelineCount(uint32_t count) override { wgui::ShaderTextured::SetPipelineCount(count); }
	  protected:
		friend LuaShaderWrapperGUITextured;
		void BaseInitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { TBaseShader::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
		void BaseInitializeShaderResources() { TBaseShader::InitializeShaderResources(); }
		void BaseInitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) { TBaseShader::InitializeRenderPass(outRenderPass, pipelineIdx); }
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx);
	};
	class DLLCLIENT LuaShaderWrapperGUITextured : public LuaShaderWrapperGraphicsBase {
	  public:
		using TShader = LShaderGuiTextured;
		friend TShader;
		LuaShaderWrapperGUITextured();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void Lua_InitializeShaderResources() override;
		virtual LShaderBase *CreateShader() const override { return new TShader {}; }
	  protected:
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};

	class LuaShaderWrapperParticle2D;
	struct DLLCLIENT LShaderParticle2D : public TLShaderBase<pragma::ShaderParticle2DBase> {
	  public:
		using TBaseShader = pragma::ShaderParticle2DBase;
		LShaderParticle2D();

		virtual void SetIdentifier(const std::string &identifier) override { pragma::ShaderParticle2DBase::SetIdentifier(identifier); }
		virtual void SetPipelineCount(uint32_t count) override { pragma::ShaderParticle2DBase::SetPipelineCount(count); }
	  protected:
		friend LuaShaderWrapperParticle2D;
		void BaseInitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { TBaseShader::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
		void BaseInitializeShaderResources() { TBaseShader::InitializeShaderResources(); }
		void BaseInitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) { TBaseShader::InitializeRenderPass(outRenderPass, pipelineIdx); }
		virtual Vector3 DoCalcVertexPosition(const pragma::CParticleSystemComponent &ptc, uint32_t ptIdx, uint32_t localVertIdx, const Vector3 &camPos, const Vector3 &camUpWs, const Vector3 &camRightWs, float nearZ, float farZ) const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx);
	};
	class DLLCLIENT LuaShaderWrapperParticle2D : public LuaShaderWrapperGraphicsBase {
	  public:
		using TShader = LShaderParticle2D;
		friend TShader;
		LuaShaderWrapperParticle2D();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void Lua_InitializeShaderResources() override;

		Vector3 Lua_CalcVertexPosition(lua_State *l, pragma::CParticleSystemComponent &hPtC, uint32_t ptIdx, uint32_t localVertIdx, const Vector3 &camPos, const Vector3 &camUpWs, const Vector3 &camRightWs, float nearZ, float farZ);
		static Vector3 Lua_default_CalcVertexPosition(lua_State *l, LuaShaderWrapperParticle2D &shader, pragma::CParticleSystemComponent &hPtC, uint32_t ptIdx, uint32_t localVertIdx, const Vector3 &camPos, const Vector3 &camUpWs, const Vector3 &camRightWs, float nearZ, float farZ)
		{
			return shader.Lua_CalcVertexPosition(l, hPtC, ptIdx, localVertIdx, camPos, camUpWs, camRightWs, nearZ, farZ);
		}
		virtual LShaderBase *CreateShader() const override { return new TShader {}; }
	  protected:
		Vector3 DoCalcVertexPosition(const pragma::CParticleSystemComponent &ptc, uint32_t ptIdx, uint32_t localVertIdx, const Vector3 &camPos, const Vector3 &camUpWs, const Vector3 &camRightWs, float nearZ, float farZ) const;
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};

	class LuaShaderWrapperImageProcessing;
	struct DLLCLIENT LShaderImageProcessing : public TLShaderBase<prosper::ShaderBaseImageProcessing> {
	  public:
		using TBaseShader = prosper::ShaderBaseImageProcessing;
		LShaderImageProcessing();

		virtual void SetIdentifier(const std::string &identifier) override { prosper::ShaderBaseImageProcessing::SetIdentifier(identifier); }
		virtual void SetPipelineCount(uint32_t count) override { prosper::ShaderBaseImageProcessing::SetPipelineCount(count); }
	  protected:
		friend LuaShaderWrapperImageProcessing;
		void BaseInitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { TBaseShader::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
		void BaseInitializeShaderResources() { TBaseShader::InitializeShaderResources(); }
		void BaseInitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) { TBaseShader::InitializeRenderPass(outRenderPass, pipelineIdx); }
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx);
	};
	class DLLCLIENT LuaShaderWrapperImageProcessing : public LuaShaderWrapperGraphicsBase {
	  public:
		using TShader = LShaderImageProcessing;
		friend TShader;
		LuaShaderWrapperImageProcessing();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void Lua_InitializeShaderResources() override;
		virtual LShaderBase *CreateShader() const override { return new TShader {}; }
	  protected:
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};

	class LuaShaderWrapperPostProcessing;
	struct DLLCLIENT LShaderPostProcessing : public TLShaderBase<ShaderPPBase> {
	  public:
		using TBaseShader = ShaderPPBase;
		LShaderPostProcessing();

		virtual void SetIdentifier(const std::string &identifier) override { ShaderPPBase::SetIdentifier(identifier); }
		virtual void SetPipelineCount(uint32_t count) override { ShaderPPBase::SetPipelineCount(count); }
	  protected:
		friend LuaShaderWrapperPostProcessing;
		void BaseInitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { TBaseShader::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
		void BaseInitializeShaderResources() { TBaseShader::InitializeShaderResources(); }
		void BaseInitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) { TBaseShader::InitializeRenderPass(outRenderPass, pipelineIdx); }
		void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx);
	};
	class DLLCLIENT LuaShaderWrapperPostProcessing : public LuaShaderWrapperGraphicsBase {
	  public:
		using TShader = LShaderPostProcessing;
		friend TShader;
		LuaShaderWrapperPostProcessing();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void Lua_InitializeShaderResources() override;
		virtual LShaderBase *CreateShader() const override { return new TShader {}; }
	  protected:
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};

	class LuaShaderWrapperTextured3D;
	struct DLLCLIENT LShaderGameWorldLightingPass : public TLShaderBase<ShaderGameWorldLightingPass> {
	  public:
		using TBaseShader = ShaderGameWorldLightingPass;
		LShaderGameWorldLightingPass();
		virtual ~LShaderGameWorldLightingPass() override;

		virtual void SetIdentifier(const std::string &identifier) override { ShaderGameWorldLightingPass::SetIdentifier(identifier); }
		virtual void SetPipelineCount(uint32_t count) override { ShaderGameWorldLightingPass::SetPipelineCount(count); }

		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual void InitializeMaterialData(const CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, pragma::rendering::shader_material::ShaderMaterialData &inOutMatData) override;
		void SetPushConstants(DataStream dsPushConstants);

		void ResetPcb();
		prosper::util::PreparedCommandBuffer &GetBindPcb() { return m_bindPcb; }
	  protected:
		friend LuaShaderWrapperTextured3D;
		std::shared_ptr<prosper::IDescriptorSetGroup> BaseInitializeMaterialDescriptorSet(CMaterial &mat);
		void BaseInitializeMaterialData(CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, pragma::rendering::shader_material::ShaderMaterialData &inOutMatData);
		void BaseInitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx);
		void BaseInitializeShaderResources();
		void BaseInitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx);
		void BaseInitializeGfxPipelineVertexAttributes();
		void BaseInitializeGfxPipelinePushConstantRanges();
		void BaseInitializeGfxPipelineDescriptorSets();

		virtual void InitializeGfxPipelineVertexAttributes() override;
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx);

		DataStream m_pushConstants;

		prosper::util::PreparedCommandBuffer m_bindPcb;
		mutable prosper::util::PreparedCommandBufferUserData m_bindUserData;
		prosper::util::PreparedCommandArgumentMap m_bindArgs;
	};
	class DLLCLIENT LuaShaderWrapperTextured3D : public LuaShaderWrapperGraphicsBase {
	  public:
		using TShader = LShaderGameWorldLightingPass;
		friend TShader;
		LuaShaderWrapperTextured3D();
		virtual ~LuaShaderWrapperTextured3D() override;
		virtual void Initialize(const luabind::object &o) override;

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void Lua_InitializeShaderResources() override;
		// virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override; // TODO: ShaderTexturedBase

		std::shared_ptr<prosper::IDescriptorSetGroup> Lua_InitializeMaterialDescriptorSet(Material &mat);
		static std::shared_ptr<prosper::IDescriptorSetGroup> Lua_default_InitializeMaterialDescriptorSet(lua_State *l, LuaShaderWrapperTextured3D &shader, Material &mat) { return shader.Lua_InitializeMaterialDescriptorSet(mat); }

		void Lua_InitializeMaterialData(CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, pragma::rendering::shader_material::ShaderMaterialData &inOutMatData);
		static void Lua_default_InitializeMaterialData(lua_State *l, LuaShaderWrapperTextured3D &shader, CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, pragma::rendering::shader_material::ShaderMaterialData &inOutMatData)
		{
			shader.Lua_InitializeMaterialData(mat, shaderMat, inOutMatData);
		}

		void Lua_InitializeGfxPipelineVertexAttributes();
		static void Lua_default_InitializeGfxPipelineVertexAttributes(lua_State *l, LuaShaderWrapperTextured3D &shader) { shader.Lua_InitializeGfxPipelineVertexAttributes(); }

		void Lua_InitializeGfxPipelinePushConstantRanges();
		static void Lua_default_InitializeGfxPipelinePushConstantRanges(lua_State *l, LuaShaderWrapperTextured3D &shader) { shader.Lua_InitializeGfxPipelinePushConstantRanges(); }

		void Lua_InitializeGfxPipelineDescriptorSets();
		static void Lua_default_InitializeGfxPipelineDescriptorSets(lua_State *l, LuaShaderWrapperTextured3D &shader) { shader.Lua_InitializeGfxPipelineDescriptorSets(); }

		void Lua_OnBindMaterial(Material &mat);
		static void Lua_default_OnBindMaterial(lua_State *l, LuaShaderWrapperTextured3D &shader, Material &mat) { shader.Lua_OnBindMaterial(mat); }

		int32_t Lua_OnDraw(ModelSubMesh &mesh);
		static int32_t Lua_default_OnDraw(lua_State *l, LuaShaderWrapperTextured3D &shader, ModelSubMesh &mesh) { return shader.Lua_OnDraw(mesh); }

		void Lua_OnBindEntity(EntityHandle &hEnt);
		static void Lua_default_OnBindEntity(lua_State *l, LuaShaderWrapperTextured3D &shader, EntityHandle &hEnt) { shader.Lua_OnBindEntity(hEnt); }

		void Lua_OnBindScene(CRasterizationRendererComponent &renderer, bool bView);
		static void Lua_default_OnBindScene(lua_State *l, LuaShaderWrapperTextured3D &shader, CRasterizationRendererComponent &renderer, bool bView) { shader.Lua_OnBindScene(renderer, bView); }

		void Lua_OnBeginDraw(prosper::ICommandBuffer &drawCmd, const Vector4 &clipPlane, uint32_t pipelineIdx, uint32_t recordFlags);
		static void Lua_default_OnBeginDraw(lua_State *l, LuaShaderWrapperTextured3D &shader, prosper::ICommandBuffer &drawCmd, const Vector4 &clipPlane, uint32_t pipelineIdx, uint32_t recordFlags) { shader.Lua_OnBeginDraw(drawCmd, clipPlane, pipelineIdx, recordFlags); }

		void Lua_OnEndDraw();
		static void Lua_default_OnEndDraw(lua_State *l, LuaShaderWrapperTextured3D &shader) { shader.Lua_OnEndDraw(); }

		void SetPushConstants(DataStream dsPushConstants);
		prosper::util::PreparedCommandBuffer &GetBindPcb();
		void InitializeMaterialBuffer(prosper::IDescriptorSetGroup &descSet, CMaterial &mat, const pragma::rendering::shader_material::ShaderMaterialData &matData);

		virtual LShaderBase *CreateShader() const override { return new TShader {}; }
	  protected:
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat);
		void InitializeMaterialData(CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, pragma::rendering::shader_material::ShaderMaterialData &inOutMatData);
		void InitializeGfxPipelineVertexAttributes();
		void InitializeGfxPipelinePushConstantRanges();
		void InitializeGfxPipelineDescriptorSets();
	};

	class LuaShaderWrapperPbr;
	struct DLLCLIENT LShaderPbr : public TLShaderBase<ShaderPBR> {
	  public:
		using TBaseShader = ShaderPBR;
		LShaderPbr();

		virtual void SetIdentifier(const std::string &identifier) override { ShaderPBR::SetIdentifier(identifier); }
		virtual void SetPipelineCount(uint32_t count) override { ShaderPBR::SetPipelineCount(count); }
	  protected:
		friend LuaShaderWrapperPbr;
		void BaseInitializeGfxPipelineVertexAttributes() { TBaseShader::InitializeGfxPipelineVertexAttributes(); }
		void BaseInitializeGfxPipelinePushConstantRanges() { TBaseShader::InitializeGfxPipelinePushConstantRanges(); }
		void BaseInitializeGfxPipelineDescriptorSets() { TBaseShader::InitializeGfxPipelineDescriptorSets(); }
		void BaseInitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { TBaseShader::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
		void BaseInitializeShaderResources() { TBaseShader::InitializeShaderResources(); }
		void BaseInitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) { TBaseShader::InitializeRenderPass(outRenderPass, pipelineIdx); }
		virtual void InitializeGfxPipelineVertexAttributes() override;
		virtual void InitializeGfxPipelinePushConstantRanges() override;
		virtual void InitializeGfxPipelineDescriptorSets() override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
		void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx);
	};
	class DLLCLIENT LuaShaderWrapperPbr : public LuaShaderWrapperGraphicsBase {
	  public:
		using TShader = LShaderPbr;
		friend TShader;
		LuaShaderWrapperPbr();

		virtual void Lua_InitializePipeline(prosper::BasePipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void Lua_InitializeShaderResources() override;
		// virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override; // TODO: ShaderTexturedBase

		void Lua_InitializeGfxPipelineVertexAttributes();
		static void Lua_default_InitializeGfxPipelineVertexAttributes(lua_State *l, LuaShaderWrapperPbr &shader) { shader.Lua_InitializeGfxPipelineVertexAttributes(); }

		void Lua_InitializeGfxPipelinePushConstantRanges();
		static void Lua_default_InitializeGfxPipelinePushConstantRanges(lua_State *l, LuaShaderWrapperPbr &shader) { shader.Lua_InitializeGfxPipelinePushConstantRanges(); }

		void Lua_InitializeGfxPipelineDescriptorSets();
		static void Lua_default_InitializeGfxPipelineDescriptorSets(lua_State *l, LuaShaderWrapperPbr &shader) { shader.Lua_InitializeGfxPipelineDescriptorSets(); }

		void Lua_OnBindMaterial(Material &mat);
		static void Lua_default_OnBindMaterial(lua_State *l, LuaShaderWrapperPbr &shader, Material &mat) { shader.Lua_OnBindMaterial(mat); }

		int32_t Lua_OnDraw(ModelSubMesh &mesh);
		static int32_t Lua_default_OnDraw(lua_State *l, LuaShaderWrapperPbr &shader, ModelSubMesh &mesh) { return shader.Lua_OnDraw(mesh); }

		void Lua_OnBindEntity(EntityHandle &hEnt);
		static void Lua_default_OnBindEntity(lua_State *l, LuaShaderWrapperPbr &shader, EntityHandle &hEnt) { shader.Lua_OnBindEntity(hEnt); }

		void Lua_OnBindScene(CRasterizationRendererComponent &renderer, bool bView);
		static void Lua_default_OnBindScene(lua_State *l, LuaShaderWrapperPbr &shader, CRasterizationRendererComponent &renderer, bool bView) { shader.Lua_OnBindScene(renderer, bView); }

		void Lua_OnBeginDraw(prosper::ICommandBuffer &drawCmd, const Vector4 &clipPlane, uint32_t pipelineIdx, uint32_t recordFlags);
		static void Lua_default_OnBeginDraw(lua_State *l, LuaShaderWrapperPbr &shader, prosper::ICommandBuffer &drawCmd, const Vector4 &clipPlane, uint32_t pipelineIdx, uint32_t recordFlags) { shader.Lua_OnBeginDraw(drawCmd, clipPlane, pipelineIdx, recordFlags); }

		void Lua_OnEndDraw();
		static void Lua_default_OnEndDraw(lua_State *l, LuaShaderWrapperPbr &shader) { shader.Lua_OnEndDraw(); }

		virtual LShaderBase *CreateShader() const override { return new TShader {}; }
	  protected:
		void InitializeGfxPipelineVertexAttributes();
		void InitializeGfxPipelinePushConstantRanges();
		void InitializeGfxPipelineDescriptorSets();
		virtual void InitializeDefaultRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};
};

#endif
