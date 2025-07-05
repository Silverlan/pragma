// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_ENV_SOUND_PROBE_HPP__
#define __C_ENV_SOUND_PROBE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma {
	class DLLCLIENT CEnvSoundProbeComponent final : public BaseEntityComponent {
	  public:
		enum class Placement : uint32_t { Centroid = 0u, Octree, UniformFloor };
		struct DLLCLIENT ProbeInfo {
			ProbeInfo() = default;
			ProbeInfo(const Vector3 &min, const Vector3 &max, Placement placement, float spacing, float heightAboveFloor) : min(min), max(max), placement(placement), spacing(spacing), heightAboveFloor(heightAboveFloor) {}
			Vector3 min = {};
			Vector3 max = {};
			Placement placement = Placement::Centroid;
			float spacing = 512.f;
			float heightAboveFloor = 50.f;
		};
		static const std::vector<ProbeInfo> &GetProbes();
		CEnvSoundProbeComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		static std::vector<ProbeInfo> s_probes;
		static CallbackHandle s_probeCallback;
		static void ClearProbes();
	  private:
		float m_spacing = 512.f;
		float m_heightAboveFloor = 50.f;
		float m_radius = 512.f;
	};
};

class DLLCLIENT CEnvSoundProbe : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
