// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.material;

export import :physics.base;
export import :physics.surface_material;

export namespace pragma::physics {
	class DLLNETWORK IMaterial : public IBase {
	  public:
		using IBase::IBase;
		virtual ~IMaterial() = default;
		void SetFriction(float friction);
		virtual float GetStaticFriction() const = 0;
		virtual void SetStaticFriction(float friction) = 0;
		virtual float GetDynamicFriction() const = 0;
		virtual void SetDynamicFriction(float friction) = 0;
		virtual float GetRestitution() const = 0;
		virtual void SetRestitution(float restitution) = 0;

		void SetSurfaceMaterial(SurfaceMaterial &surfMat);
		SurfaceMaterial *GetSurfaceMaterial() const;
		virtual void InitializeLuaObject(lua::State *lua) override;
	  private:
		uint32_t m_surfMatIdx = std::numeric_limits<uint32_t>::max();
	};
};
