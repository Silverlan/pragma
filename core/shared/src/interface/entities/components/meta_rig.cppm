// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.meta_rig;

export import :entities.components.base_animated;
export import :entities.base_entity;

export namespace pragma {
	class DLLNETWORK MetaRigComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		MetaRigComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		bool GetBonePose(animation::MetaRigBoneType bone, math::ScaledTransform &outPose, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
		bool GetBonePos(animation::MetaRigBoneType bone, Vector3 &outPos, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
		bool GetBoneRot(animation::MetaRigBoneType bone, Quat &outRot, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
		bool GetBoneScale(animation::MetaRigBoneType bone, Vector3 &outScale, math::CoordinateSpace space = math::CoordinateSpace::Local) const;

		bool SetBonePose(animation::MetaRigBoneType bone, const math::ScaledTransform &pose, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
		bool SetBonePos(animation::MetaRigBoneType bone, const Vector3 &pos, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
		bool SetBoneRot(animation::MetaRigBoneType bone, const Quat &rot, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
		bool SetBoneScale(animation::MetaRigBoneType bone, const Vector3 &scale, math::CoordinateSpace space = math::CoordinateSpace::Local) const;

		virtual void InitializeLuaObject(lua::State *lua) override;
	  private:
		BaseAnimatedComponent *m_animC = nullptr;
		std::shared_ptr<animation::MetaRig> m_metaRig = nullptr;
	};
};

void pragma::MetaRigComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = MetaRigComponent;

	for(size_t i = 0; i < math::to_integral(animation::MetaRigBoneType::Count); ++i) {
		std::string name = animation::get_meta_rig_bone_type_name(static_cast<animation::MetaRigBoneType>(i));

		std::string parentPathName;
		auto parentType = animation::get_meta_rig_bone_parent_type(static_cast<animation::MetaRigBoneType>(i));
		if(parentType)
			parentPathName = "bone/" + std::string {animation::get_meta_rig_bone_type_name(*parentType)} + "/pose";
		std::shared_ptr<ents::ParentTypeMetaData> parentMetaData {};
		if(!parentPathName.empty()) {
			parentMetaData = pragma::util::make_shared<ents::ParentTypeMetaData>();
			parentMetaData->parentProperty = parentPathName;
		}

		auto posePathName = "bone/" + name + "/pose";
		auto posPathName = "bone/" + name + "/position";
		auto rotPathName = "bone/" + name + "/rotation";
		auto scalePathName = "bone/" + name + "/scale";
		auto poseMetaData = pragma::util::make_shared<ents::PoseTypeMetaData>();
		poseMetaData->posProperty = posPathName;
		poseMetaData->rotProperty = rotPathName;
		poseMetaData->scaleProperty = scalePathName;

		auto poseComponentMetaData = pragma::util::make_shared<ents::PoseComponentTypeMetaData>();
		poseComponentMetaData->poseProperty = posePathName;

		auto coordMetaData = pragma::util::make_shared<ents::CoordinateTypeMetaData>();
		coordMetaData->space = math::CoordinateSpace::Local;
		coordMetaData->parentProperty = parentPathName;

		auto memberInfoPose = ComponentMemberInfo::CreateDummy();
		memberInfoPose.SetName(posePathName);
		memberInfoPose.AddTypeMetaData(poseMetaData);
		if(parentMetaData)
			memberInfoPose.AddTypeMetaData(parentMetaData);
		memberInfoPose.type = ents::EntityMemberType::ScaledTransform;
		memberInfoPose.userIndex = i;
		memberInfoPose.SetFlag(ComponentMemberFlags::HideInInterface);
		memberInfoPose.AddTypeMetaData(coordMetaData);
		memberInfoPose
		  .SetGetterFunction<MetaRigComponent, math::ScaledTransform, static_cast<void (*)(const ComponentMemberInfo &, MetaRigComponent &, math::ScaledTransform &)>([](const ComponentMemberInfo &memberInfo, MetaRigComponent &component, math::ScaledTransform &outValue) {
			  if(!component.GetBonePose(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), outValue)) {
				  outValue = {};
				  return;
			  }
		  })>();
		memberInfoPose.SetSetterFunction<MetaRigComponent, math::ScaledTransform,
		  static_cast<void (*)(const ComponentMemberInfo &, MetaRigComponent &, const math::ScaledTransform &)>(
		    [](const ComponentMemberInfo &memberInfo, MetaRigComponent &component, const math::ScaledTransform &value) { component.SetBonePose(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), value); })>();

		auto memberInfoPos = ComponentMemberInfo::CreateDummy();
		memberInfoPos.SetName("bone/" + name + "/position");
		memberInfoPos.type = ents::EntityMemberType::Vector3;
		memberInfoPos.userIndex = i;
		memberInfoPos.AddTypeMetaData(coordMetaData);
		memberInfoPos.AddTypeMetaData(poseComponentMetaData);
		if(parentMetaData)
			memberInfoPos.AddTypeMetaData(parentMetaData);
		memberInfoPos.SetGetterFunction<MetaRigComponent, Vector3, static_cast<void (*)(const ComponentMemberInfo &, MetaRigComponent &, Vector3 &)>([](const ComponentMemberInfo &memberInfo, MetaRigComponent &component, Vector3 &outValue) {
			if(!component.GetBonePos(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), outValue)) {
				outValue = {};
				return;
			}
		})>();
		memberInfoPos.SetSetterFunction<MetaRigComponent, Vector3, static_cast<void (*)(const ComponentMemberInfo &, MetaRigComponent &, const Vector3 &)>([](const ComponentMemberInfo &memberInfo, MetaRigComponent &component, const Vector3 &value) {
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
		memberInfoRot.SetGetterFunction<MetaRigComponent, Quat, static_cast<void (*)(const ComponentMemberInfo &, MetaRigComponent &, Quat &)>([](const ComponentMemberInfo &memberInfo, MetaRigComponent &component, Quat &outValue) {
			if(!component.GetBoneRot(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), outValue)) {
				outValue = {};
				return;
			}
		})>();
		memberInfoRot.SetSetterFunction<MetaRigComponent, Quat, static_cast<void (*)(const ComponentMemberInfo &, MetaRigComponent &, const Quat &)>([](const ComponentMemberInfo &memberInfo, MetaRigComponent &component, const Quat &value) {
			component.SetBoneRot(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), value);
		})>();

		auto memberInfoScale = memberInfoPos;
		memberInfoScale.SetName("bone/" + name + "/scale");
		memberInfoScale.userIndex = i;
		memberInfoScale.AddTypeMetaData(coordMetaData);
		memberInfoScale.AddTypeMetaData(poseMetaData);
		if(parentMetaData)
			memberInfoScale.AddTypeMetaData(parentMetaData);
		memberInfoScale.SetGetterFunction<MetaRigComponent, Vector3, static_cast<void (*)(const ComponentMemberInfo &, MetaRigComponent &, Vector3 &)>([](const ComponentMemberInfo &memberInfo, MetaRigComponent &component, Vector3 &outValue) {
			if(!component.GetBoneScale(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), outValue)) {
				outValue = {};
				return;
			}
		})>();
		memberInfoScale.SetSetterFunction<MetaRigComponent, Vector3, static_cast<void (*)(const ComponentMemberInfo &, MetaRigComponent &, const Vector3 &)>([](const ComponentMemberInfo &memberInfo, MetaRigComponent &component, const Vector3 &value) {
			component.SetBoneScale(static_cast<animation::MetaRigBoneType>(memberInfo.userIndex), value);
		})>();

		registerMember(std::move(memberInfoPose));
		registerMember(std::move(memberInfoPos));
		registerMember(std::move(memberInfoRot));
		registerMember(std::move(memberInfoScale));
	}
}
pragma::MetaRigComponent::MetaRigComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void pragma::MetaRigComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("animated");
}
void pragma::MetaRigComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void pragma::MetaRigComponent::OnRemove() { BaseEntityComponent::OnRemove(); }

