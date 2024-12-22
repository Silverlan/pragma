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

import pragma.shadergraph;

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

	struct DLLCLIENT Property {
		enum class Flags : uint32_t {
			None = 0u,
			HideInEditor = 1u,
		};
		Property(const std::string &name, pragma::shadergraph::DataType type);
		Property(const Property &other);
		Property &operator=(const Property &other);

		pragma::shadergraph::Parameter *operator->() { return &parameter; }
		const pragma::shadergraph::Parameter *operator->() const { return &parameter; }

		pragma::shadergraph::Parameter parameter;

		std::optional<GString> specializationType {};
		Flags propertyFlags = Flags::None;
		size_t offset = 0;
		size_t padding = 0;

		std::unique_ptr<std::unordered_map<std::string, uint32_t>> flags {};
		size_t GetSize() const { return udm::size_of(pragma::shadergraph::to_udm_type(parameter.type)); }
	};
	struct DLLCLIENT Texture {
		GString name;
		std::optional<std::string> defaultTexturePath {};
		std::optional<GString> specializationType {};
		bool cubemap = false;
		bool colorMap = false;
		bool required = false;
	};
	struct DLLCLIENT ShaderInputDescriptor {
		ShaderInputDescriptor(const pragma::GString &name);
		void AddProperty(Property &&prop);
		const pragma::GString name;
		std::vector<Property> properties;

		Property *FindProperty(const char *key)
		{
			auto it = m_propertyMap.find(key);
			if(it == m_propertyMap.end())
				return nullptr;
			return &properties[it->second];
		}
		const Property *FindProperty(const char *key) const { return const_cast<ShaderInputDescriptor *>(this)->FindProperty(key); }

		bool LoadFromUdmData(udm::LinkedPropertyWrapperArg prop, std::string &outErr);
		std::string ToGlslStruct() const;
	  private:
		std::unordered_map<std::string, size_t> m_propertyMap;
	};
	struct DLLCLIENT ShaderMaterial : public ShaderInputDescriptor {
		static constexpr uint32_t PREDEFINED_PROPERTY_COUNT = 6;

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
	};

	struct ShaderInputData {
		ShaderInputData(const ShaderInputDescriptor &descriptor) : m_inputDescriptor {descriptor} {}
		void ResetToDefault();
		void ResetToDefault(size_t idx);
		void ResizeToDescriptor();
		void DebugPrint();
		void PopulateFromMaterial(const CMaterial &mat);
		MaterialFlags GetFlags() const;
		void SetFlags(MaterialFlags flags);

		template<typename T>
		    requires pragma::shadergraph::is_data_type_v<T>
		std::optional<T> GetValue(const char *key) const
		{
			constexpr auto type = udm::type_to_enum<T>();
			auto *prop = m_inputDescriptor.FindProperty(key);
			if(!prop)
				return {};
			size_t offset = prop->offset;
			auto *ptr = data.data() + offset;
			return udm::visit_ng(pragma::shadergraph::to_udm_type(prop->parameter.type), [prop, ptr](auto tag) -> std::optional<T> {
				using TProp = typename decltype(tag)::type;
				if constexpr(udm::is_convertible<TProp, T>()) {
					auto *val = reinterpret_cast<const TProp *>(ptr);
					return udm::convert<TProp, T>(*val);
				}
				return {};
			});
		}
		template<typename T>
		    requires pragma::shadergraph::is_data_type_v<T>
		bool SetValue(const char *key, const T &val)
		{
			constexpr auto type = udm::type_to_enum<T>();
			auto *prop = m_inputDescriptor.FindProperty(key);
			if(!prop)
				return false;
			auto *ptr = data.data() + prop->offset;
			return udm::visit_ng(pragma::shadergraph::to_udm_type(prop->parameter.type), [prop, ptr, &val](auto tag) -> bool {
				using TProp = typename decltype(tag)::type;
				if constexpr(udm::is_convertible<T, TProp>()) {
					auto convVal = udm::convert<T, TProp>(val);
					memcpy(ptr, &convVal, sizeof(convVal));
					return true;
				}
				return false;
			});
		}

		std::vector<uint8_t> data;
	  private:
		const ShaderInputDescriptor &m_inputDescriptor;
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
REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::shader_material::Property::Flags)

#endif
