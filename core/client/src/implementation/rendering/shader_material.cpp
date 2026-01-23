// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <mpParser.h>

module pragma.client;

import :rendering.shader_material;
static spdlog::logger &LOGGER = pragma::register_logger("shader_material");

using namespace pragma::rendering::shader_material;
ShaderMaterialCache::ShaderMaterialCache() {}
std::shared_ptr<ShaderMaterial> ShaderMaterialCache::Load(const std::string &id)
{
	auto it = m_cache.find(id);
	if(it != m_cache.end())
		return it->second;
	it = m_cache.insert(std::make_pair(id, nullptr)).first;

	std::shared_ptr<udm::Data> udmData {};
	std::string shaderMaterialPath = "scripts/shader_data/materials/" + id + ".udm";
	try {
		udmData = udm::Data::Load(shaderMaterialPath);
	}
	catch(const udm::Exception &e) {
		LOGGER.error("Failed to load shader material '{}': {}!", shaderMaterialPath, e.what());
		return nullptr;
	}
	if(!udmData)
		return nullptr;
	auto shaderMat = pragma::util::make_shared<ShaderMaterial>(id);
	std::string err;
	if(shaderMat->LoadFromUdmData(udmData->GetAssetData().GetData()["shader_material"], err) == false) {
		LOGGER.error("Failed to load shader material '" + id + "'!");
		return nullptr;
	}
	it->second = shaderMat;
	return shaderMat;
}

std::shared_ptr<ShaderMaterial> ShaderMaterialCache::Get(const std::string &id) const
{
	auto it = m_cache.find(id);
	if(it == m_cache.end())
		return nullptr;
	return it->second;
}

static std::unique_ptr<ShaderMaterialCache> g_shaderMaterialCache {};
ShaderMaterialCache &pragma::rendering::shader_material::get_cache()
{
	if(!g_shaderMaterialCache)
		g_shaderMaterialCache = std::make_unique<ShaderMaterialCache>();
	return *g_shaderMaterialCache;
}
void pragma::rendering::shader_material::clear_cache() { g_shaderMaterialCache = {}; }

//////////

static std::optional<pragma::shadergraph::Value> get_ds_value(const pragma::material::Material &mat, const std::string_view &propertyPath, udm::Type type)
{
	auto sz = udm::size_of(type);
	switch(type) {
	case udm::Type::Int16:
	case udm::Type::UInt16:
	case udm::Type::Int32:
	case udm::Type::UInt32:
		{
			return udm::visit<true, false, false>(type, [type, sz, &mat, &propertyPath](auto tag) -> std::optional<pragma::shadergraph::Value> {
				using T = typename decltype(tag)::type;
				if constexpr(pragma::shadergraph::is_data_type_v<T>) {
					T val;
					if(mat.GetProperty(propertyPath, &val))
						return pragma::shadergraph::Value::Create(val);
				}
				return {};
			});
		}
	case udm::Type::Float:
		{
			float val;
			if(mat.GetProperty(propertyPath, &val))
				return pragma::shadergraph::Value::Create(val);
			return {};
		}
	case udm::Type::Half:
		{
			udm::Half val;
			if(mat.GetProperty(propertyPath, &val))
				return pragma::shadergraph::Value::Create(val);
			return {};
		}
	case udm::Type::Vector2:
		{
			Vector2 val;
			if(mat.GetProperty(propertyPath, &val))
				return pragma::shadergraph::Value::Create(val);
			return {};
		}
	case udm::Type::Vector3:
		{
			Vector3 val;
			if(mat.GetProperty(propertyPath, &val))
				return pragma::shadergraph::Value::Create(val);
			return {};
		}
	case udm::Type::Vector4:
		{
			Vector4 val;
			if(mat.GetProperty(propertyPath, &val))
				return pragma::shadergraph::Value::Create(val);
			return {};
		}
	default:
		throw std::logic_error {"Unsupported shader material property type'" + std::string {magic_enum::enum_name(type)} + "'!"};
	}
	return {};
}

