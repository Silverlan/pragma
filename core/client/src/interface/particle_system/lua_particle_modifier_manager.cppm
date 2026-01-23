// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.lua_particle_modifier_manager;

export import :particle_system.initializer_lua;

export namespace pragma::pts {
	class DLLCLIENT LuaParticleModifierManager {
	  public:
		enum class Type : uint8_t {
			Initializer = 0,
			Operator,
			Renderer,
			Emitter,

			Invalid = std::numeric_limits<uint8_t>::max()
		};
		bool RegisterModifier(Type type, std::string className, luabind::object &o);
		luabind::object *GetClassObject(std::string className);
		CParticleModifierLua *CreateModifier(std::string className) const;
	  private:
		struct ModifierInfo {
			mutable luabind::object luaClassObject;
			Type type = Type::Invalid;
		};
		std::unordered_map<std::string, ModifierInfo> m_modifiers;
	};
};
