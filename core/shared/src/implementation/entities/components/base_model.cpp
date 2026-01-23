// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_model;

using namespace pragma;

void BaseModelComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseModelComponent::EVENT_ON_MODEL_CHANGED = registerEvent("ON_MODEL_CHANGED", ComponentEventInfo::Type::Broadcast);
	baseModelComponent::EVENT_ON_MODEL_MATERIALS_LOADED = registerEvent("ON_MODEL_MATERIALS_LOADED", ComponentEventInfo::Type::Broadcast);
	baseModelComponent::EVENT_ON_SKIN_CHANGED = registerEvent("ON_SKIN_CHANGED", ComponentEventInfo::Type::Broadcast);
	baseModelComponent::EVENT_ON_BODY_GROUP_CHANGED = registerEvent("ON_BODY_GROUP_CHANGED", ComponentEventInfo::Type::Broadcast);
}
void BaseModelComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseModelComponent;

	{
		using TSkin = uint32_t;
		auto memberInfo = create_component_member_info<T, TSkin, static_cast<void (T::*)(TSkin)>(&T::SetSkin), static_cast<TSkin (T::*)() const>(&T::GetSkin)>("skin", 0);
		memberInfo.SetMin(0.f);
		registerMember(std::move(memberInfo));
	}
	{
		using TModel = std::string;
		auto memberInfo = create_component_member_info<T, TModel, [](const ComponentMemberInfo &info, T &component, const TModel &mdl) { component.SetModel(mdl); }, static_cast<TModel (T::*)() const>(&T::GetModelName)>("model", "", AttributeSpecializationType::File);
		auto &metaData = memberInfo.AddMetaData();
		metaData["assetType"] = "model";
		metaData["rootPath"] = util::Path::CreatePath(pragma::asset::get_asset_root_directory(asset::Type::Model)).GetString();
		metaData["extensions"] = pragma::asset::get_supported_extensions(asset::Type::Model, asset::FormatType::All);
		metaData["stripRootPath"] = true;
		metaData["stripExtension"] = true;
		registerMember(std::move(memberInfo));
	}
}
BaseModelComponent::BaseModelComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent)
{
	m_skin = util::SimpleProperty<util::UInt32Property, uint32_t>::Create(0u);
	m_skin->AddCallback([this](std::reference_wrapper<const uint32_t> oldVal, std::reference_wrapper<const uint32_t> newVal) { SetSkin(newVal.get()); });
}

void BaseModelComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvSetBodyGroup = SetupNetEvent("set_body_group");
	m_netEvMaxDrawDist = SetupNetEvent("set_max_draw_distance");
	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "model", false)) {
			m_kvModel = kvData.value;
			if(GetEntity().IsSpawned())
				SetModel(m_kvModel);
		}
		else if(pragma::string::compare<std::string>(kvData.key, "skin", false)) {
			m_kvSkin = util::to_int(kvData.value);
			if(GetEntity().IsSpawned())
				SetSkin(m_kvSkin);
		}
		else if(pragma::string::compare<std::string>(kvData.key, "maxvisibledist", false))
			SetMaxDrawDistance(string::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

void BaseModelComponent::SetMaxDrawDistance(float maxDist) { m_maxDrawDistance = maxDist; }
float BaseModelComponent::GetMaxDrawDistance() const { return m_maxDrawDistance; }

void BaseModelComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(m_modelName) {
		auto mdlName = std::move(*m_modelName);
		m_modelName = nullptr;
		SetModel(mdlName);
	}
	else if(m_kvModel.empty() == false)
		SetModel(m_kvModel);
	if(m_kvSkin != std::numeric_limits<uint32_t>::max())
		SetSkin(m_kvSkin);
}

