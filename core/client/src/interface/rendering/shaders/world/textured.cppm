// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.textured;

export import :rendering.shaders.textured_enums;

export import :debug.enums;
export import :entities.components.rasterization_renderer;
export import :entities.components.scene;
export import :rendering.shader_input_data;
export import :rendering.shader_material_enums;
export import :rendering.shaders.scene;

export namespace pragma::rendering::shader_material {
	struct ShaderMaterial;
};
export namespace pragma {
	class DLLCLIENT ShaderSpecializationManager {
	  public:
		using PassTypeIndex = uint32_t;
		using SpecializationFlags = uint64_t;
		using ConstantId = uint32_t;
		uint32_t GetPipelineCount() const { return m_pipelineSpecializations.size(); }
		PassTypeIndex GetBasePassType(uint32_t pipelineIdx) const;
		uint32_t GetPipelineIndexStartOffset(PassTypeIndex passType) const;
		std::optional<uint32_t> FindSpecializationPipelineIndex(PassTypeIndex passType, SpecializationFlags specializationFlags) const;
		template<typename TPassType, typename TSpecializationFlags>
		std::optional<uint32_t> FindSpecializationPipelineIndex(TPassType passType, TSpecializationFlags specializationFlags) const
		{
			return FindSpecializationPipelineIndex(static_cast<PassTypeIndex>(passType), static_cast<SpecializationFlags>(specializationFlags));
		}
	  protected:
		bool IsSpecializationConstantSet(uint32_t pipelineIdx, SpecializationFlags flag) const;
		void SetPipelineIndexRange(PassTypeIndex passType, uint32_t startIndex, uint32_t endIndex);
		template<typename TSpecializationConstantFlag>
		bool IsSpecializationConstantSet(uint32_t pipelineIdx, TSpecializationConstantFlag flag) const
		{
			return IsSpecializationConstantSet(pipelineIdx, static_cast<SpecializationFlags>(flag));
		}
		void RegisterSpecializations(PassTypeIndex passType, SpecializationFlags staticFlags, SpecializationFlags dynamicFlags);
		template<typename TPassType, typename TSpecializationConstantFlag>
		void RegisterSpecializations(TPassType passType, TSpecializationConstantFlag staticFlags, TSpecializationConstantFlag dynamicFlags)
		{
			RegisterSpecializations(static_cast<PassTypeIndex>(passType), static_cast<SpecializationFlags>(staticFlags), static_cast<SpecializationFlags>(dynamicFlags));
		}
		template<typename TSpecializationConstantFlag>
		bool AddSpecializationConstant(prosper::ShaderGraphics &shader, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx, prosper::ShaderStageFlags stageFlags, TSpecializationConstantFlag flag)
		{
			uint32_t isSet = static_cast<uint32_t>(IsSpecializationConstantSet(pipelineIdx, flag));
			return shader.AddSpecializationConstant(pipelineInfo, stageFlags, math::get_least_significant_set_bit_index(math::to_integral(flag)), sizeof(uint32_t), &isSet);
		}
	  private:
		std::vector<SpecializationFlags> m_pipelineSpecializations;
		struct PassTypeInfo {
			std::array<uint32_t, math::to_integral(GameShaderSpecializationConstantFlag::PermutationCount)> specializationToPipelineIdx;
			std::pair<uint32_t, uint32_t> pipelineIndexRange {0, 0};
		};
		PassTypeInfo &InitializePassTypeSpecializations(PassTypeIndex passType);
		// Per pass-type
		std::vector<PassTypeInfo> m_passTypeSpecializationToPipelineIdx;
	};

	class DLLCLIENT ShaderGameWorldLightingPass : public ShaderGameWorld, public ShaderSpecializationManager {
	  public:
		static ShaderGraphics::VertexBinding VERTEX_BINDING_RENDER_BUFFER_INDEX;
		static VertexAttribute VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX;

		static ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT;
		static VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_ID;
		static VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT;

		static ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT_EXT;
		static VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID;
		static VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT;

		static ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static VertexAttribute VERTEX_ATTRIBUTE_UV;
		static VertexAttribute VERTEX_ATTRIBUTE_NORMAL;
		static VertexAttribute VERTEX_ATTRIBUTE_TANGENT;
		static VertexAttribute VERTEX_ATTRIBUTE_BI_TANGENT;

