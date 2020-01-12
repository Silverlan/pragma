#ifndef __C_SHADER_SHADOW_HPP__
#define __C_SHADER_SHADOW_HPP__

#include "pragma/rendering/shaders/world/c_shader_scene.hpp"

namespace pragma
{
	class CLightPointComponent;
	class CLightSpotComponent;
	class CLightComponent;
	class DLLCLIENT ShaderShadow
		: public ShaderEntity
	{
	public:
		static Anvil::Format RENDER_PASS_DEPTH_FORMAT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_ID;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;

		enum class Flags : uint32_t
		{
			None = 0u,
			UseExtendedVertexWeights = 1u
		};

#pragma pack(push,1)
		struct PushConstants
		{
			Mat4 depthMVP;
			Vector4 lightPos; // 4th component stores the distance
			Flags flags;
		};
#pragma pack(pop)

		ShaderShadow(prosper::Context &context,const std::string &identifier);
		ShaderShadow(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader);

		bool BindLight(CLightComponent &light);
		bool BindEntity(CBaseEntity &ent,const Mat4 &depthMVP);
		bool BindMaterial(CMaterial &mat); // TODO: Transparent only
		virtual bool Draw(CModelSubMesh &mesh) override;
	protected:
		bool BindDepthMatrix(const Mat4 &depthMVP);
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	private:
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetLightDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const override;
		virtual bool BindEntity(CBaseEntity &ent) override {return false;}
	};

	class DLLCLIENT ShaderShadowTransparent
		: public ShaderShadow
	{
	public:
		//bool BindMaterial(CMaterial &mat);
	};

	//////////////////

	class DLLCLIENT ShaderShadowSpot
		: public ShaderShadow
	{
	public:
		ShaderShadowSpot(prosper::Context &context,const std::string &identifier);
	};

	//////////////////

	class DLLCLIENT ShaderShadowCSM
		: public ShaderShadow
	{
	public:
		ShaderShadowCSM(prosper::Context &context,const std::string &identifier);
	protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};

	//////////////////

	class DLLCLIENT ShaderShadowCSMTransparent
		: public ShaderShadowCSM
	{
	public:
		//bool BindMaterial(CMaterial &mat);
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderShadow::Flags)

// prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_textured.h"
#include "pragma/rendering/c_entitymeshinfo.h"
#include "pragma/rendering/lighting/c_light_ranged.h"
#include "pragma/rendering/lighting/c_light.h"

#define DEBUG_SHADOWS 0

struct DLLCLIENT EntityContainer
{
	EntityContainer(CBaseEntity *ent)
		: entity(ent)
	{}
	EntityContainer &operator=(const EntityContainer &other)=delete;
	EntityContainer(EntityContainer&)=delete;
	CBaseEntity *entity;
	std::vector<CModelSubMesh*> meshes;
};

struct DLLCLIENT LightContainer
{
	LightContainer(CLightBase *l)
		: light(l)
	{}
	LightContainer(LightContainer&)=delete;
	LightContainer &operator=(const LightContainer &other)=delete;
	CLightBase *light;
	std::vector<std::unique_ptr<EntityContainer>> containers;
};

class CLightDirectional;
namespace Vulkan {class SwapCommandBuffer;};
namespace Shader
{
	//class Frustum;
	class DLLCLIENT Shadow
		: public TexturedBase3D
	{
	public:
		enum class DLLCLIENT DescSet : uint32_t
		{
			Instance = 0,
			BoneMatrix = Instance
		};
		enum class DLLCLIENT Location : uint32_t
		{
			Vertex = 2,
			BoneWeightId = 0,
			BoneWeight = 1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Vertex = 0,
			BoneWeightId = 1,
			BoneWeight = 1,

			Instance = 0,
			BoneMatrix = Instance +1
		};
	protected:
		struct DLLCLIENT MeshInfo
		{
			MeshInfo(Material *mat,CModelSubMesh *mesh);
			Material *material;
			CModelSubMesh *mesh;
		};
		bool m_bWeighted = false;
		CLightBase *m_lightSource = nullptr;
		uint32_t m_renderFlags = 0;
		Shadow(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		virtual void BindLight(Vulkan::CommandBufferObject *cmdBuffer,CLightBase *light);
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		void RenderMeshes(Vulkan::CommandBufferObject *cmdBuffer,uint32_t idxLayer,std::vector<std::shared_ptr<CLightBase::EntityInfo>> &meshes,const Mat4 &depthMvp,bool bPushConstants=true,std::unordered_map<CBaseEntity*,std::unique_ptr<std::vector<std::shared_ptr<MeshInfo>>>> *transparentMeshes=nullptr);
		virtual bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline) override;
		virtual void EndDraw(Vulkan::CommandBufferObject *cmdBuffer) override;
		virtual void BindScene(Vulkan::CommandBufferObject *cmdBuffer,const Scene &scene,bool bView=false) override;
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
		virtual void InitializeRenderPasses() override;
		using TexturedBase3D::BeginDraw;
		using TexturedBase3D::EndDraw;
		void RenderMesh(Vulkan::CommandBufferObject *cmdBuffer,CModelSubMesh *mesh,bool bWeighted=false);
		void RenderMesh(Vulkan::CommandBufferObject *cmdBuffer,CModelSubMesh *mesh,bool bWeighted,const std::function<void(const Vulkan::CommandBufferObject*,uint32_t)> &fDraw);
	public:
		Shadow();
		void Render(Vulkan::CommandBufferObject *cmdBuffer,CLightBase *light,uint32_t imgLayer=0,CLightBase::RenderPass rp=CLightBase::RenderPass::Dynamic);
		void Render(Vulkan::CommandBufferObject *cmdBuffer,CLightBase *light,std::vector<std::shared_ptr<CLightBase::EntityInfo>> &meshes,uint32_t w,uint32_t h,uint32_t imgLayer=0,CLightBase::RenderPass rp=CLightBase::RenderPass::Dynamic);

		bool BindEntity(Vulkan::CommandBufferObject *cmdBuffer,CBaseEntity *ent,const Mat4 &depthMvp);
		void BindDepthMatrix(Vulkan::CommandBufferObject *cmdBuffer,const Mat4 &mat);
		bool BeginDrawTest(CLightBase *light,uint32_t w,uint32_t h);
		void EndDrawTest();
		void DrawTest(Vulkan::CommandBufferObject *cmdBuffer,CModelSubMesh *mesh);
		void DrawTest(Vulkan::CommandBufferObject *cmdBuffer,CModelSubMesh *mesh,const std::function<void(const Vulkan::CommandBufferObject*,uint32_t)> &fDraw);
	};
	class DLLCLIENT ShadowSpot
		: public Shadow
	{
	public:
		ShadowSpot();
	};