pragma::rendering::Property::Property(const std::string &name, shadergraph::DataType type) : parameter {name, type} {}
pragma::rendering::Property::Property(const Property &other) : parameter {other.parameter} { operator=(other); }
pragma::rendering::Property &pragma::rendering::Property::operator=(const Property &other)
{
	parameter = other.parameter;
	specializationType = other.specializationType;
	propertyFlags = other.propertyFlags;
	offset = other.offset;
	padding = other.padding;
	flags = other.flags ? std::make_unique<std::unordered_map<std::string, uint32_t>>(*other.flags) : std::unique_ptr<std::unordered_map<std::string, uint32_t>> {};
	return *this;
}

static std::string get_gl_type_name(udm::Type type)
{
	switch(type) {
	case udm::Type::Int32:
		return "int";
	case udm::Type::UInt32:
		return "uint";
	case udm::Type::Float:
		return "float";
	case udm::Type::Vector2:
		return "vec2";
	case udm::Type::Vector3:
		return "vec3";
	case udm::Type::Vector4:
		return "vec4";
	case udm::Type::Vector2i:
		return "ivec2";
	case udm::Type::Vector3i:
		return "ivec3";
	case udm::Type::Vector4i:
		return "ivec4";
	}
	throw std::logic_error {"UDM type '" + std::string {magic_enum::enum_name(type)} + "' is not supported!"};
	return {};
}

static std::string get_property_name(const std::string &name) { return pragma::string::to_camel_case(name); }

template<typename T>
static std::string vec_to_string(const std::string &typeName, T *baseVal, size_t numComponents)
{
	std::string str = typeName + "(";
	for(size_t i = 0; i < numComponents; ++i) {
		if(i > 0)
			str += ",";
		str += std::to_string(*baseVal);
		++baseVal;
	}
	str += ")";
	return str;
}

constexpr bool is_half_type(udm::Type type)
{
	switch(type) {
	case udm::Type::Half:
	case udm::Type::Int16:
	case udm::Type::UInt16:
		return true;
	}
	return false;
}

static std::string get_composite_property_name(const pragma::rendering::Property &a, const pragma::rendering::Property &b) { return get_property_name(a.parameter.name) + "_" + get_property_name(b.parameter.name); }

static std::string value_to_glsl_string(const pragma::shadergraph::Value &value, udm::Type type)
{
	return udm::visit_ng(type, [&value, type](auto tag) -> std::string {
		using T = typename decltype(tag)::type;
		if constexpr(pragma::shadergraph::is_data_type<T>()) {
			T tval;
			value.Get(tval);
			return pragma::shadergraph::to_glsl_value<T>(tval);
		}
		return {};
	});
}

std::string ShaderMaterial::GetTextureUniformVariableName(const std::string &texIdentifier)
{
	auto varName = string::to_camel_case(texIdentifier);
	if(!varName.empty())
		varName[0] = tolower(varName[0]);
	varName = "u_" + varName;
	return varName;
}

