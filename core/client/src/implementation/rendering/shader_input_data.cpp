// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <mpParser.h>

module pragma.client;

import :rendering.shader_input_data;
using namespace pragma::rendering;

static spdlog::logger &LOGGER = pragma::register_logger("shader");

void ShaderInputData::ResizeToDescriptor()
{
	if(m_inputDescriptor.properties.empty())
		return;
	auto &lastProp = m_inputDescriptor.properties.back();
	auto offset = data.size();
	data.resize(lastProp.offset + udm::size_of(pragma::shadergraph::to_udm_type(lastProp.parameter.type)));
	size_t idx = m_inputDescriptor.properties.size() - 1;
	for(auto it = m_inputDescriptor.properties.rbegin(); it != m_inputDescriptor.properties.rend(); ++it) {
		auto &prop = *it;
		if(prop.offset < offset)
			break;
		ResetToDefault(idx);
		--idx;
	}
}

void ShaderInputData::ResetToDefault(size_t idx)
{
	auto &prop = m_inputDescriptor.properties[idx];
	udm::visit_ng(pragma::shadergraph::to_udm_type(prop.parameter.type), [this, &prop](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(shadergraph::is_data_type<T>()) {
			T val {};
			prop.parameter.defaultValue.Get<T>(val);
			SetValue<T>(prop.parameter.name.c_str(), val);
		}
	});
}

void ShaderInputData::ResetToDefault()
{
	for(size_t i = 0; i < m_inputDescriptor.properties.size(); ++i)
		ResetToDefault(i);
}

void ShaderInputData::DebugPrint()
{
	std::stringstream ss;
	for(auto &prop : m_inputDescriptor.properties) {
		ss << "$" << udm::enum_type_to_ascii(pragma::shadergraph::to_udm_type(prop.parameter.type)) << " " << prop.parameter.name << " ";
		udm::visit_ng(pragma::shadergraph::to_udm_type(prop.parameter.type), [this, &prop, &ss](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(shadergraph::is_data_type<T>()) {
				auto val = GetValue<T>(prop.parameter.name.c_str());
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
	Con::COUT << "Material Data:" << Con::endl;
	Con::COUT << ss.str() << Con::endl;
}

// ShaderInputDescriptor

std::optional<pragma::shadergraph::Value> ShaderInputDescriptor::parse_flags_expression(const std::string &strVal, const std::unordered_map<std::string, uint32_t> &flags, udm::Type propType)
{
	mup::ParserX parser;
	for(auto &[flagName, flagVal] : flags)
		parser.DefineConst(_T(flagName), mup::int_type {flagVal});
	parser.SetExpr(strVal);
	try {
		auto &mupVal = parser.Eval();
		return udm::visit_ng(propType, [&mupVal](auto tag) -> std::optional<shadergraph::Value> {
			using T = typename decltype(tag)::type;
			if constexpr(shadergraph::is_data_type<T>() && udm::is_numeric_type(udm::type_to_enum<T>()) && !std::is_same<T, udm::Half>())
				return shadergraph::Value::Create<T>(static_cast<T>(mupVal.GetInteger()));
			return {};
		});
	}
	catch(const mup::ParserError &e) {
		LOGGER.error("Failed to parse property value '{}': {}", strVal, e.GetMsg());
		return {};
	}
	return {};
}

ShaderInputDescriptor::ShaderInputDescriptor(const GString &name) : name {name} {}

bool ShaderInputDescriptor::AddProperty(Property &&prop)
{
	auto it = m_propertyMap.find(prop.parameter.name);
	if(it != m_propertyMap.end())
		return false; // TODO: Update min, max, etc.
	size_t offset = 0;
	if(!properties.empty()) {
		auto &lastProp = properties.back();
		offset = lastProp.offset + udm::size_of(pragma::shadergraph::to_udm_type(lastProp.parameter.type));
	}
	properties.push_back(std::move(prop));
	properties.back().offset = offset;
	m_propertyMap[properties.back().parameter.name] = properties.size() - 1;
	return true;
}

bool ShaderInputDescriptor::LoadFromUdmData(udm::LinkedPropertyWrapperArg prop, std::string &outErr)
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
			auto *base = Import(name);
			if(!base) {
				outErr = "Failed to import shader input data '" + name + "'!";
				return false;
			}
			properties.reserve(properties.size() + base->properties.size());
			for(size_t i = 0; i < base->properties.size(); ++i) {
				auto baseProp = base->properties.at(i);
				AddProperty(std::move(baseProp));
			}
		}
		else {
			auto udmType = udm::ascii_type_to_enum(type);
			auto shType = shadergraph::to_data_type(udmType);
			if(shType == shadergraph::DataType::Invalid) {
				outErr = "Type '" + type + "' is not a valid shader input property type!";
				return false;
			}

			if(name.empty()) {
				outErr = "Shader input property name must not be empty!";
				return false;
			}

			auto udmDefault = prop["default"];
			if(!udmDefault) {
				outErr = "Shader input property '" + name + "' has no default value!";
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
	return true;
}
