/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LPARTICLE_MODIFIERS_HPP__
#define __C_LPARTICLE_MODIFIERS_HPP__

#include <pragma/lua/ldefinitions.h>
#include "pragma/lua/classes/components/c_lentity_components.hpp"

namespace pragma {
	class BaseEnvParticleSystemComponent;
};
namespace Lua {
	namespace ParticleSystemModifier {
		void register_particle_class(luabind::class_<pragma::CParticleSystemComponent, pragma::BaseEnvParticleSystemComponent> &defPtc);
		void register_modifier_class(luabind::class_<pragma::CParticleSystemComponent, pragma::BaseEnvParticleSystemComponent> &defPtc);
	};
};

class CParticleModifierLua;
namespace pragma {
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

#endif