std::string ShaderMaterial::ToGlslStruct() const
{
	std::stringstream ss;
	ss << "struct MaterialData {\n";
	for(auto it = properties.begin(); it != properties.end();) {
		auto &prop = *it;
		std::string glTypeName;
		std::string propName;
		if(is_half_type(pragma::shadergraph::to_udm_type(prop.parameter.type))) {
			auto itNext = it + 1;
			assert(itNext != properties.end());
			if(itNext == properties.end())
				throw std::runtime_error {"Property following a half-type must be another half-type!"};
			glTypeName = get_gl_type_name(udm::Type::UInt32);
			propName = get_composite_property_name(prop, *itNext);
			++it;
		}
		else {
			glTypeName = get_gl_type_name(pragma::shadergraph::to_udm_type(prop.parameter.type));
			propName = get_property_name(prop.parameter.name);
		}
		ss << "\t" << glTypeName << " " << propName << ";\n";
		++it;
	}
	ss << "};\n";

	ss << "layout(std140, LAYOUT_ID(MATERIAL, SETTINGS)) uniform Material { MaterialData material; } u_material;\n";

	std::stringstream definitions;
	std::optional<std::string> compositePropName {};
	for(auto it = properties.begin(); it != properties.end();) {
		auto &prop = *it;

		auto idName = string::get_upper(prop.parameter.name);
		ss << "#define MATERIAL_PROP_" << idName << "_ENABLED 1\n";

		auto methodName = "get_mat_" + std::string {prop.parameter.name};
		auto propName = get_property_name(prop.parameter.name);
		auto matPropName = "u_material.material." + propName;
		if(!is_half_type(pragma::shadergraph::to_udm_type(prop.parameter.type))) {
			auto glTypeName = get_gl_type_name(pragma::shadergraph::to_udm_type(prop.parameter.type));
			ss << glTypeName << " " << methodName << "() {return ";
			ss << matPropName;
			ss << ";}\n";
		}
		else {
			auto glTypeName = get_gl_type_name((pragma::shadergraph::to_udm_type(prop.parameter.type) == udm::Type::Half) ? udm::Type::Float : udm::Type::UInt32);
			ss << glTypeName << " " << methodName << "() {return ";
			std::string strValue;
			if(!compositePropName) {
				auto itNext = it + 1;
				assert(itNext != properties.end());
				if(itNext == properties.end())
					throw std::runtime_error {"Property following a half-type must be another half-type!"};
				compositePropName = "u_material.material." + get_composite_property_name(prop, *itNext);
				strValue = "(" + *compositePropName + " & 0xFFFFu)";
			}
			else {
				strValue = "((" + *compositePropName + ">>16) & 0xFFFFu)";
				compositePropName = {};
			}
			if(pragma::shadergraph::to_udm_type(prop.parameter.type) == udm::Type::Half)
				strValue = "unpackHalf2x16(" + strValue + ").x";
			ss << strValue;
			ss << ";}\n";
		}
		if(prop->enumSet) {
			for(auto &[optName, optVal] : prop->enumSet->getNameToValue()) {
				std::string defName = "MAT_" + string::get_upper(string::to_snake_case(prop.parameter.name)) + "_" + string::get_upper(string::to_snake_case(std::string {optName}));
				definitions << "#define " << defName << " " << optVal << "\n";
			}
		}
		if(prop.flags) {
			for(auto &[flagName, flagVal] : *prop.flags) {
				std::string defName = "FMAT_" + string::get_upper(string::to_snake_case(prop.parameter.name)) + "_" + string::get_upper(string::to_snake_case(std::string {flagName}));
				definitions << "#define " << defName << " " << flagVal << "\n";
			}
		}
		++it;
	}

	ss << definitions.str();
	ss << "bool is_mat_flag_set(uint flag) {return ((u_material.material.flags & flag) != 0) ? true : false;}\n";

	ss << "#include \"/common/alpha_mode.glsl\"\n";
	uint32_t isrgb = 0;
	for(auto &tex : textures) {
		auto idName = string::get_upper(tex.name);
		auto varName = GetTextureUniformVariableName(tex.name);

		ss << "#define MATERIAL_" << idName << "_ENABLED 1\n";

		auto &funcName = tex.name;
		std::string samplerType;
		std::string uvType;
		if(tex.cubemap) {
			samplerType = "samplerCube";
			uvType = "vec3";
		}
		else {
			samplerType = "sampler2D";
			uvType = "vec2";
		}
		ss << "layout(LAYOUT_ID(MATERIAL, " << idName << ")) uniform " << samplerType << " " << varName << ";\n";
		ss << "vec4 fetch_" << funcName << "(" << uvType << " uv) {\n";
		ss << "\tvec4 val = texture(" << varName << ",uv);\n";
		if(tex.colorMap) {
			if(isrgb >= MAX_NUMBER_OF_SRGB_TEXTURES)
				throw std::runtime_error {"Only up to " + std::to_string(MAX_NUMBER_OF_SRGB_TEXTURES) + "texture definitions with the 'convertToLinearSpace' flag are allowed!"};
			std::string flag = "FMAT_FLAGS_SRGB" + std::to_string(isrgb);
			ss << "\tif(!is_mat_flag_set(" << flag << "))\n";
			ss << "\t\tval.rgb = srgb_to_linear(val.rgb);\n";
			++isrgb;
		}
		ss << "\treturn val;\n";
		ss << "}\n";

		if(tex.colorMap) {
			ss << "vec4 fetch_" << funcName << "(" << uvType << " uv, vec4 instanceColor) {\n";
			ss << "\tvec4 color = fetch_" << funcName << "(uv);\n";
			ss << "\tMaterialData mat = u_material.material;\n";
			ss << "\tuint alphaMode = mat.alphaMode;\n";
			ss << "\tif(is_mat_flag_set(FMAT_FLAGS_TRANSLUCENT)) {\n";
			ss << "\t\tif(instanceColor.a < 1.0)\n";
			ss << "\t\t\talphaMode = MAT_ALPHA_MODE_BLEND;\n";
			ss << "\t\tcolor.a = apply_alpha_mode(color.a * instanceColor.a * mat.alphaFactor, alphaMode, mat.alphaCutoff) * instanceColor.a;\n";
			ss << "\t}\n";
			ss << "\telse\n";
			ss << "\t\tcolor.a = 1.0;\n";
			ss << "\tcolor.rgb *= instanceColor.rgb * mat.colorFactor.rgb;\n";
			ss << "\treturn color;\n";
			ss << "}\n";
		}
	}

	return ss.str();
}

