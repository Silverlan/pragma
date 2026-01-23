// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.base_networked;

export import :entities.components.base;

export {
	namespace pragma {
		class DLLNETWORK BaseNetworkedComponent : public BaseEntityComponent {
		  public:
			enum class NetworkFlags : uint32_t { None = 0, ExcludeFromSnapshots = 1 };
			virtual void Initialize() override;

			virtual void SetNetworkFlags(NetworkFlags flags);
			NetworkFlags GetNetworkFlags() const;
#if NETWORKED_VARS_ENABLED != 0
			struct DLLNETWORK NetworkedVariable {
				using Id = size_t;
				NetworkedVariable(Id id, pragma::util::VarType type, const std::shared_ptr<pragma::util::BaseProperty> &data);
				Id id;
				std::shared_ptr<pragma::util::BaseProperty> data;
				pragma::util::VarType type;
			};
			const std::shared_ptr<pragma::util::BaseProperty> &GetNetworkedVariableProperty(const std::string &name) const;
			const std::shared_ptr<pragma::util::BaseProperty> &GetNetworkedVariableProperty(NetworkedVariable::Id id) const;

			pragma::util::VarType GetNetworkedVariableType(NetworkedVariable::Id id) const;
			NetworkedVariable::Id CreateNetworkedVariable(const std::string &name, pragma::util::VarType type);

			template<typename T>
			bool SetNetworkedVariable(const std::string &name, const T &val);
			template<typename T>
			bool SetNetworkedVariable(NetworkedVariable::Id id, const T &val);

			const std::vector<NetworkedVariable> &GetNetworkedVariables() const;
#endif
		  protected:
			BaseNetworkedComponent(ecs::BaseEntity &ent);
			NetworkFlags m_networkFlags = NetworkFlags::None;
#if NETWORKED_VARS_ENABLED != 0
			virtual void OnNetworkedVariableCreated(NetworkedVariable &nwVar);
			template<typename T>
			T GetNetworkedVariable(const std::string &name, pragma::util::VarType type) const;

			std::vector<NetworkedVariable> m_networkedVars = {};
			std::vector<std::string> m_networkedVarNames = {};
			pragma::NetEventId m_netEvSetVar = pragma::INVALID_NET_EVENT;
#endif
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseNetworkedComponent::NetworkFlags)

#if NETWORKED_VARS_ENABLED != 0
	template<typename T>
	bool pragma::BaseNetworkedComponent::SetNetworkedVariable(NetworkedVariable::Id id, const T &val)
	{
		if(id >= m_networkedVars.size())
			return false;
		auto &nwVar = m_networkedVars.at(id);
		if constexpr(std::is_same_v<T, bool>) {
			if(nwVar.type != NetworkedVariable::Type::Bool)
				return false;
			static_cast<pragma::util::BoolProperty &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, double>) {
			if(nwVar.type != NetworkedVariable::Type::Double)
				return false;
			static_cast<pragma::util::DoubleProperty &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, float>) {
			if(nwVar.type != NetworkedVariable::Type::Float)
				return false;
			static_cast<pragma::util::FloatProperty &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, int8_t>) {
			if(nwVar.type != NetworkedVariable::Type::Int8)
				return false;
			static_cast<pragma::util::Int8Property &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, int16_t>) {
			if(nwVar.type != NetworkedVariable::Type::Int16)
				return false;
			static_cast<pragma::util::Int16Property &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, int32_t>) {
			if(nwVar.type != NetworkedVariable::Type::Int32)
				return false;
			static_cast<pragma::util::Int32Property &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, int64_t>) {
			if(nwVar.type != NetworkedVariable::Type::Int64)
				return false;
			static_cast<pragma::util::Int64Property &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, long double>) {
			if(nwVar.type != NetworkedVariable::Type::LongDouble)
				return false;
			static_cast<pragma::util::LongDoubleProperty &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, std::string>) {
			if(nwVar.type != NetworkedVariable::Type::String)
				return false;
			static_cast<pragma::util::StringProperty &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, uint8_t>) {
			if(nwVar.type != NetworkedVariable::Type::UInt8)
				return false;
			static_cast<pragma::util::UInt8Property &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, uint16_t>) {
			if(nwVar.type != NetworkedVariable::Type::UInt16)
				return false;
			static_cast<pragma::util::UInt16Property &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, uint32_t>) {
			if(nwVar.type != NetworkedVariable::Type::UInt32)
				return false;
			static_cast<pragma::util::UInt32Property &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, uint64_t>) {
			if(nwVar.type != NetworkedVariable::Type::UInt64)
				return false;
			static_cast<pragma::util::UInt64Property &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, EulerAngles>) {
			if(nwVar.type != NetworkedVariable::Type::EulerAngles)
				return false;
			static_cast<typename pragma::util::EulerAnglesProperty &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, Color>) {
			if(nwVar.type != NetworkedVariable::Type::Color)
				return false;
			static_cast<pragma::util::ColorProperty &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, Vector3>) {
			if(nwVar.type != NetworkedVariable::Type::Vector)
				return false;
			static_cast<pragma::util::Vector3Property &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, Vector2>) {
			if(nwVar.type != NetworkedVariable::Type::Vector2)
				return false;
			static_cast<pragma::util::Vector2Property &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, Vector4>) {
			if(nwVar.type != NetworkedVariable::Type::Vector4)
				return false;
			static_cast<pragma::util::Vector4Property &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, EntityHandle>) {
			if(nwVar.type != NetworkedVariable::Type::Entity)
				return false;
			static_cast<typename pragma::EntityProperty &>(*nwVar.data.get()) = val;
		}
		else if constexpr(std::is_same_v<T, Quat>) {
			if(nwVar.type != NetworkedVariable::Type::Quaternion)
				return false;
			static_cast<pragma::util::QuatProperty &>(*nwVar.data.get()) = val;
		}
		else
			return false;
		return true;
	}

	template<typename T>
	bool pragma::BaseNetworkedComponent::SetNetworkedVariable(const std::string &name, const T &val)
	{
		auto it = std::find_if(m_networkedVarNames.begin(), m_networkedVarNames.end(), [&name](const std::string &nameOther) { return pragma::string::compare(name, nameOther, false); });
		if(it == m_networkedVarNames.end())
			return false;
		return SetNetworkedVariable(it - m_networkedVarNames.begin(), val);
	}
#endif
};
