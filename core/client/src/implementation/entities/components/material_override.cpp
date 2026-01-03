// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.material_override;

using namespace pragma;
ComponentEventId cMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDES_CLEARED = INVALID_COMPONENT_ID;
ComponentEventId cMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDE_CHANGED = INVALID_COMPONENT_ID;
void CMaterialOverrideComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseModelComponent::RegisterEvents(componentManager, registerEvent);
	cMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDES_CLEARED = registerEvent("EVENT_ON_MATERIAL_OVERRIDES_CLEARED", ComponentEventInfo::Type::Broadcast);
	cMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDE_CHANGED = registerEvent("EVENT_ON_MATERIAL_OVERRIDE_CHANGED", ComponentEventInfo::Type::Broadcast);
}

CMaterialOverrideComponent::CMaterialOverrideComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}

CMaterialOverrideComponent::~CMaterialOverrideComponent() {}
void CMaterialOverrideComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CMaterialOverrideComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(cModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		m_materialOverrides.clear();
		if(GetEntity().IsSpawned())
			PopulateProperties();
	});
}

void CMaterialOverrideComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	PopulateProperties();
}

void CMaterialOverrideComponent::OnRemove()
{
	m_materialOverrides.clear();
	BaseEntityComponent::OnRemove();
	if(GetEntity().IsRemoved())
		return;
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(!mdlC)
		return;
	mdlC->SetRenderBufferListUpdateRequired();
	mdlC->UpdateRenderBufferList();
}

void CMaterialOverrideComponent::SetMaterialOverride(uint32_t idx, const std::string &matOverride)
{
	auto *mat = GetNetworkState().LoadMaterial(matOverride);
	if(mat)
		SetMaterialOverride(idx, static_cast<material::CMaterial &>(*mat));
	else
		ClearMaterialOverride(idx);
}
void CMaterialOverrideComponent::SetMaterialOverride(uint32_t idx, material::CMaterial &mat)
{
	if(idx >= m_materialOverrides.size())
		m_materialOverrides.resize(idx + 1);
	m_materialOverrides.at(idx).materialOverride = mat.GetHandle();
	UpdateMaterialOverride(idx, mat);
}
void CMaterialOverrideComponent::UpdateMaterialOverride(uint32_t matIdx, material::CMaterial &mat)
{
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC)
		mdlC->SetRenderBufferListUpdateRequired();
	static_cast<material::CMaterial &>(mat).LoadTextures(false, true);
	mat.UpdateTextures(true); // Ensure all textures have been fully loaded
	if(mdlC)
		mdlC->UpdateRenderBufferList();

	BroadcastEvent(cMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDE_CHANGED, CEOnMaterialOverrideChanged {matIdx, mat});
}
void CMaterialOverrideComponent::ClearMaterialOverride(uint32_t idx)
{
	if(idx >= m_materialOverrides.size())
		return;
	m_materialOverrides.at(idx).materialOverride = {};
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC) {
		mdlC->SetRenderBufferListUpdateRequired();
		mdlC->UpdateRenderBufferList();
	}
}
void CMaterialOverrideComponent::ClearMaterialOverrides()
{
	if(m_materialOverrides.empty())
		return;
	for(auto &mo : m_materialOverrides)
		mo.materialOverride = {};
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC)
		mdlC->SetRenderBufferListUpdateRequired();
	BroadcastEvent(cMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDES_CLEARED);
}
material::CMaterial *CMaterialOverrideComponent::GetMaterialOverride(uint32_t idx) const { return (idx < m_materialOverrides.size()) ? const_cast<material::CMaterial *>(static_cast<const material::CMaterial *>(m_materialOverrides.at(idx).materialOverride.get())) : nullptr; }

const ComponentMemberInfo *CMaterialOverrideComponent::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto numStatic = GetStaticMemberCount();
	if(idx < numStatic)
		return BaseEntityComponent::GetMemberInfo(idx);
	return DynamicMemberRegister::GetMemberInfo(idx);
}

std::optional<ComponentMemberIndex> CMaterialOverrideComponent::DoGetMemberIndex(const std::string &name) const
{
	auto idx = BaseEntityComponent::DoGetMemberIndex(name);
	if(idx.has_value())
		return idx;
	idx = DynamicMemberRegister::GetMemberIndex(name);
	if(idx.has_value())
		return *idx; // +GetStaticMemberCount();
	return std::optional<ComponentMemberIndex> {};
}

void CMaterialOverrideComponent::PopulateProperties()
{
	ClearMembers();
	util::ScopeGuard sg {[this]() { OnMembersChanged(); }};

	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return;

	auto &mats = mdl->GetMaterials();
	ReserveMembers(mats.size());
	for(size_t matIdx = 0; matIdx < mats.size(); ++matIdx) {
		auto &mat = mats[matIdx];
		if(!mat)
			continue;
		auto matName = std::string {util::FilePath(mat->GetName()).GetFileName()};
		string::to_lower(matName);

		auto propName = matName;
		auto *cPropName = register_global_string(propName);

		auto memberInfo = ComponentMemberInfo::CreateDummy();

		auto &metaData = memberInfo.AddMetaData();
		metaData["assetType"] = "material";
		metaData["rootPath"] = util::Path::CreatePath(pragma::asset::get_asset_root_directory(asset::Type::Material)).GetString();
		metaData["extensions"] = pragma::asset::get_supported_extensions(asset::Type::Material, asset::FormatType::All);
		metaData["stripRootPath"] = true;
		metaData["stripExtension"] = true;

		memberInfo.SetSpecializationType(AttributeSpecializationType::File);
		memberInfo.SetName(matName);
		memberInfo.type = ents::EntityMemberType::String;
		memberInfo.SetDefault<std::string>("");
		memberInfo.userIndex = matIdx;
		using TComponent = CMaterialOverrideComponent;
		using TValue = udm::String;
		memberInfo.SetGetterFunction<TComponent, TValue, +[](const ComponentMemberInfo &memberInfo, TComponent &component, TValue &outValue) {
			auto *matOverride = component.GetMaterialOverride(memberInfo.userIndex);
			if(!matOverride) {
				outValue = {};
				return;
			}
			outValue = util::FilePath(matOverride->GetName()).GetFileName();
		}>();
		memberInfo.SetSetterFunction<TComponent, TValue, +[](const ComponentMemberInfo &memberInfo, TComponent &component, const TValue &value) {
			if(value.empty()) {
				component.ClearMaterialOverride(memberInfo.userIndex);
				return;
			}
			component.SetMaterialOverride(memberInfo.userIndex, value);
		}>();
		RegisterMember(std::move(memberInfo));
	}
}