ShaderMaterial::ShaderMaterial(const GString &name) : ShaderInputDescriptor {name}
{
	properties.reserve(PREDEFINED_PROPERTY_COUNT);

	{
		Property propColor {"color_factor", shadergraph::DataType::Vector};
		propColor->defaultValue = Vector3 {1.f, 1.f, 1.f};
		propColor.specializationType = "color";
		AddProperty(std::move(propColor));
	}

	{
		Property propAlphaFactor {"alpha_factor", shadergraph::DataType::Float};
		propAlphaFactor->defaultValue = 1.f;
		propAlphaFactor->SetRange(0.f, 1.f);
		AddProperty(std::move(propAlphaFactor));
	}

	{
		Property propAlphaMode {"alpha_mode", shadergraph::DataType::UInt};
		propAlphaMode->defaultValue = static_cast<uint32_t>(AlphaMode::Opaque);
		std::unordered_map<std::string, uint32_t> options {
		  {"Opaque", static_cast<uint32_t>(AlphaMode::Opaque)},
		  {"Mask", static_cast<uint32_t>(AlphaMode::Mask)},
		  {"Blend", static_cast<uint32_t>(AlphaMode::Blend)},
		};
		propAlphaMode->SetEnumValues(options);
		AddProperty(std::move(propAlphaMode));
	}

	{
		Property propCutoff {"alpha_cutoff", shadergraph::DataType::Float};
		propCutoff->defaultValue = static_cast<float>(0.5f);
		propCutoff->SetRange(0.f, 1.f);
		AddProperty(std::move(propCutoff));
	}

	{
		Property propFlags {"flags", shadergraph::DataType::UInt};
		propFlags->defaultValue = static_cast<uint32_t>(MaterialFlags::None);
		propFlags.propertyFlags |= Property::Flags::HideInEditor;
		auto names = magic_enum::enum_names<MaterialFlags>();
		auto values = magic_enum::enum_values<MaterialFlags>();
		std::unordered_map<std::string, uint32_t> flags;
		flags.reserve(names.size());
		for(size_t i = 0; i < names.size(); ++i) {
			auto &name = names[i];
			auto &val = values[i];
			flags[std::string {name}] = math::to_integral(val);
		}
		propFlags.flags = std::make_unique<std::unordered_map<std::string, uint32_t>>(std::move(flags));
		AddProperty(std::move(propFlags));
	}

	// Placeholder for vec4 alignment
	{
		Property propPlaceholder {"placeholder", shadergraph::DataType::Float};
		propPlaceholder->defaultValue = 0.5f;
		propPlaceholder.propertyFlags |= Property::Flags::HideInEditor;
		AddProperty(std::move(propPlaceholder));
	}
	assert(properties.size() == PREDEFINED_PROPERTY_COUNT);
	if(properties.size() != PREDEFINED_PROPERTY_COUNT)
		throw std::logic_error {"Number of predefined properties does not match expected count of " + std::to_string(PREDEFINED_PROPERTY_COUNT) + "!"};
}

pragma::rendering::ShaderInputDescriptor *ShaderMaterial::Import(const std::string &name) { return g_shaderMaterialCache->Load(name).get(); }

