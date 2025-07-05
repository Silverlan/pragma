// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __AI_MEMORY_H__
#define __AI_MEMORY_H__

#include "pragma/serverdefinitions.h"
#include <mathutil/glmutil.h>

#define MAX_AIMEMORY_FRAGMENTS 25

#define AI_MEMORY_NEXT_CHECK_IF_HIDDEN 0.25f
#define AI_MEMORY_NEXT_CHECK_IF_VISIBLE 0.1f

class BaseEntity;
namespace pragma {
	namespace ai {
		class DLLSERVER Memory {
		  public:
			class DLLSERVER Fragment {
			  protected:
				void Clear();
				friend Memory;
			  public:
				Fragment();
				Fragment(const Fragment &) = delete;
				Fragment &operator=(const Fragment &) = delete;
				Fragment Copy() const;
				bool occupied;
				EntityHandle hEntity;
				Vector3 lastPosition;
				Vector3 lastVelocity;
				float lastDistance;
				float lastSeen;
				float lastHeared;
				float lastCheck;
				bool visible;
				float GetLastTimeSensed() const;

				void UpdateVisibility(float dist);
			};
			enum class DLLSERVER MemoryType : uint32_t {
				Visual = 0,
				Sound,
				Smell // Unused
			};
		  protected:
			void Memorize(const BaseEntity &ent, MemoryType memType, const Vector3 &pos, float dist, const Vector3 &vel, int idx, Fragment **out = nullptr);
		  public:
			Memory();
			Memory(const Memory &) = delete;
			Memory Copy() const;
			Memory &operator=(const Memory &) = delete;
			uint32_t occupiedFragmentCount;
			std::array<Fragment, MAX_AIMEMORY_FRAGMENTS> fragments;
			bool Memorize(const BaseEntity &ent, MemoryType memType, const Vector3 &pos, float dist, const Vector3 &vel = {}, Fragment **out = nullptr);
			Fragment *FindFragment(const BaseEntity &ent);
			void Forget(const BaseEntity &ent);
			void Clear();
			void Clear(Fragment &fragment);
			void Update();
		};
	};
};

#endif
