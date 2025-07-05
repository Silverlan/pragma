// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BONE_MERGE_COMPONENT_HPP__
#define __BONE_MERGE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
	class DLLNETWORK BoneMergeComponent final : public BaseEntityComponent {
	  public:
		static bool can_merge(const Model &mdl, const Model &mdlParent, bool includeRootBones = false);
		static ComponentEventId EVENT_ON_TARGET_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		BoneMergeComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnTick(double tDelta) override;

		void SetTarget(const pragma::EntityURef &target);
		const pragma::EntityURef &GetTarget() const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  private:
		struct BoneMapping {
			animation::BoneId boneId;
			animation::BoneId parentBoneId;
		};
		std::vector<BoneMapping> m_boneMappings;
		pragma::EntityURef m_target;
		pragma::ComponentHandle<BaseAnimatedComponent> m_animC;
		pragma::ComponentHandle<BaseAnimatedComponent> m_animCParent;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		void SetTargetDirty();
		void UpdateBoneMappings();
		void MergeBonePoses();
	};
};

#endif