bool pragma::MetaRigComponent::GetBonePose(animation::MetaRigBoneType bone, math::ScaledTransform &outPose, math::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->GetBonePose(m_metaRig->GetBoneId(bone), outPose, space))
		return false;
	return true;
}
bool pragma::MetaRigComponent::GetBonePos(animation::MetaRigBoneType bone, Vector3 &outPos, math::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->GetBonePos(m_metaRig->GetBoneId(bone), outPos, space))
		return false;
	return true;
}
bool pragma::MetaRigComponent::GetBoneRot(animation::MetaRigBoneType bone, Quat &outRot, math::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->GetBoneRot(m_metaRig->GetBoneId(bone), outRot, space))
		return false;
	return true;
}
bool pragma::MetaRigComponent::GetBoneScale(animation::MetaRigBoneType bone, Vector3 &outScale, math::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->GetBoneScale(m_metaRig->GetBoneId(bone), outScale, space))
		return false;
	return true;
}

bool pragma::MetaRigComponent::SetBonePose(animation::MetaRigBoneType bone, const math::ScaledTransform &pose, math::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->SetBonePose(m_metaRig->GetBoneId(bone), pose, space))
		return false;
	return true;
}
bool pragma::MetaRigComponent::SetBonePos(animation::MetaRigBoneType bone, const Vector3 &pos, math::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->SetBonePos(m_metaRig->GetBoneId(bone), pos, space))
		return false;
	return true;
}
bool pragma::MetaRigComponent::SetBoneRot(animation::MetaRigBoneType bone, const Quat &rot, math::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->SetBoneRot(m_metaRig->GetBoneId(bone), rot, space))
		return false;
	return true;
}
bool pragma::MetaRigComponent::SetBoneScale(animation::MetaRigBoneType bone, const Vector3 &scale, math::CoordinateSpace space) const
{
	if(!m_animC || !m_metaRig)
		return false;
	if(!m_animC->SetBoneScale(m_metaRig->GetBoneId(bone), scale, space))
		return false;
	return true;
}
