#ifndef __C_SHADER_FLAT_HPP__
#define __C_SHADER_FLAT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"

namespace pragma
{
	class DLLCLIENT ShaderFlat
		: public ShaderScene
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_UV;

		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;

		ShaderFlat(prosper::Context &context,const std::string &identifier);

		bool BindScene(const Scene &scene,bool bView);
		bool BindEntity(CBaseEntity &ent);
		bool BindMaterial(CMaterial &mat);
		bool Draw(CModelSubMesh &mesh);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	private:
		// These are unused
		virtual bool BindSceneCamera(const Scene &scene,bool bView) override {return false;}
		virtual bool BindRenderSettings(Anvil::DescriptorSet &descSetRenderSettings) override {return false;}
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override {return std::numeric_limits<uint32_t>::max();}
		virtual uint32_t GetCameraDescriptorSetIndex() const override {return std::numeric_limits<uint32_t>::max();}
	};
};

#endif