bool BaseModelComponent::GetAttachment(unsigned int attID, Vector3 *pos, EulerAngles *angles) const
{
	Quat rot;
	if(GetAttachment(attID, pos, &rot) == false)
		return false;
	if(angles)
		*angles = EulerAngles {rot};
	return true;
}
bool BaseModelComponent::GetAttachment(const std::string &name, Vector3 *pos, EulerAngles *angles) const
{
	auto &hMdl = GetEntity().GetModel();
	if(hMdl == nullptr)
		return false;
	int attID = hMdl->LookupAttachment(name);
	if(attID == -1)
		return false;
	return GetAttachment(attID, pos, angles);
}
bool BaseModelComponent::GetAttachment(unsigned int attID, Vector3 *pos, Quat *rot) const
{
	auto &ent = GetEntity();
	auto &mdl = GetModel();
	auto *att = mdl ? mdl->GetAttachment(attID) : nullptr;
	if(att == nullptr)
		return false;
	math::Transform pose {};
	// ent.GetPose(pose);
	auto animC = ent.GetAnimatedComponent();
	if(animC.valid()) {
		Quat rotBone;
		Vector3 posBone;
		if(animC->GetBonePose(att->bone, &posBone, &rotBone, nullptr, math::CoordinateSpace::Object) == false)
			return false;
		pose *= math::Transform {posBone, rotBone};
		pose *= math::Transform {att->offset, uquat::create(att->angles)};
	}
	else {
		auto attPose = mdl->CalcReferenceAttachmentPose(attID);
		if(attPose.has_value())
			pose *= *attPose;
	}
	pose.SetOrigin(pose.GetOrigin() * ent.GetScale());
	if(pos)
		*pos = pose.GetOrigin();
	if(rot)
		*rot = pose.GetRotation();
	return true;
}
bool BaseModelComponent::GetAttachment(const std::string &name, Vector3 *pos, Quat *rot) const
{
	auto &hMdl = GetEntity().GetModel();
	if(hMdl == nullptr)
		return false;
	return GetAttachment(hMdl->LookupAttachment(name), pos, rot);
}

void BaseModelComponent::OnRemove() { BaseEntityComponent::OnRemove(); }

void BaseModelComponent::GetAnimations(Activity activity, std::vector<unsigned int> &animations) const
{
	auto hModel = GetModel();
	if(hModel == nullptr)
		return;
	hModel->GetAnimations(activity, animations);
}

unsigned char BaseModelComponent::GetAnimationActivityWeight(unsigned int animation) const
{
	auto hModel = GetModel();
	if(hModel == nullptr)
		return 0;
	return hModel->GetAnimationActivityWeight(animation);
}
Activity BaseModelComponent::GetAnimationActivity(unsigned int animation) const
{
	auto hModel = GetModel();
	if(hModel == nullptr)
		return Activity::Invalid;
	return hModel->GetAnimationActivity(animation);
}
float BaseModelComponent::GetAnimationDuration(unsigned int animation) const
{
	auto hModel = GetModel();
	if(hModel == nullptr)
		return 0.f;
	return hModel->GetAnimationDuration(animation);
}
int BaseModelComponent::LookupAttachment(const std::string &name) const
{
	auto &hMdl = GetModel();
	if(hMdl == nullptr)
		return -1;
	return hMdl->LookupAttachment(name);
}
int BaseModelComponent::LookupBlendController(const std::string &controller) const
{
	auto hModel = GetModel();
	if(hModel == nullptr)
		return -1;
	return hModel->LookupBlendController(controller);
}
Int32 BaseModelComponent::LookupBone(const std::string &name) const
{
	auto mdl = GetModel();
	if(mdl == nullptr)
		return -1;
	return mdl->LookupBone(name);
}
std::string BaseModelComponent::GetModelName() const
{
	if(m_modelName == nullptr)
		return "";
	return *m_modelName;
}
void BaseModelComponent::SetModelName(const std::string &name) { m_modelName = std::make_unique<std::string>(name); }
bool BaseModelComponent::HasModel() const { return m_model != nullptr; }

