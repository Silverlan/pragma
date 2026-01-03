// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.bone_merge;

export import :entities.components.base;
export import :entities.universal_reference;
export import :model.animation.bone;
export import :types;

export {
	namespace pragma {
		namespace boneMergeComponent {
			REGISTER_COMPONENT_EVENT(EVENT_ON_TARGET_CHANGED)
		}
		class DLLNETWORK BoneMergeComponent final : public BaseEntityComponent {
		  public:
			static bool can_merge(const asset::Model &mdl, const asset::Model &mdlParent, bool includeRootBones = false);

			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

			BoneMergeComponent(ecs::BaseEntity &ent);
			virtual void Initialize() override;
			virtual void OnRemove() override;
			virtual void OnTick(double tDelta) override;

			void SetTarget(const EntityURef &target);
			const EntityURef &GetTarget() const;

			virtual void InitializeLuaObject(lua::State *lua) override;
		  private:
			struct BoneMapping {
				animation::BoneId boneId;
				animation::BoneId parentBoneId;
			};
			std::vector<BoneMapping> m_boneMappings;
			EntityURef m_target;
			ComponentHandle<BaseAnimatedComponent> m_animC;
			ComponentHandle<BaseAnimatedComponent> m_animCParent;
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
			virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
			void SetTargetDirty();
			void UpdateBoneMappings();
			void MergeBonePoses();
		};
	};
}
