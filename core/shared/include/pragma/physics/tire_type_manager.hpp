// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __TIRE_TYPE_MANAGER_HPP__
#define __TIRE_TYPE_MANAGER_HPP__

#include "pragma/util/util_typed_manager.hpp"
#include <optional>
#include <unordered_map>

namespace pragma::physics {
	class SurfaceType;
	class DLLNETWORK TireType : public BaseNamedType {
	  public:
		void SetFrictionModifier(SurfaceType &surfType, float modifier);
		std::optional<float> GetFrictionModifier(SurfaceType &surfType);
		const std::unordered_map<SurfaceType *, float> &GetFrictionModifiers() const;
	  protected:
		template<class TType>
		friend class pragma::TTypeManager; //this must be explicit for some reason.
		TireType(TypeId id, const std::string &name);
	  private:
		std::unordered_map<SurfaceType *, float> m_frictionModifiers = {};
	};

	using TireTypeManager = TTypeManager<TireType>;
};

#endif