const std::vector<uint32_t> &BaseModelComponent::GetBodyGroups() const { return m_bodyGroups; }
UInt32 BaseModelComponent::GetBodyGroup(unsigned int groupId) const
{
	if(groupId >= m_bodyGroups.size())
		return 0;
	return m_bodyGroups[groupId];
}
bool BaseModelComponent::SetBodyGroup(UInt32 groupId, UInt32 id)
{
	auto hMdl = GetModel();
	if(hMdl == nullptr)
		return false;
	auto numGroups = hMdl->GetBodyGroupCount();
	if(groupId >= numGroups)
		return false;
	//if(m_bodyGroups.size() != numGroups)
	//	m_bodyGroups.resize(numGroups); // Already done in BaseEntity::SetModel
	m_bodyGroups[groupId] = id;

	CEOnBodyGroupChanged evData {groupId, id};
	BroadcastEvent(baseModelComponent::EVENT_ON_BODY_GROUP_CHANGED, evData);
	return true;
}
void BaseModelComponent::SetBodyGroup(const std::string &name, UInt32 id)
{
	auto hMdl = GetModel();
	if(hMdl == nullptr)
		return;
	auto groupId = hMdl->GetBodyGroupId(name);
	if(groupId == -1)
		return;
	SetBodyGroup(groupId, id);
}

bool BaseModelComponent::HasModelMaterialsLoaded() const { return m_bMaterialsLoaded; }

void BaseModelComponent::SetModel(const std::string &mdl)
{
	auto *nw = GetEntity().GetNetworkState();
	auto *game = nw->GetGameState();
	auto &mdlManager = nw->GetModelManager();
	auto normalizedMdl = mdlManager.ToCacheIdentifier(mdl);
	if(m_modelName && *m_modelName == normalizedMdl)
		return;
	m_modelName = nullptr;

	if(mdl.empty() == true) {
		if(m_model)
			SetModel(std::shared_ptr<asset::Model> {});
		return;
	}

	m_modelName = std::make_unique<std::string>(normalizedMdl);
	if(!GetEntity().IsSpawned()) {
		mdlManager.PreloadAsset(mdl);
		return;
	}

	auto prevMdl = GetModel();
	auto model = game->LoadModel(*m_modelName);
	if(model == nullptr) {
		model = game->LoadModel("error");
		if(model == nullptr) {
			if(GetModel() == prevMdl) // Model might have been changed during TModelLoader::Load-call in single player (on the client)
				SetModel(std::shared_ptr<asset::Model>(nullptr));
			return;
		}
	}

	if(m_model.get() == model.get())
		return; // Nothing to do

	SetModel(model);
}

void BaseModelComponent::OnModelMaterialsLoaded()
{
	m_bMaterialsLoaded = true;
	BroadcastEvent(baseModelComponent::EVENT_ON_MODEL_MATERIALS_LOADED);
}
const std::shared_ptr<asset::Model> &BaseModelComponent::GetModel() const { return m_model; }
unsigned int BaseModelComponent::GetSkin() const { return *m_skin; }
const std::shared_ptr<util::UInt32Property> &BaseModelComponent::GetSkinProperty() const { return m_skin; }
void BaseModelComponent::SetSkin(unsigned int skin)
{
	if(*m_skin == skin)
		return;
	*m_skin = skin;
	auto &model = GetModel();
	if(model != nullptr)
		model->PrecacheTextureGroup(skin);

	CEOnSkinChanged evData {skin};
	BroadcastEvent(baseModelComponent::EVENT_ON_SKIN_CHANGED, evData);
}

