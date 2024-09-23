/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_PARTICLE_BLOB_HPP__
#define __C_SHADER_PARTICLE_BLOB_HPP__

#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"

namespace pragma {
	namespace rendering::shader_material {
		struct ShaderMaterial;
	};
	class DLLCLIENT ShaderParticleBlob : public ShaderParticle2DBase {
	  public:
		enum class DebugMode : uint8_t { None = 0, EyeDir, SurfaceNormal, FlatColor };

		static const uint32_t MAX_BLOB_NEIGHBORS = 8;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BLOB_NEIGHBORS;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BLOB_NEIGHBORS;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDERER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SHADOWS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_PBR;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_PARTICLE_DATA;

		ShaderParticleBlob(prosper::IPrContext &context, const std::string &identifier);
		virtual uint32_t GetSceneDescriptorSetIndex() const override;
		bool RecordDraw(prosper::ShaderBindState &bindState, pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const CParticleSystemComponent &ps, CParticleSystemComponent::OrientationType orientationType, ParticleRenderFlags renderFlags,
		  prosper::IBuffer &blobIndexBuffer, prosper::IDescriptorSet &dsParticles, uint32_t particleBufferOffset);
		virtual bool RecordBindScene(prosper::ICommandBuffer &cmd, const prosper::IShaderPipelineLayout &layout, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer,
		  prosper::IDescriptorSet &dsRenderSettings, prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows) const override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
	  protected:
		virtual void GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual bool RecordParticleMaterial(prosper::ShaderBindState &bindState, const CRasterizationRendererComponent &renderer, const CParticleSystemComponent &ps) const override;
		std::shared_ptr<rendering::shader_material::ShaderMaterial> m_shaderMaterial;
		std::unique_ptr<prosper::DescriptorSetInfo> m_materialDescSetInfo;
	};
};

#endif
