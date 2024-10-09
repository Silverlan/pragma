/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "pragma/rendering/shader_material/shader_material.hpp"
#include <pragma/logging.hpp>
#include <cmaterial.h>
#include <util_image.hpp>
#include <mpParser.h>

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
	auto shaderMat = std::make_shared<ShaderMaterial>();
	std::string err;
	if(shaderMat->LoadFromUdmData(udmData->GetAssetData().GetData()["shader_material"], err) == false) {
		LOGGER.error("Failed to load shader material '" + id + "'!");
		return nullptr;
	}
	it->second = shaderMat;
	return shaderMat;
}

static std::unique_ptr<pragma::rendering::shader_material::ShaderMaterialCache> g_shaderMaterialCache {};
pragma::rendering::shader_material::ShaderMaterialCache &pragma::rendering::shader_material::get_cache()
{
	if(!g_shaderMaterialCache)
		g_shaderMaterialCache = std::make_unique<pragma::rendering::shader_material::ShaderMaterialCache>();
	return *g_shaderMaterialCache;
}
void pragma::rendering::shader_material::clear_cache() { g_shaderMaterialCache = {}; }

//////////

const std::unordered_set<udm::Type> allowedTypes {
  udm::Type::Int16,
  udm::Type::UInt16,
  udm::Type::Int32,
  udm::Type::UInt32,
  udm::Type::Float,
  udm::Type::Half,
  udm::Type::Vector2,
  udm::Type::Vector3,
  udm::Type::Vector4,
  udm::Type::Vector2i,
  udm::Type::Vector3i,
  udm::Type::Vector4i,
};

static std::optional<PropertyValue> get_ds_value(ds::Value &dv, udm::Type type)
{
	auto sz = udm::size_of(type);
	switch(type) {
	case udm::Type::Int16:
	case udm::Type::UInt16:
	case udm::Type::Int32:
	case udm::Type::UInt32:
		{
			return udm::visit<true, false, false>(type, [type, sz, &dv](auto tag) -> std::optional<PropertyValue> {
				using T = typename decltype(tag)::type;
				if constexpr(is_valid_property_type_v<T>) {
					if constexpr(!std::is_same_v<T, udm::Half>)
						return PropertyValue {static_cast<T>(dv.GetInt())};
				}
				return {};
			});
		}
	case udm::Type::Float:
		{
			auto val = static_cast<udm::Float>(dv.GetFloat());
			return PropertyValue {val};
		}
	case udm::Type::Half:
		{
			auto half = static_cast<udm::Half>(dv.GetFloat());
			return PropertyValue {half};
		}
	case udm::Type::Vector2:
		{
			auto val = static_cast<udm::Vector2>(dv.GetVector2());
			return PropertyValue {val};
		}
	case udm::Type::Vector3:
		{
			auto val = static_cast<udm::Vector3>(dv.GetVector());
			return PropertyValue {val};
		}
	case udm::Type::Vector4:
		{
			auto val = static_cast<udm::Vector4>(dv.GetVector4());
			return PropertyValue {val};
		}
	case udm::Type::Vector2i:
		{
			auto val = static_cast<udm::Vector2i>(dv.GetVector2());
			return PropertyValue {val};
		}
	case udm::Type::Vector3i:
		{
			auto val = static_cast<udm::Vector3i>(dv.GetVector());
			return PropertyValue {val};
		}
	case udm::Type::Vector4i:
		{
			auto val = static_cast<udm::Vector4i>(dv.GetVector4());
			return PropertyValue {val};
		}
	default:
		throw std::logic_error {"Unsupported shader material property type'" + std::string {magic_enum::enum_name(type)} + "'!"};
	}
	return {};
}

