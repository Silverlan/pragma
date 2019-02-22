#ifndef __C_SHADER_PARTICLE_BLOB_HPP__
#define __C_SHADER_PARTICLE_BLOB_HPP__

#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"

namespace pragma
{
	class DLLCLIENT ShaderParticleBlob
		: public ShaderParticle2DBase
	{
	public:
		static const uint32_t MAX_BLOB_NEIGHBORS = 8;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BLOB_NEIGHBORS;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BLOB_NEIGHBORS;

		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_PARTICLE_DATA;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_CUBEMAP;

#pragma pack(push,1)
		struct PushConstants
		{
			Vector4 specularColor;
			float reflectionIntensity;
			float refractionIndexRatio;
			uint32_t debugMode;
		};
#pragma pack(pop)

		ShaderParticleBlob(prosper::Context &context,const std::string &identifier);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_textured.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_base.h"

namespace Shader
{
	class DLLCLIENT ParticleBlob
		: public ParticleBase
	{
	public:
		enum class DLLCLIENT DebugMode : uint32_t
		{
			None = 0,
			EyeDir,
			SurfaceNormals,
			FlatColor
		};
	public:
		enum class DLLCLIENT Location : std::underlying_type_t<ParticleBase::Location>
		{
			AdjacentBlobs = umath::to_integral(ParticleBase::Location::AnimationStart) +1
		};
		enum class DLLCLIENT DescSet : std::underlying_type_t<ParticleBase::DescSet>
		{
			ParticleData = umath::to_integral(ParticleBase::DescSet::ShadowCubeMaps) +1,
			CubeMap = ParticleData +1
		};
		enum class DLLCLIENT Binding : std::underlying_type_t<ParticleBase::Binding>
		{
			ParticleData = 0,

			AdjacentBlobs = umath::to_integral(ParticleBase::Binding::AnimationStart) +1
		};
		static const uint32_t MAX_BLOB_NEIGHBORS = 8;
	protected:
		ParticleBlob(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		using ParticleBase::BeginDraw;
		using ParticleBase::Draw;
	public:
		ParticleBlob();
		uint32_t GetDescSet(DescSet set) const;
		bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSet &descSetParticles,vk::DeviceSize offset,const Vector4 &specularColor,float reflectionIntensity=0.f,float refractionIndexRatio=1.f,DebugMode debugMode=DebugMode::None);
		void Draw(Camera &cam,CParticleSystem *particle,Bool bloom,const Vulkan::Buffer &adjacentBlobBuffer);
	};
};
#endif
#endif