	class DLLCLIENT ShadowTransparent
		: public Shadow
	{
	private:
		using Shadow::Render;
	protected:
		ShadowTransparent(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		enum class DLLCLIENT DescSet : uint32_t
		{
			DiffuseMap = umath::to_integral(Shadow::DescSet::BoneMatrix) +1,
		};
		enum class DLLCLIENT Location : uint32_t
		{
			UV = umath::to_integral(Shadow::Location::Vertex) +1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			DiffuseMap = 0,
			UV = umath::to_integral(Shadow::Binding::Vertex)
		};
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
	public:
		ShadowTransparent();
		void Render(Vulkan::CommandBufferObject *cmdBuffer,CLightBase *light,uint32_t imgLayer,CLightBase::RenderPass rp,const std::unordered_map<CBaseEntity*,std::unique_ptr<std::vector<std::shared_ptr<MeshInfo>>>> &meshInfo);
		bool BindMaterial(Vulkan::CommandBufferObject *cmdBuffer,Material *mat);
	};
	class DLLCLIENT ShadowTransparentSpot
		: public ShadowTransparent
	{
	public:
		ShadowTransparentSpot();
	};

	class DLLCLIENT ShadowCSM
		: public Shadow
	{
	protected:
		ShadowCSM(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void BindLight(Vulkan::CommandBufferObject *cmdBuffer,CLightBase *light) override;
	public:
		ShadowCSM();
	};

	class DLLCLIENT ShadowCSMTransparent
		: public ShadowCSM
	{
	protected:
		enum class DLLCLIENT DescSet : uint32_t
		{
			DiffuseMap = umath::to_integral(ShadowCSM::DescSet::BoneMatrix) +1,
		};
		enum class DLLCLIENT Location : uint32_t
		{
			UV = umath::to_integral(ShadowCSM::Location::Vertex) +1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			DiffuseMap = 0,
			UV = umath::to_integral(ShadowCSM::Binding::Vertex)
		};
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
	public:
		ShadowCSMTransparent();
		bool BindMaterial(Vulkan::CommandBufferObject *cmdBuffer,Material *mat);
	};

	class DLLCLIENT ShadowCSMStatic
		: public ShadowCSM
	{
	public:
		static const uint32_t MAX_CASCADE_COUNT;
		enum class DLLCLIENT DescSet : uint32_t
		{
			DepthMatrix = umath::to_integral(Shadow::DescSet::BoneMatrix) +1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			DepthMatrix = 0
		};
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	public:
		ShadowCSMStatic();
		void Render(CLightDirectional &light,std::vector<Vulkan::SwapCommandBuffer> &cmdBuffers,CLightBase::EntityInfo &info);
	};
};
#endif
#endif