		static ShaderGraphics::VertexBinding VERTEX_BINDING_LIGHTMAP;
		static VertexAttribute VERTEX_ATTRIBUTE_LIGHTMAP_UV;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDERER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SHADOWS;

		static bool InitializeMaterialBuffer(prosper::IDescriptorSet &descSet, material::CMaterial &mat, const rendering::ShaderInputData &matData, uint32_t bindingIdx);
		static std::unique_ptr<prosper::DescriptorSetInfo> CreateMaterialDescriptorSetInfo(const rendering::shader_material::ShaderMaterial &shaderMaterial);

		enum class VertexBinding : uint32_t { LightmapUv = math::to_integral(ShaderEntity::VertexBinding::Count) };

		enum class MaterialBinding : uint32_t {
			MaterialSettings = 0u,
			DiffuseMap,
			NormalMap,
			SpecularMap,
			ParallaxMap,
			GlowMap,

			Count
		};

		enum class InstanceBinding : uint32_t { Instance = 0u, BoneMatrices, VertexAnimations, VertexAnimationFrameData };

#pragma pack(push, 1)
		struct PushConstants : public ScenePushConstants {
			static_assert(sizeof(SceneDebugMode) == sizeof(uint32_t));
			SceneDebugMode debugMode;
			float reflectionProbeIntensity;
			Vector2 padding;

			void Initialize()
			{
				ScenePushConstants::Initialize();
				debugMode = SceneDebugMode::None;
				reflectionProbeIntensity = 1.f;
			}
		};
#pragma pack(pop)

		ShaderGameWorldLightingPass(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");
		virtual ~ShaderGameWorldLightingPass() override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(material::CMaterial &mat) override;
		virtual bool GetRenderBufferTargets(geometry::CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const override;
		virtual uint32_t GetSceneDescriptorSetIndex() const override;
		bool RecordPushSceneConstants(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const Vector4 &drawOrigin) const;

		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetRendererDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual uint32_t GetPassPipelineIndexStartOffset(rendering::PassType passType) const override;
		std::optional<uint32_t> FindPipelineIndex(rendering::PassType passType, GameShaderSpecialization specialization, GameShaderSpecializationConstantFlag specializationFlags) const;
		virtual GameShaderSpecializationConstantFlag GetMaterialPipelineSpecializationRequirements(material::CMaterial &mat) const;
		virtual bool IsTranslucentPipeline(uint32_t pipelineIdx) const;

		void SetShaderMaterialName(const std::optional<std::string> &shaderMaterial);
		const std::optional<std::string> &GetShaderMaterialName() const;
		const rendering::shader_material::ShaderMaterial *GetShaderMaterial() const { return m_shaderMaterial.get(); }

		virtual GameShaderSpecializationConstantFlag GetBaseSpecializationFlags() const;
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, SceneFlags &inOutSceneFlags) const override;
		virtual bool IsUsingLightmaps() const override { return true; }
		bool IsDepthPrepassEnabled() const;
		void SetDepthPrepassEnabled(bool enabled) { m_depthPrepassEnabled = enabled; }
		static std::shared_ptr<material::Texture> GetTexture(const std::string &texName, bool load = false);
	  protected:
		using ShaderEntity::RecordDraw;
		GameShaderSpecializationConstantFlag GetStaticSpecializationConstantFlags(GameShaderSpecialization specialization) const;
		virtual void OnPipelinesInitialized() override;
		virtual void ApplyMaterialFlags(material::CMaterial &mat, rendering::shader_material::MaterialFlags &outFlags) const;
		virtual void UpdateRenderFlags(geometry::CModelSubMesh &mesh, SceneFlags &inOutFlags);
		virtual void GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode) override;
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(material::CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo);
		bool InitializeMaterialBuffer(prosper::IDescriptorSet &descSet, material::CMaterial &mat, const rendering::ShaderInputData &matData);
		virtual void InitializeGfxPipelineVertexAttributes();
		virtual void InitializeGfxPipelinePushConstantRanges();
		virtual void InitializeGfxPipelineDescriptorSets();
		prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		virtual void InitializeShaderResources() override;
		virtual void InitializeMaterialData(const material::CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, rendering::ShaderInputData &inOutMatData);
		virtual void InitializeShaderMaterial();
		bool m_depthPrepassEnabled = true;
		std::optional<std::string> m_shaderMaterialName = "pbr";
		std::shared_ptr<rendering::shader_material::ShaderMaterial> m_shaderMaterial;
	};
};
