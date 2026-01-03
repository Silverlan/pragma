// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_material;

export import :rendering.shader_input_data;
export import :rendering.shader_material_enums;
export import pragma.cmaterialsystem;
export import pragma.shadergraph;

export namespace pragma::rendering::shader_material {
	struct DLLCLIENT Texture {
		GString name;
		std::optional<std::string> defaultTexturePath {};
		std::optional<GString> specializationType {};
		bool cubemap = false;
		bool colorMap = false;
		bool required = false;
	};
	struct DLLCLIENT ShaderMaterial : public ShaderInputDescriptor {
		static constexpr uint32_t PREDEFINED_PROPERTY_COUNT = 6;

		static void PopulateShaderInputDataFromMaterial(ShaderInputData &inputData, const material::CMaterial &mat);
		static MaterialFlags GetFlagsFromShaderInputData(const ShaderInputData &inputData);
		static void SetShaderInputDataFlags(ShaderInputData &inputData, MaterialFlags flags);
		static std::string GetTextureUniformVariableName(const std::string &texIdentifier);

		ShaderMaterial(const GString &name);
		std::vector<Texture> textures;

		Texture *FindTexture(const char *key)
		{
			auto it = std::find_if(textures.begin(), textures.end(), [key](const Texture &tex) { return tex.name == key; });
			if(it == textures.end())
				return nullptr;
			return &*it;
		}
		const Texture *FindTexture(const char *key) const { return const_cast<ShaderMaterial *>(this)->FindTexture(key); }

		bool LoadFromUdmData(udm::LinkedPropertyWrapperArg prop, std::string &outErr);
		std::string ToGlslStruct() const;
	  protected:
		virtual ShaderInputDescriptor *Import(const std::string &name) override;
	};

	class DLLCLIENT ShaderMaterialCache {
	  public:
		ShaderMaterialCache();
		std::shared_ptr<ShaderMaterial> Load(const std::string &id);
		std::shared_ptr<ShaderMaterial> Get(const std::string &id) const;
		const std::unordered_map<std::string, std::shared_ptr<ShaderMaterial>> &GetShaderMaterials() const { return m_cache; }
	  private:
		std::unordered_map<std::string, std::shared_ptr<ShaderMaterial>> m_cache;
	};
	DLLCLIENT ShaderMaterialCache &get_cache();
	DLLCLIENT void clear_cache();
};
