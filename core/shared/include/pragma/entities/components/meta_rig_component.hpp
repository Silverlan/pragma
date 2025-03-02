/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#ifndef __META_RIG_COMPONENT_HPP__
#define __META_RIG_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
	namespace animation {
		enum class MetaRigBoneType : uint8_t;
		struct MetaRig;
	};
	class DLLNETWORK MetaRigComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		MetaRigComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		bool GetBonePose(animation::MetaRigBoneType bone, umath::ScaledTransform &outPose, umath::CoordinateSpace space = umath::CoordinateSpace::Local) const;
		bool GetBonePos(animation::MetaRigBoneType bone, Vector3 &outPos, umath::CoordinateSpace space = umath::CoordinateSpace::Local) const;
		bool GetBoneRot(animation::MetaRigBoneType bone, Quat &outRot, umath::CoordinateSpace space = umath::CoordinateSpace::Local) const;
		bool GetBoneScale(animation::MetaRigBoneType bone, Vector3 &outScale, umath::CoordinateSpace space = umath::CoordinateSpace::Local) const;

		bool SetBonePose(animation::MetaRigBoneType bone, const umath::ScaledTransform &pose, umath::CoordinateSpace space = umath::CoordinateSpace::Local) const;
		bool SetBonePos(animation::MetaRigBoneType bone, const Vector3 &pos, umath::CoordinateSpace space = umath::CoordinateSpace::Local) const;
		bool SetBoneRot(animation::MetaRigBoneType bone, const Quat &rot, umath::CoordinateSpace space = umath::CoordinateSpace::Local) const;
		bool SetBoneScale(animation::MetaRigBoneType bone, const Vector3 &scale, umath::CoordinateSpace space = umath::CoordinateSpace::Local) const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  private:
		BaseAnimatedComponent *m_animC = nullptr;
		std::shared_ptr<animation::MetaRig> m_metaRig = nullptr;
	};
};

#endif
