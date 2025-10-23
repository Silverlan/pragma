// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <memory>
#include <string>

export module pragma.shared:entities.components.meta_rig;

export import :entities.components.base_animated;
export import :entities.base_entity;

export namespace pragma {
	class DLLNETWORK MetaRigComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		MetaRigComponent(pragma::ecs::BaseEntity &ent);
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

void pragma::MetaRigComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = MetaRigComponent;

	for(size_t i = 0; i < umath::to_integral(animation::MetaRigBoneType::Count); ++i) {
		std::string name = animation::get_meta_rig_bone_type_name(static_cast<animation::MetaRigBoneType>(i));

		std::string parentPathName;
		auto parentType = animation::get_meta_rig_bone_parent_type(static_cast<animation::MetaRigBoneType>(i));
		if(parentType)
			parentPathName = "bone/" + std::string {animation::get_meta_rig_bone_type_name(*parentType)} + "/pose";
		std::shared_ptr<ents::ParentTypeMetaData> parentMetaData {};
		if(!parentPathName.empty()) {
			parentMetaData = std::make_shared<ents::ParentTypeMetaData>();
			parentMetaData->parentProperty = parentPathName;
		}

		auto posePathName = "bone/" + name + "/pose";
		auto posPathName = "bone/" + name + "/position";
		auto rotPathName = "bone/" + name + "/rotation";
		auto scalePathName = "bone/" + name + "/scale";
		auto poseMetaData = std::make_shared<ents::PoseTypeMetaData>();
		poseMetaData->posProperty = posPathName;
		poseMetaData->rotProperty = rotPathName;
		poseMetaData->scaleProperty = scalePathName;

		auto poseComponentMetaData = std::make_shared<ents::PoseComponentTypeMetaData>();
		poseComponentMetaData->poseProperty = posePathName;

		auto coordMetaData = std::make_shared<ents::CoordinateTypeMetaData>();
		coordMetaData->space = umath::CoordinateSpace::Local;
		coordMetaData->parentProperty = parentPathName;

		auto memberInfoPose = pragma::ComponentMemberInfo::CreateDummy();
		memberInfoPose.SetName(posePathName);
		memberInfoPose.AddTypeMetaData(poseMetaData);
		if(parentMetaData)
			memberInfoPose.AddTypeMetaData(parentMetaData);
		memberInfoPose.type = ents::EntityMemberType::ScaledTransform;
		memberInfoPose.userIndex = i;
		memberInfoPose.SetFlag(pragma::ComponentMemberFlags::HideInInterface);
		memberInfoPose.AddTypeMetaData(coordMetaData);
		memberInfoPose
		  .SetGetterFunction<MetaRigComponent, umath::ScaledTransform, static_cast<void (*)(const pragma::ComponentMemberInfo &, MetaRigComponent &, umath::ScaledTransform &)>([](const pragma::ComponentMemberInfo &memberInfo, MetaRigComponent &component, umath::ScaledTransform &outValue) {
			  if(!component.GetBonePose(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), outValue)) {
				  outValue = {};
				  return;
			  }
		  })>();
		memberInfoPose.SetSetterFunction<MetaRigComponent, umath::ScaledTransform,
		  static_cast<void (*)(const pragma::ComponentMemberInfo &, MetaRigComponent &, const umath::ScaledTransform &)>(
		    [](const pragma::ComponentMemberInfo &memberInfo, MetaRigComponent &component, const umath::ScaledTransform &value) { component.SetBonePose(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), value); })>();

		auto memberInfoPos = pragma::ComponentMemberInfo::CreateDummy();
		memberInfoPos.SetName("bone/" + name + "/position");
		memberInfoPos.type = ents::EntityMemberType::Vector3;
		memberInfoPos.userIndex = i;
		memberInfoPos.AddTypeMetaData(coordMetaData);
		memberInfoPos.AddTypeMetaData(poseComponentMetaData);
		if(parentMetaData)
			memberInfoPos.AddTypeMetaData(parentMetaData);
		memberInfoPos.SetGetterFunction<MetaRigComponent, Vector3, static_cast<void (*)(const pragma::ComponentMemberInfo &, MetaRigComponent &, Vector3 &)>([](const pragma::ComponentMemberInfo &memberInfo, MetaRigComponent &component, Vector3 &outValue) {
			if(!component.GetBonePos(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), outValue)) {
				outValue = {};
				return;
			}
		})>();
		memberInfoPos.SetSetterFunction<MetaRigComponent, Vector3, static_cast<void (*)(const pragma::ComponentMemberInfo &, MetaRigComponent &, const Vector3 &)>([](const pragma::ComponentMemberInfo &memberInfo, MetaRigComponent &component, const Vector3 &value) {
			component.SetBonePos(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), value);
		})>();

		auto memberInfoRot = memberInfoPos;
		memberInfoRot.SetName("bone/" + name + "/rotation");
		memberInfoRot.type = ents::EntityMemberType::Quaternion;
		memberInfoRot.userIndex = i;
		memberInfoRot.AddTypeMetaData(coordMetaData);
		memberInfoRot.AddTypeMetaData(poseComponentMetaData);
		if(parentMetaData)
			memberInfoRot.AddTypeMetaData(parentMetaData);
		memberInfoRot.SetGetterFunction<MetaRigComponent, Quat, static_cast<void (*)(const pragma::ComponentMemberInfo &, MetaRigComponent &, Quat &)>([](const pragma::ComponentMemberInfo &memberInfo, MetaRigComponent &component, Quat &outValue) {
			if(!component.GetBoneRot(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), outValue)) {
				outValue = {};
				return;
			}
		})>();
		memberInfoRot.SetSetterFunction<MetaRigComponent, Quat, static_cast<void (*)(const pragma::ComponentMemberInfo &, MetaRigComponent &, const Quat &)>([](const pragma::ComponentMemberInfo &memberInfo, MetaRigComponent &component, const Quat &value) {
			component.SetBoneRot(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), value);
		})>();

		auto memberInfoScale = memberInfoPos;
		memberInfoScale.SetName("bone/" + name + "/scale");
		memberInfoScale.userIndex = i;
		memberInfoScale.AddTypeMetaData(coordMetaData);
		memberInfoScale.AddTypeMetaData(poseMetaData);
		if(parentMetaData)
			memberInfoScale.AddTypeMetaData(parentMetaData);
		memberInfoScale.SetGetterFunction<MetaRigComponent, Vector3, static_cast<void (*)(const pragma::ComponentMemberInfo &, MetaRigComponent &, Vector3 &)>([](const pragma::ComponentMemberInfo &memberInfo, MetaRigComponent &component, Vector3 &outValue) {
			if(!component.GetBoneScale(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), outValue)) {
				outValue = {};
				return;
			}
		})>();
		memberInfoScale.SetSetterFunction<MetaRigComponent, Vector3, static_cast<void (*)(const pragma::ComponentMemberInfo &, MetaRigComponent &, const Vector3 &)>([](const pragma::ComponentMemberInfo &memberInfo, MetaRigComponent &component, const Vector3 &value) {
			component.SetBoneScale(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), value);
		})>();

		registerMember(std::move(memberInfoPose));
		registerMember(std::move(memberInfoPos));
		registerMember(std::move(memberInfoRot));
		registerMember(std::move(memberInfoScale));
	}
}
pragma::MetaRigComponent::MetaRigComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void pragma::MetaRigComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("animated");
}
void pragma::MetaRigComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void pragma::MetaRigComponent::OnRemove() { BaseEntityComponent::OnRemove(); }

