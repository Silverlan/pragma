// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.debug_hitbox;

import :debug;

export namespace pragma {
	class DLLCLIENT CDebugHitboxComponent final : public BaseEntityComponent {
	  public:
		CDebugHitboxComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual ~CDebugHitboxComponent() override;
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;
		virtual void OnTick(double tDelta) override;

		void SetHitboxColor(animation::BoneId boneId, const std::optional<Color> &color);
	  protected:
		void InitializeDebugObjects();
		void ClearDebugObjects();
		std::vector<std::shared_ptr<debug::DebugRenderer::BaseObject>> m_debugObjects;
		std::unordered_map<animation::BoneId, Color> m_hitboxColors;
		bool m_dirty = false;
	};
};