static void to_srgb_color(Vector3 &col) { pragma::image::linear_to_srgb(col); }
static void to_srgb_color(Vector4 &col) { col = Vector4 {pragma::image::linear_to_srgb(reinterpret_cast<Vector3 &>(col)), col.w}; }

void ShaderMaterial::PopulateShaderInputDataFromMaterial(ShaderInputData &inputData, const material::CMaterial &mat)
{
	inputData.data.resize(MAX_MATERIAL_SIZE);

	uint8_t *dataPtr = inputData.data.data();
	size_t totalSize = 0;
	std::optional<uint32_t> compositeVal {};
	auto &descriptor = inputData.GetDescriptor();
	for(auto it = descriptor.properties.begin(); it != descriptor.properties.end();) {
		auto &prop = *it;
		auto valSize = prop.GetSize();
		totalSize += valSize + prop.padding;
		if(totalSize > MAX_MATERIAL_SIZE)
			throw std::runtime_error {"Size of shader material properties (" + util::get_pretty_bytes(totalSize) + " exceeds maximum allowed size of " + util::get_pretty_bytes(MAX_MATERIAL_SIZE) + "!"};

		auto val = prop->defaultValue;
		auto matValType = mat.GetPropertyType(prop.parameter.name);
		if(matValType != material::PropertyType::None && matValType != material::PropertyType::Block) {
			auto handled = false;
			if(mat.GetPropertyValueType(prop.parameter.name) == datasystem::ValueType::String) {
				std::string strVal;
				mat.GetProperty(prop.parameter.name, &strVal);
				if(prop->enumSet) {
					auto ival = prop->enumSet->findValue(strVal);
					if(ival)
						val = *ival;
					else
						LOGGER.error("Invalid property value '{}' for property '{}'", strVal, prop->name);
					handled = true;
				}
				else if(prop.flags) {
					auto evalVal = parse_flags_expression(strVal, *prop.flags, pragma::shadergraph::to_udm_type(prop.parameter.type));
					if(evalVal)
						val = *evalVal;
					handled = true;
				}
			}
			if(!handled) {
				auto dsVal = get_ds_value(mat, prop.parameter.name, pragma::shadergraph::to_udm_type(prop.parameter.type));
				if(dsVal) {
					val = *dsVal;
					if(prop.specializationType && *prop.specializationType == "color") {
						switch(pragma::shadergraph::to_udm_type(prop.parameter.type)) {
						case udm::Type::Vector3:
							{
								Vector3 col {};
								val.Get<Vector3>(col);
								to_srgb_color(col);
								val.Set<Vector3>(col);
								break;
							}
						case udm::Type::Vector4:
							{
								Vector4 col {};
								val.Get<Vector4>(col);
								to_srgb_color(col);
								val.Set<Vector4>(col);
								break;
							}
						default:
							throw std::runtime_error {"Invalid property type '" + std::string {magic_enum::enum_name(pragma::shadergraph::to_udm_type(prop.parameter.type))} + "' for specialization type '" + std::string {*prop.specializationType} + "'!"};
						}
					}
				}
			}
		}

		if(is_half_type(pragma::shadergraph::to_udm_type(prop.parameter.type))) {
			auto encoded = udm::visit_ng(pragma::shadergraph::to_udm_type(prop.parameter.type), [&prop, &val, &compositeVal](auto tag) -> uint16_t {
				using T = typename decltype(tag)::type;
				constexpr auto udmType = udm::type_to_enum<T>();
				if constexpr(is_half_type(udmType)) {
					if constexpr(udmType == udm::Type::Half) {
						T half {};
						val.Get<T>(half);
						return half.value;
					}
					else {
						T tval {};
						val.Get<T>(tval);
						return tval;
					}
				}
				return 0;
			});
			if(compositeVal) {
				*compositeVal |= static_cast<uint32_t>(encoded) << 16;
				memcpy(dataPtr, &*compositeVal, sizeof(*compositeVal));
				dataPtr += sizeof(*compositeVal) + prop.padding;
				compositeVal = {};
			}
			else {
				compositeVal = 0;
				*compositeVal |= encoded;
				assert(prop.padding == 0);
			}
		}
		else {
			if(compositeVal)
				throw std::runtime_error {"Property following a half-type must be another half-type!"};
			udm::visit_ng(pragma::shadergraph::to_udm_type(prop.parameter.type), [&prop, dataPtr, &val](auto tag) {
				using T = typename decltype(tag)::type;
				if constexpr(shadergraph::is_data_type<T>()) {
					T tval;
					val.Get<T>(tval);
					memcpy(dataPtr, &tval, sizeof(tval));
				}
			});
			dataPtr += valSize + prop.padding;
		}

		++it;
	}

	auto flags = GetFlagsFromShaderInputData(inputData);
	if(mat.GetProperty("black_to_alpha", false))
		flags |= MaterialFlags::BlackToAlpha;
	SetShaderInputDataFlags(inputData, flags);
}
MaterialFlags ShaderMaterial::GetFlagsFromShaderInputData(const ShaderInputData &inputData) { return static_cast<MaterialFlags>(*inputData.GetValue<uint32_t>("flags")); }
void ShaderMaterial::SetShaderInputDataFlags(ShaderInputData &inputData, MaterialFlags flags) { inputData.SetValue<uint32_t>("flags", static_cast<uint32_t>(flags)); }

