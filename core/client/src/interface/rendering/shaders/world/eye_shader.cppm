// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.world_eye;

export import :rendering.shaders.pbr;

export namespace pragma {
	class DLLCLIENT ShaderEye : public ShaderPBR {
	  public:
#pragma pack(push, 1)
		struct PushConstants {
			Vector4 irisProjectionU = {};
			Vector4 irisProjectionV = {};
			union {
				Vector4 eyeOrigin = {};
				Vector2 irisUvClampRange;
			};

			float maxDilationFactor = 1.f;
			float dilationFactor = 0.5f;
			float irisUvRadius = 0.2f;
		};
#pragma pack(pop)

		ShaderEye(prosper::IPrContext &context, const std::string &identifier);

		//
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, SceneFlags &inOutSceneFlags) const override;
		virtual bool OnRecordDrawMesh(rendering::ShaderProcessor &shaderProcessor, geometry::CModelSubMesh &mesh) const override;
		virtual bool IsLegacyShader() const { return false; }
	  protected:
		ShaderEye(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");
		bool BindEyeball(rendering::ShaderProcessor &shaderProcessor, uint32_t skinMatIdx) const;
		virtual void InitializeGfxPipelinePushConstantRanges() override;
	};

	class DLLCLIENT ShaderEyeLegacy : public ShaderEye {
	  public:
		ShaderEyeLegacy(prosper::IPrContext &context, const std::string &identifier);
		virtual bool IsLegacyShader() const override { return true; }
	  private:
	};
};