pragma::rendering::shader_material::Property::Property(const Property &other) { operator=(other); }
pragma::rendering::shader_material::Property &pragma::rendering::shader_material::Property::operator=(const Property &other)
{
	type = other.type;
	specializationType = other.specializationType;
	name = other.name;
	defaultValue = other.defaultValue;
	offset = other.offset;
	padding = other.padding;
	options = other.options ? std::make_unique<std::unordered_map<std::string, PropertyValue>>(*other.options) : std::unique_ptr<std::unordered_map<std::string, PropertyValue>> {};
	flags = other.flags ? std::make_unique<std::unordered_map<std::string, uint32_t>>(*other.flags) : std::unique_ptr<std::unordered_map<std::string, uint32_t>> {};
#ifdef _WIN32
	static_assert(sizeof(*this) == 72);
#endif
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

static std::string get_property_name(const std::string &name) { return ustring::to_camel_case(name); }

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

static std::string get_composite_property_name(const Property &a, const Property &b) { return get_property_name(a.name) + "_" + get_property_name(b.name); }

static std::string value_to_glsl_string(const pragma::rendering::shader_material::PropertyValue &value, udm::Type type)
{
	return udm::visit_ng(type, [&value, type](auto tag) -> std::string {
		using T = typename decltype(tag)::type;
		if constexpr(is_valid_property_type_v<T>) {
			if constexpr(udm::is_vector_type<T>)
				return vec_to_string(get_gl_type_name(type), &std::get<T>(value)[0], T::length());
			else
				return std::to_string(std::get<T>(value));
		}
		return {};
	});
}

std::string ShaderMaterial::ToGlslStruct() const
{
	std::stringstream ss;
	ss << "struct MaterialData {\n";
	for(auto it = properties.begin(); it != properties.end();) {
		auto &prop = *it;
		std::string glTypeName;
		std::string propName;
		if(is_half_type(prop.type)) {
			auto itNext = it + 1;
			assert(itNext != properties.end());
			if(itNext == properties.end())
				throw std::runtime_error {"Property following a half-type must be another half-type!"};
			glTypeName = get_gl_type_name(udm::Type::UInt32);
			propName = get_composite_property_name(prop, *itNext);
			++it;
		}
		else {
			glTypeName = get_gl_type_name(prop.type);
			propName = get_property_name(prop.name);
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

		auto idName = ustring::get_upper(prop.name);
		ss << "#define MATERIAL_PROP_" << idName << "_ENABLED 1\n";

		auto methodName = "get_mat_" + std::string {prop.name};
		auto propName = get_property_name(prop.name);
		auto matPropName = "u_material.material." + propName;
		if(!is_half_type(prop.type)) {
			auto glTypeName = get_gl_type_name(prop.type);
			ss << glTypeName << " " << methodName << "() {return ";
			ss << matPropName;
			ss << ";}\n";
		}
		else {
			auto glTypeName = get_gl_type_name((prop.type == udm::Type::Half) ? udm::Type::Float : udm::Type::UInt32);
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
			if(prop.type == udm::Type::Half)
				strValue = "unpackHalf2x16(" + strValue + ").x";
			ss << strValue;
			ss << ";}\n";
		}
		if(prop.options) {
			for(auto &[optName, optVal] : *prop.options) {
				std::string defName = "MAT_" + ustring::get_upper(ustring::to_snake_case(prop.name)) + "_" + ustring::get_upper(ustring::to_snake_case(std::string {optName}));
				definitions << "#define " << defName << " " << value_to_glsl_string(optVal, prop.type) << "\n";
			}
		}
		if(prop.flags) {
			for(auto &[flagName, flagVal] : *prop.flags) {
				std::string defName = "FMAT_" + ustring::get_upper(ustring::to_snake_case(prop.name)) + "_" + ustring::get_upper(ustring::to_snake_case(std::string {flagName}));
				definitions << "#define " << defName << " " << value_to_glsl_string(flagVal, prop.type) << "\n";
			}
		}
		++it;
	}

	ss << definitions.str();
	ss << "bool is_mat_flag_set(uint flag) {return ((u_material.material.flags & flag) != 0) ? true : false;}\n";

	ss << "#include \"/common/alpha_mode.glsl\"\n";
	uint32_t isrgb = 0;
	for(auto &tex : textures) {
		auto idName = ustring::get_upper(tex.name);
		auto varName = ustring::to_camel_case(tex.name);
		if(!varName.empty())
			varName[0] = tolower(varName[0]);
		varName = "u_" + varName;

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

ShaderMaterial::ShaderMaterial()
{
	properties.reserve(PREDEFINED_PROPERTY_COUNT);

	{
		Property propColor {};
		propColor.name = "color_factor";
		propColor.type = udm::Type::Vector3;
		propColor.defaultValue = Vector3 {1.f, 1.f, 1.f};
		AddProperty(std::move(propColor));
	}

	{
		Property propAlphaFactor {};
		propAlphaFactor.name = "alpha_factor";
		propAlphaFactor.type = udm::Type::Float;
		propAlphaFactor.defaultValue = 1.f;
		AddProperty(std::move(propAlphaFactor));
	}

	{
		Property propAlphaMode {};
		propAlphaMode.name = "alpha_mode";
		propAlphaMode.type = udm::Type::UInt32;
		propAlphaMode.defaultValue = static_cast<uint32_t>(AlphaMode::Opaque);

		std::unordered_map<std::string, PropertyValue> options {
		  {"Opaque", static_cast<uint32_t>(AlphaMode::Opaque)},
		  {"Mask", static_cast<uint32_t>(AlphaMode::Mask)},
		  {"Blend", static_cast<uint32_t>(AlphaMode::Blend)},
		};
		propAlphaMode.options = std::make_unique<decltype(options)>(std::move(options));
		AddProperty(std::move(propAlphaMode));
	}

	{
		Property propCutoff {};
		propCutoff.name = "alpha_cutoff";
		propCutoff.type = udm::Type::Float;
		propCutoff.defaultValue = static_cast<float>(0.5f);
		AddProperty(std::move(propCutoff));
	}

	{
		Property propFlags {};
		propFlags.name = "flags";
		propFlags.type = udm::Type::UInt32;
		propFlags.defaultValue = static_cast<uint32_t>(MaterialFlags::None);
		auto names = magic_enum::flags::enum_names<MaterialFlags>();
		auto values = magic_enum::flags::enum_values<MaterialFlags>();
		std::unordered_map<std::string, uint32_t> flags;
		flags.reserve(names.size());
		for(size_t i = 0; i < names.size(); ++i) {
			auto &name = names[i];
			auto &val = values[i];
			flags[std::string {name}] = umath::to_integral(val);
		}
		propFlags.flags = std::make_unique<std::unordered_map<std::string, uint32_t>>(std::move(flags));
		AddProperty(std::move(propFlags));
	}

	// Placeholder for vec4 alignment
	{
		Property propPlaceholder {};
		propPlaceholder.name = "placeholder";
		propPlaceholder.type = udm::Type::Float;
		propPlaceholder.defaultValue = 0.5f;
		AddProperty(std::move(propPlaceholder));
	}
	assert(properties.size() == PREDEFINED_PROPERTY_COUNT);
	if(properties.size() != PREDEFINED_PROPERTY_COUNT)
		throw std::logic_error {"Number of predefined properties does not match expected count of " + std::to_string(PREDEFINED_PROPERTY_COUNT) + "!"};
}

void ShaderMaterial::AddProperty(Property &&prop)
{
	size_t offset = 0;
	if(!properties.empty()) {
		auto &lastProp = properties.back();
		offset = lastProp.offset + udm::size_of(lastProp.type);
	}
	properties.push_back(std::move(prop));
	properties.back().offset = offset;
}

MaterialFlags ShaderMaterialData::GetFlags() const { return static_cast<MaterialFlags>(*GetValue<uint32_t>("flags")); }
void ShaderMaterialData::SetFlags(MaterialFlags flags) { SetValue<uint32_t>("flags", static_cast<uint32_t>(flags)); }

static void to_srgb_color(Vector3 &col) { uimg::linear_to_srgb(col); }
static void to_srgb_color(Vector4 &col) { col = Vector4 {uimg::linear_to_srgb(reinterpret_cast<Vector3 &>(col)), col.w}; }

static std::optional<PropertyValue> parse_flags_expression(const std::string &strVal, const std::unordered_map<std::string, uint32_t> &flags, udm::Type propType)
{
	mup::ParserX parser;
	for(auto &[flagName, flagVal] : flags)
		parser.DefineConst(_T(flagName), mup::int_type {flagVal});
	parser.SetExpr(strVal);
	try {
		auto &mupVal = parser.Eval();
		return udm::visit_ng(propType, [&mupVal](auto tag) -> std::optional<PropertyValue> {
			using T = typename decltype(tag)::type;
			if constexpr(is_valid_property_type_v<T> && udm::is_numeric_type(udm::type_to_enum<T>()) && !std::is_same<T, udm::Half>())
				return static_cast<T>(mupVal.GetInteger());
			return {};
		});
	}
	catch(const mup::ParserError &e) {
		LOGGER.error("Failed to parse property value '{}': {}", strVal, e.GetMsg());
		return {};
	}
	return {};
}

void ShaderMaterialData::DebugPrint()
{
	std::stringstream ss;
	for(auto &prop : m_shaderMaterial.properties) {
		ss << "$" << udm::enum_type_to_ascii(prop.type) << " " << prop.name << " ";
		udm::visit_ng(prop.type, [this, &prop, &ss](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(is_valid_property_type_v<T>) {
				auto val = GetValue<T>(prop.name.c_str());
				if(!val)
					ss << "NULL";
				else
					ss << udm::convert<T, udm::String>(*val);
			}
			else
				ss << "UNSUPPORTED";
		});
		ss << "\n";
	}
	Con::cout << "Material Data:" << Con::endl;
	Con::cout << ss.str() << Con::endl;
}

void ShaderMaterialData::PopulateFromMaterial(const CMaterial &mat)
{
	auto &matData = mat.GetDataBlock();
	uint8_t *dataPtr = data.data();
	size_t totalSize = 0;
	std::optional<uint32_t> compositeVal {};
	for(auto it = m_shaderMaterial.properties.begin(); it != m_shaderMaterial.properties.end();) {
		auto &prop = *it;
		auto valSize = prop.GetSize();
		totalSize += valSize + prop.padding;
		if(totalSize > MAX_MATERIAL_SIZE)
			throw std::runtime_error {"Size of shader material properties (" + util::get_pretty_bytes(totalSize) + " exceeds maximum allowed size of " + util::get_pretty_bytes(MAX_MATERIAL_SIZE) + "!"};

		auto val = prop.defaultValue;
		auto matVal = matData->GetDataValue(prop.name);
		if(matVal) {
			auto handled = false;
			if(matVal->GetTypeString() == "string") {
				if(prop.options) {
					auto strVal = matVal->GetString();
					auto it = prop.options->find(strVal);
					if(it != prop.options->end())
						val = it->second;
					else
						LOGGER.error("Invalid property value '{}' for property '{}'", strVal, prop.name.str);
					handled = true;
				}
				else if(prop.flags) {
					auto evalVal = parse_flags_expression(matVal->GetString(), *prop.flags, prop.type);
					if(evalVal)
						val = *evalVal;
					handled = true;
				}
			}
			if(!handled) {
				auto dsVal = get_ds_value(*matVal, prop.type);
				if(dsVal) {
					val = *dsVal;
					if(prop.specializationType && *prop.specializationType == "color") {
						switch(prop.type) {
						case udm::Type::Vector3:
							to_srgb_color(std::get<udm::Vector3>(val));
							break;
						case udm::Type::Vector4:
							to_srgb_color(std::get<udm::Vector4>(val));
							break;
						default:
							throw std::runtime_error {"Invalid property type '" + std::string {magic_enum::enum_name(prop.type)} + "' for specialization type '" + std::string {*prop.specializationType} + "'!"};
						}
					}
				}
			}
		}

		if(is_half_type(prop.type)) {
			auto encoded = udm::visit_ng(prop.type, [&prop, &val, &compositeVal](auto tag) -> uint16_t {
				using T = typename decltype(tag)::type;
				constexpr auto udmType = udm::type_to_enum<T>();
				if constexpr(is_half_type(udmType)) {
					if constexpr(udmType == udm::Type::Half) {
						auto &half = std::get<T>(val);
						return half.value;
					}
					else
						return std::get<T>(val);
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
			udm::visit_ng(prop.type, [&prop, dataPtr, &val](auto tag) {
				using T = typename decltype(tag)::type;
				if constexpr(is_valid_property_type_v<T>) {
					auto &tval = std::get<T>(val);
					memcpy(dataPtr, &tval, sizeof(tval));
				}
			});
			dataPtr += valSize + prop.padding;
		}

		++it;
	}

	auto flags = GetFlags();
	if(matData->GetBool("black_to_alpha") == true)
		flags |= MaterialFlags::BlackToAlpha;
	SetFlags(flags);
}

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
			auto it = allowedTypes.find(udmType);
			if(it == allowedTypes.end()) {
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
			std::optional<PropertyValue> defVal {};
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
				defVal = udm::visit_ng(udmType, [&udmDefault](auto tag) -> std::optional<PropertyValue> {
					using T = typename decltype(tag)::type;
					if constexpr(is_valid_property_type_v<T>) {
						auto val = udmDefault.ToValue<T>();
						if(!val)
							return {};
						return PropertyValue {*val};
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

			Property matProp {};
			matProp.name = std::move(name);
			matProp.type = udmType;
			matProp.defaultValue = std::move(*defVal);
			matProp.flags = std::move(flags);
			totalSize += udm::size_of(udmType);

			std::string specializationType;
			if(prop["specializationType"] >> specializationType)
				matProp.specializationType = specializationType;

			auto udmOptions = prop["options"];
			if(udmOptions) {
				matProp.options = std::make_unique<std::unordered_map<std::string, PropertyValue>>();
				auto &options = *matProp.options;
				options.reserve(udmOptions.GetSize());
				for(size_t i = 0; auto &udmOpt : udmOptions) {
					auto name = udmOpt.GetValue<std::string>();
					udm::visit<true, false, false>(udmType, [&name, &options, i](auto tag) {
						using T = typename decltype(tag)::type;
						if constexpr(!std::is_same_v<T, udm::Half> && is_valid_property_type_v<T>)
							options[name] = static_cast<T>(i);
					});
					++i;
				}
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
		}

		textures.push_back({});
		auto &shaderTex = textures.back();
		shaderTex.name = std::move(name);

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
