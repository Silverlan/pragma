#ifndef __C_SHADER_PARTICLE_UNLIT_H__
#define __C_SHADER_PARTICLE_UNLIT_H__
// prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_textured.h"

#define SHADER_TEX_UNIT_PARTICLE_SCENE_DEPTH 7

namespace Shader
{
	class DLLCLIENT ParticleBase
		: public Base
	{
	public:
		enum class DLLCLIENT RenderFlags : uint32_t
		{
			None = 0,
			Animated = 1,
			Unlit = Animated<<1,
			BlackToAlpha = Unlit<<1,
			SoftParticles = BlackToAlpha<<1
		};
	protected:
		enum class DLLCLIENT DescSet : uint32_t
		{
			ParticleMap = 0,
			DepthMap = ParticleMap +1,
			Animation = DepthMap +1,
			RenderSettings = Animation +1,
			Camera = RenderSettings,
			Time = Camera +1,
			CSM = Time,
			LightSources = CSM +1,
			TileVisLightIndexBuffer = LightSources,
			ShadowData = LightSources,
			CSMTextureArray = ShadowData +1,
			ShadowMaps = CSMTextureArray +1u,
			ShadowCubeMaps = ShadowMaps
		};
		enum class DLLCLIENT Location : uint32_t
		{
			Vertex = 0,
			Xyzs = 1,
			Color = 2,
			Rotation = 3,
			AnimationStart = 4
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Vertex = 0,
			Xyzs = 1,
			Color = 1,
			Rotation = 1,
			AnimationStart = 2,

			ParticleMap = 0,
			DepthMap = 0,
			Animation = 0,

			Camera = 0,
			RenderSettings = Camera +1,

			Time = 0,
			CSM = 1,
			LightSources = 0,
			TileVisLightIndexBuffer = LightSources +1,
			ShadowData = TileVisLightIndexBuffer +1,
			CSMTextureArray = 0,
			ShadowMaps = 0u,
			ShadowCubeMaps = ShadowMaps +umath::to_integral(GameLimits::MaxActiveShadowMaps)
		};
	public:
		static const uint32_t MAX_PARTICLE_COUNT;
	protected:
		ParticleBase(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		bool m_bEnableLighting = false;
		bool m_bUseSceneDepthTexture = true;
		bool m_bUseParticleTexture = true;
		Vulkan::SwapDescriptorBuffer m_descBufferViewCam = nullptr;
		RenderFlags GetRenderFlags(CParticleSystem *particle);
		void GetParticleSystemInfo(Camera *cam,Mat4 &matrix,CParticleSystem *particle,Material *mat,Vector3 &up,Vector3 &right,float &nearZ,float &farZ) const;
		void InitializeVertexDataDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions);
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
		virtual bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline) override;
		virtual void Draw(Camera &cam,CParticleSystem *particle,Bool bloom,const std::function<void(const Vulkan::Context&,const Vulkan::CommandBuffer&)> &fCallback);
		void BindScene(uint32_t descSet,Vulkan::CommandBufferObject *cmdBuffer,const Scene &scene,bool bView);
	public:
		using Base::BeginDraw;
		virtual void InitializeMaterial(Material *mat,bool bReload=false) override;
		virtual uint32_t GetDescSet(DescSet set) const;
		void GenerateDepthMapDescriptorSet(Vulkan::DescriptorSet &texture);
		void GenerateParticleMapDescriptorSet(Vulkan::DescriptorSet &texture);
		void GenerateAnimationDescriptorSet(Vulkan::DescriptorSet &anim);
		void BindScene(Vulkan::CommandBufferObject *cmdBuffer,const Scene &scene,bool bView=false);
		void BindScene(const Scene &scene,bool bView=false);
		void BindLights(const Vulkan::DescriptorSetObject *shadowMapDescSet,const Vulkan::DescriptorSetObject *lightSourcesDescSet);
		void Draw(Camera &cam,CParticleSystem *particle,Bool bloom);
	};
};
REGISTER_BASIC_ARITHMETIC_OPERATORS(Shader::ParticleBase::RenderFlags);
#endif
#endif
