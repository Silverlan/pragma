/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __SHADER_MATERIAL_HPP__
#define __SHADER_MATERIAL_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shader_input_data.hpp"
#include <udm.hpp>

import pragma.shadergraph;

namespace pragma::rendering {
	struct ShaderInputData;
};

namespace pragma::rendering::shader_material {
	constexpr size_t MAX_MATERIAL_SIZE = 128; // Max size per material in bytes

	enum class MaterialFlags : uint32_t {
		None = 0,
		HasNormalMap = 1,
		HasParallaxMap = HasNormalMap << 1,
		HasEmissionMap = HasParallaxMap << 1,
		Translucent = HasEmissionMap << 1,
		BlackToAlpha = Translucent << 1,

		GlowMode1 = BlackToAlpha << 1,
		GlowMode2 = GlowMode1 << 1,
		GlowMode3 = GlowMode2 << 1,
		GlowMode4 = GlowMode3 << 1,

		// See MAX_NUMBER_OF_SRGB_TEXTURES
		Srgb0 = GlowMode4 << 1u,
		Srgb1 = Srgb0 << 1u,
		Srgb2 = Srgb1 << 1u,
		Srgb3 = Srgb2 << 1u,

		Debug = Srgb3 << 1u,

		HasWrinkleMaps = Debug << 1u,
		HasRmaMap = HasWrinkleMaps << 1u,

		User = HasRmaMap << 1u,

		Last = User,
	};
	constexpr uint32_t MAX_NUMBER_OF_SRGB_TEXTURES = 4;

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

		static void PopulateShaderInputDataFromMaterial(ShaderInputData &inputData, const CMaterial &mat);
		static MaterialFlags GetFlagsFromShaderInputData(const ShaderInputData &inputData);
		static void SetShaderInputDataFlags(ShaderInputData &inputData, MaterialFlags flags);
		static std::string GetTextureUniformVariableName(const std::string &texIdentifier);

		ShaderMaterial(const pragma::GString &name);
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
REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::shader_material::MaterialFlags)

#endif