material::CMaterial *CMaterialOverrideComponent::GetRenderMaterial(uint32_t idx) const
{
	auto c = GetEntity().GetComponent<CMaterialPropertyOverrideComponent>();
	if(c.valid()) {
		auto *mat = c->GetRenderMaterial(idx);
		if(mat)
			return mat;
	}
	if(idx >= m_materialOverrides.size())
		return nullptr;
	auto &matOverride = m_materialOverrides[idx];
	return const_cast<material::CMaterial *>(static_cast<const material::CMaterial *>(matOverride.materialOverride.get()));
}

size_t CMaterialOverrideComponent::GetMaterialOverrideCount() const { return m_materialOverrides.size(); }

void CMaterialOverrideComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	auto def = pragma::LuaCore::create_entity_component_class<CMaterialOverrideComponent, BaseEntityComponent>("MaterialOverrideComponent");
	def.add_static_constant("EVENT_ON_MATERIAL_OVERRIDES_CLEARED", cMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDES_CLEARED);
	def.def("SetMaterialOverride", static_cast<void (CMaterialOverrideComponent::*)(uint32_t, const std::string &)>(&CMaterialOverrideComponent::SetMaterialOverride));
	def.def("SetMaterialOverride", static_cast<void (CMaterialOverrideComponent::*)(uint32_t, material::CMaterial &)>(&CMaterialOverrideComponent::SetMaterialOverride));
	def.def(
	  "SetMaterialOverride", +[](lua::State *l, CMaterialOverrideComponent &hModel, const std::string &matSrc, const std::string &matDst) {
		  auto &mdl = hModel.GetEntity().GetModel();
		  if(!mdl)
			  return;
		  auto &mats = mdl->GetMaterials();
		  auto it = std::find_if(mats.begin(), mats.end(), [&matSrc](const material::MaterialHandle &hMat) {
			  if(!hMat)
				  return false;
			  return pragma::asset::matches(const_cast<material::Material *>(hMat.get())->GetName(), matSrc, asset::Type::Material);
		  });
		  if(it == mats.end())
			  return;
		  hModel.SetMaterialOverride(it - mats.begin(), matDst);
	  });
	def.def(
	  "SetMaterialOverride", +[](lua::State *l, CMaterialOverrideComponent &hModel, const std::string &matSrc, material::CMaterial &matDst) {
		  auto &mdl = hModel.GetEntity().GetModel();
		  if(!mdl)
			  return;
		  auto &mats = mdl->GetMaterials();
		  auto it = std::find_if(mats.begin(), mats.end(), [&matSrc](const material::MaterialHandle &hMat) {
			  if(!hMat)
				  return false;
			  return pragma::asset::matches(const_cast<material::Material *>(hMat.get())->GetName(), matSrc, asset::Type::Material);
		  });
		  if(it == mats.end())
			  return;
		  hModel.SetMaterialOverride(it - mats.begin(), matDst);
	  });
	def.def(
	  "ClearMaterialOverride", +[](lua::State *l, CMaterialOverrideComponent &hModel, const std::string &matSrc) {
		  auto &mdl = hModel.GetEntity().GetModel();
		  if(!mdl)
			  return;
		  auto &mats = mdl->GetMaterials();
		  auto it = std::find_if(mats.begin(), mats.end(), [&matSrc](const material::MaterialHandle &hMat) {
			  if(!hMat)
				  return false;
			  return pragma::asset::matches(const_cast<material::Material *>(hMat.get())->GetName(), matSrc, asset::Type::Material);
		  });
		  if(it == mats.end())
			  return;
		  hModel.ClearMaterialOverride(it - mats.begin());
	  });
	def.def("ClearMaterialOverride", &CMaterialOverrideComponent::ClearMaterialOverride);
	def.def("ClearMaterialOverrides", &CMaterialOverrideComponent::ClearMaterialOverrides);
	def.def("GetMaterialOverride", &CMaterialOverrideComponent::GetMaterialOverride);
	def.def("GetMaterialOverrideCount", &CMaterialOverrideComponent::GetMaterialOverrideCount);
	def.def("GetRenderMaterial", &CMaterialOverrideComponent::GetRenderMaterial);
	modEnts[def];
}

////////////

CEOnMaterialOverrideChanged::CEOnMaterialOverrideChanged(uint32_t idx, material::CMaterial &mat) : materialIndex {idx}, material {mat} {}
void CEOnMaterialOverrideChanged::PushArguments(lua::State *l) { Lua::PushInt(l, materialIndex); }
