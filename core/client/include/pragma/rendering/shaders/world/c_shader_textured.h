#ifndef __C_SHADER_TEXTURED_H__
#define __C_SHADER_TEXTURED_H__
// prosper TODO
#if 0
#include "pragma/rendering/shaders/c_shader.h"
#include "shader_textured.h"
#include <pragma/game/game_limits.h>

#define SHADER_TEX_UNIT_DIFFUSE_MAP 0
#define SHADER_TEX_UNIT_NORMAL_MAP 2
#define SHADER_TEX_UNIT_BUMP_MAP 3
#define SHADER_TEX_UNIT_SPECULAR_MAP 5
#define SHADER_TEX_UNIT_GLOW_MAP 6
#define SHADER_TEX_UNIT_ALPHA_MAP 7
#define SHADER_TEX_UNIT_DISPLACEMENT_MAP 8
#define SHADER_TEX_UNIT_PARALLAX_MAP 9
#define SHADER_SHADOW_MAP_TEXTURE_START 20 // Vulkan TODO: Remove these!

#define DEFAULT_PARALLAX_HEIGHT_SCALE 0.025f
#define DEFAULT_ALPHA_DISCARD_THRESHOLD 0.99f

//#define SHADER_USE_PCF_JITTER
#ifdef SHADER_USE_PCF_JITTER
	#define SHADER_TEX_UNIT_SHADOW_JITTER_MAP 9
#endif

