// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.surface_type_manager;

export import :util.type_manager;

export namespace pragma::util {
	template<class TType>
	class TTypeManager;
}
export namespace pragma::physics {
	class DLLNETWORK SurfaceType : public util::BaseNamedType {
	  protected:
		template<class TType>
		friend class util::TTypeManager;
		SurfaceType(util::TypeId id, const std::string &name);
	};

	using SurfaceTypeManager = util::TTypeManager<SurfaceType>;
};