bool pragma::MetaRigComponent::GetBonePose(animation::MetaRigBoneType bone, umath::ScaledTransform &outPose, umath::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->GetBonePose(m_metaRig->GetBoneId(bone), outPose, space))
		return false;
	return true;
}
bool pragma::MetaRigComponent::GetBonePos(animation::MetaRigBoneType bone, Vector3 &outPos, umath::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->GetBonePos(m_metaRig->GetBoneId(bone), outPos, space))
		return false;
	return true;
}
bool pragma::MetaRigComponent::GetBoneRot(animation::MetaRigBoneType bone, Quat &outRot, umath::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->GetBoneRot(m_metaRig->GetBoneId(bone), outRot, space))
		return false;
	return true;
}
bool pragma::MetaRigComponent::GetBoneScale(animation::MetaRigBoneType bone, Vector3 &outScale, umath::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->GetBoneScale(m_metaRig->GetBoneId(bone), outScale, space))
		return false;
	return true;
}

bool pragma::MetaRigComponent::SetBonePose(animation::MetaRigBoneType bone, const umath::ScaledTransform &pose, umath::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->SetBonePose(m_metaRig->GetBoneId(bone), pose, space))
		return false;
	return true;
}
bool pragma::MetaRigComponent::SetBonePos(animation::MetaRigBoneType bone, const Vector3 &pos, umath::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->SetBonePos(m_metaRig->GetBoneId(bone), pos, space))
		return false;
	return true;
}
bool pragma::MetaRigComponent::SetBoneRot(animation::MetaRigBoneType bone, const Quat &rot, umath::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->SetBoneRot(m_metaRig->GetBoneId(bone), rot, space))
		return false;
	return true;
}
bool pragma::MetaRigComponent::SetBoneScale(animation::MetaRigBoneType bone, const Vector3 &scale, umath::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->SetBoneScale(m_metaRig->GetBoneId(bone), scale, space))
		return false;
	return true;
}
