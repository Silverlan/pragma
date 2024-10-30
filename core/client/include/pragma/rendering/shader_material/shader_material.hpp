/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __SHADER_MATERIAL_HPP__
#define __SHADER_MATERIAL_HPP__

#include "pragma/clientdefinitions.h"
#include <udm.hpp>

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

	using PropertyValue = std::variant<udm::Int16, udm::UInt16, udm::Int32, udm::UInt32, udm::Float, udm::Half, udm::Vector2, udm::Vector3, udm::Vector4, udm::Vector2i, udm::Vector3i, udm::Vector4i>;
	constexpr bool is_valid_property_type(udm::Type type)
	{
		switch(type) {
		case udm::Type::Int16:
		case udm::Type::UInt16:
		case udm::Type::Int32:
		case udm::Type::UInt32:
		case udm::Type::Float:
		case udm::Type::Half:
		case udm::Type::Vector2:
		case udm::Type::Vector3:
		case udm::Type::Vector4:
		case udm::Type::Vector2i:
		case udm::Type::Vector3i:
		case udm::Type::Vector4i:
			return true;
		}
		return false;
	}
	template<typename T>
	concept is_valid_property_type_v = is_valid_property_type(udm::type_to_enum<T>());

	struct DLLCLIENT Property {
		Property() = default;
		Property(const Property &other);
		Property &operator=(const Property &other);
		udm::Type type;
		std::optional<GString> specializationType {};
		GString name;
		PropertyValue defaultValue;
		size_t offset = 0;
		size_t padding = 0;

		std::unique_ptr<std::unordered_map<std::string, PropertyValue>> options {};
		std::unique_ptr<std::unordered_map<std::string, uint32_t>> flags {};
		size_t GetSize() const { return udm::size_of(type); }
	};
	struct DLLCLIENT Texture {
		GString name;
		std::optional<std::string> defaultTexturePath {};
		bool cubemap = false;
		bool colorMap = false;
		bool required = false;
	};
	struct DLLCLIENT ShaderMaterial {
		static constexpr uint32_t PREDEFINED_PROPERTY_COUNT = 6;

		ShaderMaterial();
		void AddProperty(Property &&prop);
		std::vector<Property> properties;
		std::vector<Texture> textures;

		Property *FindProperty(const char *key)
		{
			auto it = std::find_if(properties.begin(), properties.end(), [key](const Property &prop) { return prop.name == key; });
			if(it == properties.end())
				return nullptr;
			return &*it;
		}
		const Property *FindProperty(const char *key) const { return const_cast<ShaderMaterial *>(this)->FindProperty(key); }

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
	};

	struct ShaderMaterialData {
		ShaderMaterialData(const ShaderMaterial &shaderMaterial) : m_shaderMaterial {shaderMaterial} {}
		void DebugPrint();
		void PopulateFromMaterial(const CMaterial &mat);
		MaterialFlags GetFlags() const;
		void SetFlags(MaterialFlags flags);

		template<typename T>
		    requires is_valid_property_type_v<T>
		std::optional<T> GetValue(const char *key) const
		{
			constexpr auto type = udm::type_to_enum<T>();
			auto *prop = m_shaderMaterial.FindProperty(key);
			if(!prop)
				return {};
			size_t offset = prop->offset;
			auto *ptr = data.data() + offset;
			return udm::visit_ng(prop->type, [prop, ptr](auto tag) -> std::optional<T> {
				using TProp = typename decltype(tag)::type;
				if constexpr(udm::is_convertible<TProp, T>()) {
					auto *val = reinterpret_cast<const TProp *>(ptr);
					return udm::convert<TProp, T>(*val);
				}
				return {};
			});
		}
		template<typename T>
		    requires is_valid_property_type_v<T>
		bool SetValue(const char *key, const T &val)
		{
			constexpr auto type = udm::type_to_enum<T>();
			auto *prop = m_shaderMaterial.FindProperty(key);
			if(!prop)
				return false;
			auto *ptr = data.data() + prop->offset;
			return udm::visit_ng(prop->type, [prop, ptr, &val](auto tag) -> bool {
				using TProp = typename decltype(tag)::type;
				if constexpr(udm::is_convertible<T, TProp>()) {
					auto convVal = udm::convert<T, TProp>(val);
					memcpy(ptr, &convVal, sizeof(convVal));
					return true;
				}
				return false;
			});
		}

		std::array<uint8_t, pragma::rendering::shader_material::MAX_MATERIAL_SIZE> data;
	  private:
		const ShaderMaterial &m_shaderMaterial;
	};

	class DLLCLIENT ShaderMaterialCache {
	  public:
		ShaderMaterialCache();
		std::shared_ptr<ShaderMaterial> Load(const std::string &id);
	  private:
		std::unordered_map<std::string, std::shared_ptr<ShaderMaterial>> m_cache;
	};
	DLLCLIENT ShaderMaterialCache &get_cache();
	DLLCLIENT void clear_cache();
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::shader_material::MaterialFlags)

#endif