class Scene;
namespace Shader
{
	class DLLCLIENT TexturedBase3D
		: public Base3D,public TexturedBase
	{
	protected:
		static std::unordered_map<uint32_t,std::vector<Vulkan::SwapDescriptorBuffer>> s_descriptorCache;
		static uint32_t s_shaderInstanceCount;
	public:
#pragma pack(push,1)
		struct DLLCLIENT Fog
		{
			enum class DLLCLIENT Type : uint32_t
			{
				Linear = 0,
				Exponential,
				Exponential2
			};
			enum class DLLCLIENT Flag : uint32_t
			{
				None = 0,
				Enabled = 1
			};
			Vector4 color = {0.f,0.f,0.f,0.f};
			float start = 0.f;
			float end = 0.f;
			float density = 0.f;
			Type type = Type::Linear;
			Flag flags = Flag::None;
		};
#pragma pack(pop)
		enum class DLLCLIENT PipelineType : uint32_t
		{
			Regular = 0,
			Reflection, // Inverted culling
			Count
		};
		enum class DLLCLIENT DescSet : uint32_t
		{
			Instance = 0,
			BoneMatrix = Instance,
			Camera = BoneMatrix +1,
			RenderSettings = Camera,
			SSAOMap = RenderSettings,
			DiffuseMap = SSAOMap +1,
			NormalMap = DiffuseMap,
			SpecularMap = NormalMap,
			ParallaxMap = SpecularMap,
			GlowMap = ParallaxMap,
			Debug = GlowMap +1,
			Time = Debug,
			CSM = Debug,
			LightSources = CSM +1,
			TileVisLightIndexBuffer = LightSources,
			ShadowData = LightSources,
			CSMTextureArray = ShadowData +1,
			ShadowMaps = CSMTextureArray +1,
			ShadowCubeMaps = ShadowMaps
		};
		enum class DLLCLIENT Location : uint32_t
		{
			Vertex = 2,
			UV = 3,
			Normal = 4,
			Tangent = 5,
			BiTangent = 6,
			BoneWeightId = 0,
			BoneWeight = 1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Vertex = 0,
			UV = 0,
			Normal = 0,
			Tangent = 0,
			BiTangent = 0,
			BoneWeightId = 1,
			BoneWeight = 1,

			Instance = 0,
			BoneMatrix = Instance +1,
			VertexAnimations = BoneMatrix +1,
			VertexAnimationFrameData = VertexAnimations +1,

			Camera = 0,
			RenderSettings = Camera +1,
			SSAOMap = RenderSettings +1,

			DiffuseMap = 0,
			NormalMap = 1,
			SpecularMap = 2,
			ParallaxMap = 3,
			GlowMap = 4,
			Debug = 0,
			Time = 1,
			CSM = 2,
			LightSources = 0,
			TileVisLightIndexBuffer = LightSources +1,
			ShadowData = TileVisLightIndexBuffer +1,
			CSMTextureArray = 0,
			ShadowMaps = 0u,
			ShadowCubeMaps = ShadowMaps +umath::to_integral(GameLimits::MaxActiveShadowMaps)
		};
		static const uint32_t PUSH_CONSTANT_COUNT;
		static const uint32_t MAX_ENTITY_COUNT;
	protected:
		TexturedBase3D(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");

		std::size_t m_reflectionPipelineIdx = std::numeric_limits<std::size_t>::max();
		bool m_bUseBloomAttachment;
		bool m_bUseBoneWeights;
		bool m_bUseMultiSample;
		const Scene *m_boundScene = nullptr;
		uint32_t m_vertexAnimOffset = 0u;

		virtual void BindMaterialParameters(Material *mat);
		void BindMaterial(uint32_t descSet,Material *mat);
		const Vulkan::DescriptorSet &InitializeDescriptorSet(Material *mat,bool bReload=false);
		void BindScene(uint32_t descSetSceneId,Vulkan::CommandBufferObject *cmdBuffer,const Scene &scene,bool bView);
		virtual void InitializeMaterialBindings(std::vector<Vulkan::DescriptorSetLayout::Binding> &bindings);
		virtual void InitializePipeline() override;
		virtual void InitializeShader() override;
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
		virtual void InitializeAttachments(std::vector<vk::PipelineColorBlendAttachmentState> &attachments) override;
		virtual void InitializeRenderPasses() override;
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
		virtual bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline) override;
		virtual bool BindEntity(Vulkan::CommandBufferObject *cmdBuffer,CBaseEntity *ent,bool &bWeighted);
		void PushVertexAnimationOffset(Vulkan::CommandBufferObject *cmdBuffer,uint32_t offset);
	public:
		virtual ~TexturedBase3D() override;
		virtual TexturedBase3D *Get3DTexturedShader() override;
		virtual void Initialize() override;

		virtual void InitializeMaterial(Material *mat,bool bReload=false) override;
		static Vulkan::DescriptorSet CreateLightDescriptorSet();
		static Vulkan::DescriptorSet CreateShadowMapDescriptorSet();
		static Vulkan::Buffer CreateFogBuffer(const Fog &fog);
		void SetUseBloomAttachment(bool b);
		bool GetUseBloomAttachment() const;
		bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,PipelineType type);
		void BindScene(const Scene &scene,bool bView=false);
		virtual void BindScene(Vulkan::CommandBufferObject *cmdBuffer,const Scene &scene,bool bView=false);
		virtual void BindMaterial(Material *mat);
		void BindInstanceDescriptorSet(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSetObject *descSetInstance,uint32_t baseOffset=0);
		bool BindEntity(Vulkan::CommandBufferObject *cmdBuffer,CBaseEntity *ent);
		bool BindEntity(CBaseEntity *ent);
		virtual void EndDraw(Vulkan::CommandBufferObject *cmdBuffer) override;
		using Base3D::BeginDraw;
		using Base3D::EndDraw;

		void SetVertexAnimationOffset(Vulkan::CommandBufferObject *cmdBuffer,uint32_t offset);
		void SetVertexAnimationOffset(Vulkan::CommandBufferObject *cmdBuffer);

		virtual void BindLights(const Vulkan::DescriptorSetObject *shadowMapDescSet,const Vulkan::DescriptorSetObject *lightSourcesDescSet);
	};

	class DLLCLIENT Textured3D
		: public TexturedBase3D
	{
	protected:
		virtual void InitializeShader() override;
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
		Textured3D(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
	public:
		Textured3D();
		Textured3D(std::string identifier);
		virtual void BindMaterial(Material *mat) override;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(Shader::TexturedBase3D::Fog::Flag);
#endif
#endif
