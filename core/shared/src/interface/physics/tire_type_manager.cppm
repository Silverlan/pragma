// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.tire_type_manager;

export import :util.type_manager;

export import :physics.surface_type_manager;

export namespace pragma::util {
	template<class TType>
	class TTypeManager;
}
export namespace pragma::physics {
	class DLLNETWORK TireType : public util::BaseNamedType {
	  public:
		void SetFrictionModifier(SurfaceType &surfType, float modifier);
		std::optional<float> GetFrictionModifier(SurfaceType &surfType);
		const std::unordered_map<SurfaceType *, float> &GetFrictionModifiers() const;
	  protected:
		template<class TType>
		friend class util::TTypeManager; //this must be explicit for some reason.
		TireType(util::TypeId id, const std::string &name);
	  private:
		std::unordered_map<SurfaceType *, float> m_frictionModifiers = {};
	};

	using TireTypeManager = util::TTypeManager<TireType>;
};