bool ShaderMaterial::LoadFromUdmData(udm::LinkedPropertyWrapperArg prop, std::string &outErr)
{
	size_t totalSize = 0;
	auto wasPrevPropHalf = false;
	auto udmProps = prop["properties"];
	properties.reserve(properties.size() + udmProps.GetSize());
	for(auto &prop : udmProps) {
		std::string type;
		prop["type"] >> type;
		std::string name;
		prop["name"] >> name;

		if(type == "import") {
			auto base = g_shaderMaterialCache->Load(name);
			if(!base) {
				outErr = "Failed to import shader material '" + name + "'!";
				return false;
			}
			properties.reserve(properties.size() + base->properties.size());
			for(size_t i = PREDEFINED_PROPERTY_COUNT; i < base->properties.size(); ++i) {
				auto baseProp = base->properties.at(i);
				AddProperty(std::move(baseProp));
			}
		}
		else {
			auto udmType = udm::ascii_type_to_enum(type);
			auto shType = shadergraph::to_data_type(udmType);
			if(shType == shadergraph::DataType::Invalid) {
				outErr = "Type '" + type + "' is not a valid material property type!";
				return false;
			}

			if(name.empty()) {
				outErr = "Material property name must not be empty!";
				return false;
			}

			auto udmDefault = prop["default"];
			if(!udmDefault) {
				outErr = "Material property '" + name + "' has no default value!";
				return false;
			}

			std::unique_ptr<std::unordered_map<std::string, uint32_t>> flags {};
			std::optional<shadergraph::Value> defVal {};
			auto udmFlags = prop["flags"];
			if(udmFlags) {
				flags = std::make_unique<std::unordered_map<std::string, uint32_t>>();
				uint32_t flagValue = 1;
				flags->reserve(udmFlags.GetSize());
				for(auto &udmFlag : udmFlags) {
					auto name = udmFlag.GetValue<std::string>();
					(*flags)[name] = flagValue;

					flagValue <<= 1;
				}

				if(udmDefault.GetType() == udm::Type::String) {
					auto &strVal = udmDefault.GetValue<udm::String>();
					defVal = parse_flags_expression(strVal, *flags, udmType);
				}
			}

			if(!defVal) {
				defVal = udm::visit_ng(udmType, [&udmDefault](auto tag) -> std::optional<shadergraph::Value> {
					using T = typename decltype(tag)::type;
					if constexpr(shadergraph::is_data_type<T>()) {
						auto val = udmDefault.ToValue<T>();
						if(!val)
							return {};
						return shadergraph::Value::Create(*val);
					}
					return {};
				});
			}
			if(!defVal) {
				outErr = "Unable to convert default value of property '" + name + "' to specified type '" + std::string {magic_enum::enum_name(udmType)} + "'!";
				return false;
			}

			if(wasPrevPropHalf) {
				auto isValidHalfType = false;
				switch(udmType) {
				case udm::Type::Int16:
				case udm::Type::UInt16:
				case udm::Type::Float:
				case udm::Type::Half:
					isValidHalfType = true;
					break;
				}

				if(!isValidHalfType) {
					outErr = "Property type following a half-property must be a int16 or floating-point type, but type of property '" + name + "' is '" + std::string {magic_enum::enum_name(udmType)} + "'!";
					return false;
				}

				wasPrevPropHalf = false;
			}

			// TODO: Padding!
			if(udmType == udm::Type::Half)
				wasPrevPropHalf = true;

			Property matProp {std::move(name), shadergraph::to_data_type(udmType)};
			matProp->defaultValue = std::move(*defVal);
			matProp.flags = std::move(flags);
			totalSize += udm::size_of(udmType);

			std::string specializationType;
			if(prop["specializationType"] >> specializationType)
				matProp.specializationType = specializationType;

			auto hideInEditor = false;
			if((prop["hideInEditor"] >> hideInEditor) && hideInEditor)
				matProp.propertyFlags |= Property::Flags::HideInEditor;

			auto udmMin = prop["min"];
			auto udmMax = prop["max"];
			if(udmMin || udmMax) {
				if(!udmMin || !udmMax) {
					outErr = "If min has been defined, max has to be defined as well (and vice versa)!";
					return false;
				}

				auto range = udm::visit_ng(udmType, [&udmMin, &udmMax](auto tag) -> std::optional<std::pair<float, float>> {
					using T = typename decltype(tag)::type;
					if constexpr(shadergraph::is_data_type<T>() && std::is_same_v<T, float>) {
						auto min = udmMin.ToValue<T>();
						auto max = udmMax.ToValue<T>();
						if(!min || !max)
							return {};
						return std::pair<float, float> {static_cast<float>(*min), static_cast<float>(*max)};
					}
					return {};
				});
				if(!range) {
					outErr = "Failed to convert min/max values to property type!";
					return {};
				}
				matProp->SetRange(range->first, range->second);
			}

			auto udmOptions = prop["options"];
			if(udmOptions) {
				std::unordered_map<std::string, uint32_t> options;
				options.reserve(udmOptions.GetSize());
				for(size_t i = 0; auto &udmOpt : udmOptions) {
					auto name = udmOpt.GetValue<std::string>();
					udm::visit<true, false, false>(udmType, [&name, &options, i](auto tag) {
						using T = typename decltype(tag)::type;
						if constexpr(!std::is_same_v<T, udm::Half> && shadergraph::is_data_type<T>())
							options[name] = static_cast<T>(i);
					});
					++i;
				}
				matProp->SetEnumValues(options);
			}
			AddProperty(std::move(matProp));
		}
	}

	auto udmTextures = prop["textures"];
	textures.reserve(textures.size() + udmTextures.GetSize());
	for(auto tex : udmTextures) {
		std::string name;
		tex["name"] >> name;
		if(name.empty()) {
			outErr = "Material texture name must not be empty!";
			return false;
		}

		std::string type;
		if((tex["type"] >> type) && type == "import") {
			auto base = g_shaderMaterialCache->Load(name);
			if(!base) {
				outErr = "Failed to import shader material '" + name + "'!";
				return false;
			}
			textures.reserve(textures.size() + base->textures.size());
			for(auto &tex : base->textures)
				textures.push_back(tex);
			continue;
		}

		textures.push_back({});
		auto &shaderTex = textures.back();
		shaderTex.name = std::move(name);

		std::string specializationType;
		if(tex["specializationType"] >> specializationType)
			shaderTex.specializationType = specializationType;

		std::string defaultTexturePath;
		if(tex["default"] >> defaultTexturePath)
			shaderTex.defaultTexturePath = defaultTexturePath;
		tex["cubemap"] >> shaderTex.cubemap;
		tex["required"] >> shaderTex.required;
		tex["colorMap"] >> shaderTex.colorMap;
	}

	if(totalSize > MAX_MATERIAL_SIZE) {
		outErr = "Total size of material properties (" + util::get_pretty_bytes(totalSize) + ") exceeds maximum allowed size of " + util::get_pretty_bytes(MAX_MATERIAL_SIZE) + "!";
		return false;
	}
	return true;
}
