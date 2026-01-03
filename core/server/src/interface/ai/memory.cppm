// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.memory;

export import pragma.shared;

export namespace pragma {
	constexpr uint32_t MAX_AIMEMORY_FRAGMENTS = 25;

	constexpr float AI_MEMORY_NEXT_CHECK_IF_HIDDEN = 0.25f;
	constexpr float AI_MEMORY_NEXT_CHECK_IF_VISIBLE = 0.1f;

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
			void Memorize(const ecs::BaseEntity &ent, MemoryType memType, const Vector3 &pos, float dist, const Vector3 &vel, int idx, Fragment **out = nullptr);
		  public:
			Memory();
			Memory(const Memory &) = delete;
			Memory Copy() const;
			Memory &operator=(const Memory &) = delete;
			uint32_t occupiedFragmentCount;
			std::array<Fragment, MAX_AIMEMORY_FRAGMENTS> fragments;
			bool Memorize(const ecs::BaseEntity &ent, MemoryType memType, const Vector3 &pos, float dist, const Vector3 &vel = {}, Fragment **out = nullptr);
			Fragment *FindFragment(const ecs::BaseEntity &ent);
			void Forget(const ecs::BaseEntity &ent);
			void Clear();
			void Clear(Fragment &fragment);
			void Update();
		};
	};
};