void BaseModelComponent::SetModel(const std::shared_ptr<asset::Model> &mdl)
{
	ClearMembers();

	// If bodygroups have been specified before the entity was spawned, keep them.
	// Otherwise they will be discarded.
	auto keepBodygroups = math::is_flag_set(GetEntity().GetStateFlags(), ecs::BaseEntity::StateFlags::IsSpawning);

	m_model = mdl;
	if(!keepBodygroups)
		m_bodyGroups.clear();
	m_bMaterialsLoaded = true;
	if(m_onModelMaterialsLoaded.IsValid())
		m_onModelMaterialsLoaded.Remove();
	if(mdl == nullptr) {
		if(GetEntity().IsRemoved())
			return;
		m_modelName = nullptr;
		if(!keepBodygroups)
			m_bodyGroups.clear();
		OnModelChanged(nullptr);
		OnMembersChanged();
		return;
	}
	m_bMaterialsLoaded = false;
	m_onModelMaterialsLoaded = mdl->CallOnMaterialsLoaded([this]() { OnModelMaterialsLoaded(); });
	m_bodyGroups.resize(mdl->GetBodyGroupCount());

	ReserveMembers(m_bodyGroups.size());
	for(uint32_t idx = 0; auto &bg : mdl->GetBodyGroups()) {
		auto memberInfo = ComponentMemberInfo::CreateDummy();
		memberInfo.SetName("bodyGroup/" + bg.name);
		memberInfo.type = ents::EntityMemberType::UInt32;
		memberInfo.userIndex = idx++;
		memberInfo.SetMax(bg.meshGroups.size());
		using TValue = uint32_t;
		using TComponent = BaseModelComponent;
		memberInfo
		  .SetGetterFunction<TComponent, TValue, static_cast<void (*)(const ComponentMemberInfo &, TComponent &, TValue &)>([](const ComponentMemberInfo &memberInfo, TComponent &component, TValue &outValue) { outValue = component.GetBodyGroup(memberInfo.userIndex); })>();
		memberInfo.SetSetterFunction<TComponent, TValue,
		  static_cast<void (*)(const ComponentMemberInfo &, TComponent &, const TValue &)>([](const ComponentMemberInfo &memberInfo, TComponent &component, const TValue &value) { component.SetBodyGroup(memberInfo.userIndex, value); })>();
		RegisterMember(std::move(memberInfo));
	}
	OnMembersChanged();

	/*if(skeleton == nullptr)
	{
		UpdateChildParentInfo();
		return;
	}*/
	SetSkin(GetSkin());

	OnModelChanged(mdl);
}

void BaseModelComponent::OnModelChanged(const std::shared_ptr<asset::Model> &model)
{
	CEOnModelChanged evData {model};
	BroadcastEvent(baseModelComponent::EVENT_ON_MODEL_CHANGED, evData);
}

const ComponentMemberInfo *BaseModelComponent::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto numStatic = GetStaticMemberCount();
	if(idx < numStatic)
		return BaseEntityComponent::GetMemberInfo(idx);
	return DynamicMemberRegister::GetMemberInfo(idx);
}

std::optional<ComponentMemberIndex> BaseModelComponent::DoGetMemberIndex(const std::string &name) const
{
	auto idx = BaseEntityComponent::DoGetMemberIndex(name);
	if(idx.has_value())
		return idx;
	idx = DynamicMemberRegister::GetMemberIndex(name);
	if(idx.has_value())
		return *idx; // +GetStaticMemberCount();
	return std::optional<ComponentMemberIndex> {};
}

uint32_t BaseModelComponent::GetHitboxCount() const
{
	auto &mdl = GetModel();
	if(mdl == nullptr)
		return 0;
	return mdl.get()->GetHitboxCount();
}
void BaseModelComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(IntersectionHandlerComponent))
		m_intersectionHandlerComponent = static_cast<IntersectionHandlerComponent *>(&component);
}
void BaseModelComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(IntersectionHandlerComponent))
		m_intersectionHandlerComponent = nullptr;
}
const BaseBvhComponent *BaseModelComponent::GetBvhComponent() const { return const_cast<BaseModelComponent *>(this)->GetBvhComponent(); }
BaseBvhComponent *BaseModelComponent::GetBvhComponent() { return m_bvhComponent; }
const IntersectionHandlerComponent *BaseModelComponent::GetIntersectionHandlerComponent() const { return const_cast<BaseModelComponent *>(this)->GetIntersectionHandlerComponent(); }
IntersectionHandlerComponent *BaseModelComponent::GetIntersectionHandlerComponent() { return m_intersectionHandlerComponent; }
bool BaseModelComponent::GetHitboxBounds(uint32_t boneId, Vector3 &min, Vector3 &max, Vector3 &origin, Quat &rot, math::CoordinateSpace space) const
{
	if(HasModel() == false) {
		min = Vector3 {0.f, 0.f, 0.f};
		max = Vector3 {0.f, 0.f, 0.f};
		origin = Vector3 {0.f, 0.f, 0.f};
		rot = uquat::identity();
		return false;
	}
	GetModel().get()->GetHitboxBounds(boneId, min, max);
	auto &ent = GetEntity();
	auto animComponent = ent.GetAnimatedComponent();
	auto &processedBones = animComponent->GetProcessedBones();
	if(animComponent.expired() || boneId >= processedBones.size())
		return false;
	switch(space) {
	case math::CoordinateSpace::Local:
		{
			origin = uvec::PRM_ORIGIN;
			rot = uquat::identity();
			break;
		}
	case math::CoordinateSpace::Object:
		{
			auto &pose = processedBones[boneId];
			origin = pose.GetOrigin();
			rot = pose.GetRotation();

			min *= pose.GetScale();
			max *= pose.GetScale();
			break;
		}
	case math::CoordinateSpace::World:
		{
			auto pose = ent.GetPose() * processedBones[boneId];
			origin = pose.GetOrigin();
			rot = pose.GetRotation();

			min *= pose.GetScale();
			max *= pose.GetScale();
			break;
		}
	}
	return true;
}
uint32_t BaseModelComponent::GetFlexControllerCount() const
{
	auto &hMdl = GetModel();
	return (hMdl != nullptr) ? hMdl->GetFlexControllerCount() : 0u;
}
int BaseModelComponent::LookupAnimation(const std::string &name) const
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return -1;
	return hModel->LookupAnimation(name);
}
bool BaseModelComponent::LookupFlexController(const std::string &name, uint32_t &flexId) const
{
	auto &hMdl = GetModel();
	if(hMdl == nullptr)
		return false;
	return hMdl->GetFlexControllerId(name, flexId);
}
void BaseModelComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);

	udm["modelName"] = GetModelName();
	udm["skin"] = GetSkin();

	// Write body groups
	auto &bodyGroups = GetBodyGroups();
	udm["bodyGroups"] = bodyGroups;
}
void BaseModelComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);

	std::string modelName;
	udm["modelName"](modelName);
	if(modelName.empty() == false)
		SetModel(modelName);

	uint32_t skin = 0;
	udm["skin"](skin);
	SetSkin(skin);

	// Read body groups
	std::vector<uint32_t> bodyGroups;
	udm["bodyGroups"](bodyGroups);
	for(auto i = decltype(bodyGroups.size()) {0u}; i < bodyGroups.size(); ++i)
		SetBodyGroup(i, bodyGroups[i]);
}

///////////////

CEOnBodyGroupChanged::CEOnBodyGroupChanged(uint32_t groupId, uint32_t typeId) : groupId {groupId}, typeId {typeId} {}
void CEOnBodyGroupChanged::PushArguments(lua::State *l)
{
	Lua::PushInt(l, groupId);
	Lua::PushInt(l, typeId);
}

///////////////

CEOnSkinChanged::CEOnSkinChanged(uint32_t skinId) : skinId {skinId} {}
void CEOnSkinChanged::PushArguments(lua::State *l) { Lua::PushInt(l, skinId); }

///////////////

CEOnModelChanged::CEOnModelChanged(const std::shared_ptr<asset::Model> &model) : model {model} {}
void CEOnModelChanged::PushArguments(lua::State *l) { Lua::Push<std::shared_ptr<asset::Model>>(l, model); }
