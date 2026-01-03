// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.member_info;

using namespace pragma;

static ComponentEventId g_nextEventId = 0u;
static std::unordered_map<std::string, ComponentEventId> g_componentEventIds;

static ComponentEventId get_component_event_id(const std::string &name)
{
	auto it = g_componentEventIds.find(name);
	if(it != g_componentEventIds.end())
		return it->second;
	auto id = g_nextEventId++;
	g_componentEventIds[name] = id;
	return id;
}

size_t pragma::get_component_member_name_hash(const char *name) { return get_component_member_name_hash(std::string {name}); }
size_t pragma::get_component_member_name_hash(const std::string &name)
{
	auto lname = name;
	// pragma::string::to_lower(lname);
	return std::hash<std::string> {}(lname);
}
std::string pragma::get_normalized_component_member_name(const std::string &name)
{
	auto lname = name;
	// pragma::string::to_lower(lname);
	return lname;
}
ComponentMemberInfo::ComponentMemberInfo() : m_specializationType {AttributeSpecializationType::None} {}
ComponentMemberInfo::ComponentMemberInfo(const ComponentMemberInfo &other) { operator=(other); }
ComponentMemberInfo &ComponentMemberInfo::operator=(const ComponentMemberInfo &other)
{
	// We need to make sure these are cleared before changing the type
	m_default = std::unique_ptr<void, void (*)(void *)> {nullptr, [](void *) {}};
	m_customSpecializationType = nullptr;

	m_metaData = nullptr;
	m_enumConverter = nullptr;

	type = other.type;
	setterFunction = other.setterFunction;
	getterFunction = other.getterFunction;
	interpolationFunction = other.interpolationFunction;
	updateDependenciesFunction = other.updateDependenciesFunction;
	userIndex = other.userIndex;
	m_name = other.m_name;
	m_nameHash = other.m_nameHash;
	m_specializationType = other.m_specializationType;
	m_customSpecializationType = other.m_customSpecializationType ? std::make_unique<std::string>(*other.m_customSpecializationType) : nullptr;
	m_typeMetaData = other.m_typeMetaData;
	m_metaData = other.m_metaData;
	m_flags = other.m_flags;
	m_enumConverter = other.m_enumConverter ? std::make_unique<EnumConverter>(*other.m_enumConverter) : nullptr;
	if(other.m_default) {
		// Default value is currently only allowed for UDM types. Tag: component-member-udm-default
		if(ents::is_udm_member_type(type)) {
			udm::visit(ents::member_type_to_udm_type(type), [this, &other](auto tag) {
				using T = typename decltype(tag)::type;
				constexpr auto eType = udm::type_to_enum<T>();
				if constexpr(eType != udm::Type::Element && !udm::is_array_type(eType))
					SetDefault<T>(*static_cast<T *>(other.m_default.get()));
			});
		}
	}
	static_assert(sizeof(*this) == 144);
	return *this;
}
void ComponentMemberInfo::SetSpecializationType(AttributeSpecializationType type)
{
	m_specializationType = type;
	m_customSpecializationType = nullptr;
}
void ComponentMemberInfo::SetSpecializationType(std::string customType)
{
	m_specializationType = AttributeSpecializationType::Custom;
	m_customSpecializationType = std::make_unique<std::string>(customType);
}
void ComponentMemberInfo::SetMin(float min)
{
	auto &metaData = AddTypeMetaData<ents::RangeTypeMetaData>();
	metaData.min = min;
}
void ComponentMemberInfo::SetMax(float max)
{
	auto &metaData = AddTypeMetaData<ents::RangeTypeMetaData>();
	metaData.max = max;
}
void ComponentMemberInfo::SetStepSize(float stepSize)
{
	auto &metaData = AddTypeMetaData<ents::RangeTypeMetaData>();
	metaData.stepSize = stepSize;
}
std::optional<float> ComponentMemberInfo::GetMin() const
{
	auto *metaData = FindTypeMetaData<ents::RangeTypeMetaData>();
	return metaData ? metaData->min : std::optional<float> {};
}
std::optional<float> ComponentMemberInfo::GetMax() const
{
	auto *metaData = FindTypeMetaData<ents::RangeTypeMetaData>();
	return metaData ? metaData->max : std::optional<float> {};
}
std::optional<float> ComponentMemberInfo::GetStepSize() const
{
	auto *metaData = FindTypeMetaData<ents::RangeTypeMetaData>();
	return metaData ? metaData->stepSize : std::optional<float> {};
}
udm::Property &ComponentMemberInfo::AddMetaData()
{
	auto prop = udm::Property::Create(udm::Type::Element);
	AddMetaData(prop);
	return *prop;
}
void ComponentMemberInfo::AddMetaData(const udm::PProperty &prop) { m_metaData = prop; }
const udm::PProperty &ComponentMemberInfo::GetMetaData() const { return m_metaData; }
void ComponentMemberInfo::AddTypeMetaData(const std::shared_ptr<ents::TypeMetaData> &typeMetaData) { m_typeMetaData.push_back(typeMetaData); }
const ents::TypeMetaData *ComponentMemberInfo::FindTypeMetaData(std::type_index typeId) const
{
	auto it = std::find_if(m_typeMetaData.begin(), m_typeMetaData.end(), [typeId](const std::shared_ptr<ents::TypeMetaData> &typeMetaData) { return typeId == typeid(*typeMetaData); });
	return (it != m_typeMetaData.end()) ? it->get() : nullptr;
}
void ComponentMemberInfo::SetEnum(const EnumConverter::NameToEnumFunction &nameToEnum, const EnumConverter::EnumToNameFunction &enumToName, const EnumConverter::EnumValueGetFunction &getValues) { m_enumConverter = std::make_unique<EnumConverter>(nameToEnum, enumToName, getValues); }
bool ComponentMemberInfo::IsEnum() const { return m_enumConverter != nullptr; }
std::optional<int64_t> ComponentMemberInfo::EnumNameToValue(const std::string &name) const
{
	if(!m_enumConverter)
		return {};
	return m_enumConverter->nameToEnum(name);
}
bool ComponentMemberInfo::GetEnumValues(std::vector<int64_t> &outValues) const
{
	if(!m_enumConverter)
		return false;
	outValues = std::move(m_enumConverter->getValues());
	return true;
}
std::optional<std::string> ComponentMemberInfo::ValueToEnumName(int64_t value) const
{
	if(!m_enumConverter)
		return {};
	return m_enumConverter->enumToName(value);
}
void ComponentMemberInfo::UpdateDependencies(BaseEntityComponent &component, std::vector<std::string> &outAffectedProps)
{
	if(!updateDependenciesFunction)
		return;
	updateDependenciesFunction(component, outAffectedProps);
}
void ComponentMemberInfo::ResetToDefault(BaseEntityComponent &component)
{
	if(!m_default)
		return;
	ents::visit_member(type, [this, &component](auto tag) {
		using T = typename decltype(tag)::type;
		setterFunction(*this, component, m_default.get());
	});
}
void ComponentMemberInfo::SetFlags(ComponentMemberFlags flags) { m_flags = flags; }
ComponentMemberFlags ComponentMemberInfo::GetFlags() const { return m_flags; }
bool ComponentMemberInfo::HasFlag(ComponentMemberFlags flag) const { return math::is_flag_set(m_flags, flag); }
void ComponentMemberInfo::SetFlag(ComponentMemberFlags flag, bool set) { math::set_flag(m_flags, flag, set); }
void ComponentMemberInfo::SetName(const GString &name)
{
	m_name = name;
	m_nameHash = get_component_member_name_hash(*name);
}

//////////////

ComponentRegInfo::ComponentRegInfo(GString category, Flags flags) : categoryPath {category}, flags {flags} {}
ComponentRegInfo::ComponentRegInfo(Flags flags) : ComponentRegInfo {"internal", flags} {}

ComponentInfo::ComponentInfo(const ComponentInfo &other) { operator=(other); }
ComponentInfo::ComponentInfo(ComponentInfo &&other) { operator=(std::move(other)); }
ComponentInfo &ComponentInfo::operator=(const ComponentInfo &other)
{
	name = other.name;
	factory = other.factory;
	id = other.id;
	flags = other.flags;
	members = other.members;
	memberNameToIndex = other.memberNameToIndex;
	onCreateCallbacks = nullptr;
	if(other.onCreateCallbacks) {
		onCreateCallbacks = std::make_unique<std::vector<CallbackHandle>>();
		onCreateCallbacks->reserve(other.onCreateCallbacks->size());
		for(auto &cb : *other.onCreateCallbacks)
			onCreateCallbacks->push_back(cb);
	}
#ifdef _MSVC_VER
	static_assert(sizeof(*this) == 200);
#endif
	return *this;
}
ComponentInfo &ComponentInfo::operator=(ComponentInfo &&other)
{
	name = std::move(other.name);
	factory = std::move(other.factory);
	id = other.id;
	flags = other.flags, members = std::move(other.members);
	memberNameToIndex = std::move(other.memberNameToIndex);
	onCreateCallbacks = std::move(other.onCreateCallbacks);
#ifdef _MSVC_VER
	static_assert(sizeof(*this) == 200);
#endif
	return *this;
}
std::optional<ComponentMemberIndex> ComponentInfo::FindMember(const std::string &name) const
{
	auto hash = get_component_member_name_hash(name);
	auto it = std::find_if(members.begin(), members.end(), [hash](const ComponentMemberInfo &memberInfo) { return memberInfo.GetNameHash() == hash; });
	return (it != members.end()) ? (it - members.begin()) : std::optional<ComponentMemberIndex> {};
}

//////////////

util::TSharedHandle<BaseEntityComponent> EntityComponentManager::CreateComponent(ComponentId componentId, ecs::BaseEntity &ent) const
{
	if(componentId >= m_componentInfos.size() || m_componentInfos[componentId]->id == INVALID_COMPONENT_ID) {
		// Component has been pre-registered, but its script has not yet been loaded!
		// 'info'-members will not be valid, so we have to retrieve the component information
		// (name, etc.) from the pre-register data
		auto it = std::find_if(m_preRegistered.begin(), m_preRegistered.end(), [componentId](const std::unique_ptr<ComponentInfo> &componentInfo) { return componentInfo->id == componentId; });
		if(it == m_preRegistered.end()) {
			// Component has NOT been pre-registered? Then where did the component id come from?
			// This should be unreachable!
			Con::CERR << "Attempted to create unknown component '" << componentId << "'!" << Con::endl;
			return nullptr;
		}
		auto &preRegInfo = *it;
		// Attempt to create the component by name instead.
		// This will automatically attempt to load the appropriate
		// component script.
		std::string name = preRegInfo->name; // Name has to be copied, because pre-register information may be invalidated by the 'CreateComponent'-call
		return CreateComponent(name, ent);
	}
	auto &info = m_componentInfos.at(componentId);
	auto r = info->factory(ent);
	if(r == nullptr)
		return nullptr;
	r->m_componentId = info->id;
	m_components.at(r->m_componentId).Push(*r);
	if(info->onCreateCallbacks) {
		for(auto it = info->onCreateCallbacks->begin(); it != info->onCreateCallbacks->end();) {
			auto &cb = *it;
			if(cb.IsValid())
				cb.Call<void, std::reference_wrapper<BaseEntityComponent>>(*r);
			if(cb.IsValid() == false) {
				it = info->onCreateCallbacks->erase(it);
				continue;
			}
			++it;
		}
		if(info->onCreateCallbacks->empty())
			info->onCreateCallbacks = nullptr;
	}
	return r;
}
util::TSharedHandle<BaseEntityComponent> EntityComponentManager::CreateComponent(const std::string &name, ecs::BaseEntity &ent) const
{
	auto componentId = INVALID_COMPONENT_ID;
	// Try to find the component as Lua component and load it (if the component hasn't been registered yet)
	if(GetComponentTypeId(name, componentId, false) == false && (ent.GetNetworkState()->GetGameState()->LoadLuaComponentByName(name) == false || GetComponentTypeId(name, componentId, false) == false))
		return nullptr;
	auto *componentInfo = GetComponentInfo(componentId);
	if(componentInfo == nullptr || componentInfo->IsValid() == false)
		return nullptr;
	return CreateComponent(componentInfo->id, ent);
}
ComponentId EntityComponentManager::PreRegisterComponentType(const std::string &name)
{
	auto componentId = INVALID_COMPONENT_ID;
	if(GetComponentTypeId(name, componentId))
		return componentId;
	m_preRegistered.push_back(std::make_unique<ComponentInfo>());
	std::string lname = name;
	string::to_lower(lname);
	auto &componentInfo = *m_preRegistered.back();
	componentInfo.name = lname;
	componentInfo.id = m_nextComponentId++;
	if(componentInfo.id != m_components.size())
		throw std::logic_error("Newly registered component id does not match expected component type count!");
	if(m_components.size() == m_components.capacity())
		m_components.reserve(m_components.size() + 50);
	m_components.push_back({});
	return componentInfo.id;
}
ComponentId EntityComponentManager::RegisterComponentType(const std::string &name, const std::function<util::TSharedHandle<BaseEntityComponent>(ecs::BaseEntity &)> &factory, const ComponentRegInfo &regInfo, ComponentFlags flags, std::type_index typeIndex)
{
	return RegisterComponentType(name, factory, regInfo, flags, &typeIndex);
}
ComponentId EntityComponentManager::RegisterComponentType(const std::string &name, const std::function<util::TSharedHandle<BaseEntityComponent>(ecs::BaseEntity &)> &factory, const ComponentRegInfo &regInfo, ComponentFlags flags)
{
	return RegisterComponentType(name, factory, regInfo, flags, nullptr);
}
void EntityComponentManager::LinkComponentType(ComponentId linkFrom, ComponentId linkTo)
{
	auto it = m_linkedComponentTypes.find(linkFrom);
	if(it == m_linkedComponentTypes.end())
		it = m_linkedComponentTypes.insert(std::make_pair(linkFrom, std::vector<ComponentTypeLinkInfo> {})).first;
	auto &infos = it->second;
	auto itV = std::find_if(infos.begin(), infos.end(), [linkTo](const ComponentTypeLinkInfo &linkInfo) { return linkInfo.targetType == linkTo; });
	if(itV != infos.end())
		return;
	infos.push_back({});
	auto &linkInfo = infos.back();
	linkInfo.targetType = linkTo;
	linkInfo.onCreateCallback = AddCreationCallback(linkTo, [linkFrom](std::reference_wrapper<BaseEntityComponent> c) { c.get().GetEntity().AddComponent(linkFrom); });
}
CallbackHandle EntityComponentManager::AddCreationCallback(ComponentId componentId, const std::function<void(std::reference_wrapper<BaseEntityComponent>)> &onCreate)
{
	auto *info = GetComponentInfo(componentId);
	if(!info) {
		auto it = std::find_if(m_preRegistered.begin(), m_preRegistered.end(), [componentId](const std::unique_ptr<ComponentInfo> &componentInfo) { return componentInfo->id == componentId; });
		if(it == m_preRegistered.end())
			throw std::runtime_error {"Invalid component (" + std::to_string(componentId) + ")"};
		info = it->get();
	}
	if(!info->onCreateCallbacks)
		info->onCreateCallbacks = std::make_unique<std::vector<CallbackHandle>>();
	auto cb = FunctionCallback<void, std::reference_wrapper<BaseEntityComponent>>::Create(onCreate);
	info->onCreateCallbacks->push_back(cb);
	return cb;
}
CallbackHandle EntityComponentManager::AddCreationCallback(const std::string &componentName, const std::function<void(std::reference_wrapper<BaseEntityComponent>)> &onCreate)
{
	ComponentId id;
	if(!GetComponentTypeId(componentName, id))
		id = PreRegisterComponentType(componentName);
	return AddCreationCallback(id, onCreate);
}
ComponentId EntityComponentManager::RegisterComponentType(const std::string &name, const std::function<util::TSharedHandle<BaseEntityComponent>(ecs::BaseEntity &)> &factory, const ComponentRegInfo &regInfo, ComponentFlags flags, const std::type_index *typeIndex)
{
	if(typeIndex != nullptr) {
		auto it = m_typeIndexToComponentId.find(*typeIndex);
		if(it != m_typeIndexToComponentId.end())
			throw std::runtime_error("Attempted to register component '" + name + "' which has already been registered previously!");
	}
	auto componentId = INVALID_COMPONENT_ID;
	if(GetComponentTypeId(name, componentId, false)) {
		// Overwrite previous definition
		auto &pComponentInfo = *GetComponentInfo(componentId);
		pComponentInfo.factory = factory;
		pComponentInfo.flags = flags;
		OnComponentTypeRegistered(pComponentInfo);
		return componentId;
	}
	auto idx = PreRegisterComponentType(name);
	auto itPre = std::find_if(m_preRegistered.begin(), m_preRegistered.end(), [&name](const std::unique_ptr<ComponentInfo> &componentInfo) { return string::compare(name.c_str(), componentInfo->name.c_str(), false); });
	if(itPre == m_preRegistered.end())
		throw std::logic_error("Error when attempting to pre-register component " + name + "!");
	auto &preReg = *itPre;
	componentId = preReg->id;
	if(componentId >= m_componentInfos.size()) {
		auto offset = m_componentInfos.size();
		m_componentInfos.resize(componentId + 1u);
		for(size_t i = offset; i < m_componentInfos.size(); ++i) {
			auto &info = m_componentInfos[i];
			info = std::make_unique<ComponentInfo>();
		}
	}
	auto &componentInfo = m_componentInfos.at(componentId) = std::move(preReg);
	if(typeIndex != nullptr) {
		m_typeIndexToComponentId.insert(std::make_pair(*typeIndex, componentId));
		if(componentId >= m_componentIdToTypeIndex.size())
			m_componentIdToTypeIndex.resize(componentId + 1u, nullptr);
		m_componentIdToTypeIndex.at(componentId) = pragma::util::make_shared<std::type_index>(*typeIndex);
	}
	m_preRegistered.erase(itPre);

	if(math::is_flag_set(regInfo.flags, ComponentRegInfo::Flags::HideInEditor))
		flags |= ComponentFlags::HideInEditor;

	componentInfo->factory = factory;
	componentInfo->flags = flags;
	componentInfo->category = regInfo.categoryPath;
	OnComponentTypeRegistered(*componentInfo);
	return componentInfo->id;
}
bool EntityComponentManager::GetComponentTypeIndex(ComponentId componentId, std::type_index &typeIndex) const
{
	if(componentId >= m_componentIdToTypeIndex.size())
		return false;
	auto &pTypeIndex = m_componentIdToTypeIndex.at(componentId);
	if(pTypeIndex == nullptr)
		return false;
	typeIndex = *pTypeIndex;
	return true;
}
bool EntityComponentManager::GetComponentId(std::type_index typeIndex, ComponentId &componentId) const
{
	auto it = m_typeIndexToComponentId.find(typeIndex);
	if(it == m_typeIndexToComponentId.end())
		return false;
	componentId = it->second;
	return true;
}
bool EntityComponentManager::GetComponentTypeId(const std::string &name, ComponentId &outId, bool bIncludePreregistered) const
{
	if(bIncludePreregistered == true) {
		auto itPre = std::find_if(m_preRegistered.begin(), m_preRegistered.end(), [&name](const std::unique_ptr<ComponentInfo> &componentInfo) { return string::compare(name.c_str(), componentInfo->name.c_str()); });
		if(itPre != m_preRegistered.end()) {
			outId = (*itPre)->id;
			return true;
		}
	}
	auto it = std::find_if(m_componentInfos.begin(), m_componentInfos.end(), [&name](const std::unique_ptr<ComponentInfo> &componentInfo) { return string::compare(name.c_str(), componentInfo->name.c_str(), false); });
	if(it == m_componentInfos.end())
		return false;
	outId = (*it)->id;
	return true;
}
const ComponentInfo *EntityComponentManager::GetComponentInfo(ComponentId id) const { return const_cast<EntityComponentManager *>(this)->GetComponentInfo(id); }
ComponentInfo *EntityComponentManager::GetComponentInfo(ComponentId id)
{
	if(id >= m_componentInfos.size())
		return nullptr;
	return m_componentInfos.at(id).get();
}
ComponentMemberIndex EntityComponentManager::RegisterMember(ComponentInfo &componentInfo, ComponentMemberInfo &&memberInfo)
{
	std::string lname = memberInfo.GetName();
	string::to_lower(lname);
	componentInfo.members.push_back(std::move(memberInfo));
	auto idx = componentInfo.members.size() - 1;
	componentInfo.memberNameToIndex[lname] = idx;
	return idx;
}
const std::vector<std::unique_ptr<ComponentInfo>> &EntityComponentManager::GetRegisteredComponentTypes() const { return m_componentInfos; }
void EntityComponentManager::OnComponentTypeRegistered(const ComponentInfo &componentInfo) {}
ComponentEventId EntityComponentManager::RegisterEvent(const std::string &evName, std::type_index typeIndex, ComponentEventInfo::Type type)
{
	auto it = std::find_if(m_componentEvents.begin(), m_componentEvents.end(), [&evName](const std::pair<const ComponentEventId, ComponentEventInfo> &pair) { return evName == pair.second.name; });
	if(it == m_componentEvents.end()) {
		auto id = get_component_event_id(evName);
		it = m_componentEvents.insert(std::make_pair(id, ComponentEventInfo {evName, {}, typeIndex, type})).first;
		it->second.id = id;
	}
	return it->first;
}
std::optional<ComponentEventId> EntityComponentManager::FindEventId(const std::string &componentName, const std::string &evName) const
{
	ComponentId componentId;
	if(GetComponentTypeId(componentName, componentId) == false)
		return {};
	return FindEventId(componentId, evName);
}
std::optional<ComponentEventId> EntityComponentManager::FindEventId(ComponentId componentId, const std::string &evName) const
{
	auto *componentInfo = GetComponentInfo(componentId);
	if(!componentInfo)
		return {};
	std::string fullName = componentInfo->name;
	fullName = fullName + '_' + evName;
	auto it = std::find_if(m_componentEvents.begin(), m_componentEvents.end(), [componentId, &fullName](const std::pair<ComponentEventId, ComponentEventInfo> &pair) { return pair.second.componentId == componentId && pair.second.name == fullName; });
	if(it == m_componentEvents.end())
		return {};
	return it->first;
}
ComponentEventId EntityComponentManager::RegisterEventById(const std::string &evName, ComponentId componentId, ComponentEventInfo::Type type)
{
	auto it = std::find_if(m_componentEvents.begin(), m_componentEvents.end(), [&evName](const std::pair<const ComponentEventId, ComponentEventInfo> &pair) { return evName == pair.second.name; });
	if(it == m_componentEvents.end()) {
		auto id = get_component_event_id(evName);
		it = m_componentEvents.insert(std::make_pair(id, ComponentEventInfo {evName, componentId, {}, type})).first;
	}
	return it->first;
}
bool EntityComponentManager::GetEventId(const std::string &evName, ComponentEventId &evId) const
{
	auto it = std::find_if(m_componentEvents.begin(), m_componentEvents.end(), [&evName](const std::pair<const ComponentEventId, ComponentEventInfo> &pair) { return evName == pair.second.name; });
	if(it == m_componentEvents.end())
		return false;
	evId = it->first;
	return true;
}
ComponentEventId EntityComponentManager::GetEventId(const std::string &evName) const
{
	ComponentEventId evId;
	if(GetEventId(evName, evId) == false)
		throw std::logic_error("Entity component event '" + evName + "' has not been registered!");
	return evId;
}
bool EntityComponentManager::GetEventName(ComponentEventId evId, std::string &outEvName) const
{
	auto it = m_componentEvents.find(evId);
	if(it == m_componentEvents.end())
		return false;
	outEvName = it->second.name;
	return true;
}
std::string EntityComponentManager::GetEventName(ComponentEventId evId) const
{
	std::string name;
	if(GetEventName(evId, name) == false)
		throw std::logic_error("Entity component event '" + std::to_string(evId) + "' has not been registered!");
	return name;
}
const std::unordered_map<ComponentEventId, ComponentEventInfo> &EntityComponentManager::GetEvents() const { return m_componentEvents; }
const std::vector<EntityComponentManager::ComponentContainerInfo> &EntityComponentManager::GetComponents() const { return const_cast<EntityComponentManager *>(this)->GetComponents(); }
std::vector<EntityComponentManager::ComponentContainerInfo> &EntityComponentManager::GetComponents() { return m_components; }
const std::vector<BaseEntityComponent *> &EntityComponentManager::GetComponents(ComponentId componentId) const
{
	std::size_t count;
	return GetComponents(componentId, count);
}
const std::vector<BaseEntityComponent *> &EntityComponentManager::GetComponents(ComponentId componentId, std::size_t &count) const
{
	if(componentId >= m_components.size()) {
		static std::vector<BaseEntityComponent *> empty {};
		count = 0ull;
		return empty;
	}
	auto &info = m_components.at(componentId);
	count = info.GetCount();
	return info.GetComponents();
}
void EntityComponentManager::DeregisterComponent(BaseEntityComponent &component) { m_components.at(component.GetComponentId()).Pop(component); }

void EntityComponentManager::RegisterLuaBindings(lua::State *l, luabind::module_ &module)
{
	for(auto &reg : m_luaBindingRegistrations)
		reg(l, module);
	m_luaBindingRegistrations.clear();
}

////////////////////

void EntityComponentManager::ComponentContainerInfo::Push(BaseEntityComponent &component)
{
	if(m_freeIndices.empty() == false) {
		auto idx = m_freeIndices.front();
		m_freeIndices.pop();
		while(idx >= m_count && m_freeIndices.empty() == false) {
			idx = m_freeIndices.front();
			m_freeIndices.pop();
		}
		if(idx >= m_count) {
			Push(component);
			return;
		}
		m_components.at(idx) = &component;
		return;
	}
	if(m_count == m_components.capacity())
		m_components.reserve(math::max(math::ceil(m_components.capacity() * 1.4f), 50)); // Increase capacity to 140% of current capacity
	if(m_count < m_components.size()) {
		m_components.at(m_count++) = &component;
		return;
	}
	m_components.push_back(&component);
	++m_count;
}
void EntityComponentManager::ComponentContainerInfo::Pop(BaseEntityComponent &component)
{
	auto it = std::find_if(m_components.begin(), m_components.end(), [&component](const BaseEntityComponent *componentOther) { return &component == componentOther; });
	if(it == m_components.end())
		return;
	auto idx = it - m_components.begin();
	m_components.at(idx) = nullptr;
	if((idx + 1u) == m_count) {
		while(idx > 0ull && m_components.at(idx) == nullptr) {
			--m_count;
			--idx;
		}
		if(idx == 0ull && m_components.front() == nullptr)
			m_count = 0ull;
	}
	else
		m_freeIndices.push(idx);
}
const std::vector<BaseEntityComponent *> &EntityComponentManager::ComponentContainerInfo::GetComponents() const { return m_components; }
std::size_t EntityComponentManager::ComponentContainerInfo::GetCount() const { return m_count; }

////////////////////

ComponentMemberInfo ComponentMemberInfo::CreateDummy() { return ComponentMemberInfo {}; }
ComponentMemberInfo::ComponentMemberInfo(const char *name, ents::EntityMemberType type, const ApplyFunction &applyFunc, const GetFunction &getFunc)
    : m_name {name}, m_nameHash {get_component_member_name_hash(*m_name)}, type {type}, setterFunction {applyFunc}, getterFunction {getFunc}
{
}
ComponentMemberInfo::ComponentMemberInfo(const std::string &name, ents::EntityMemberType type, const ApplyFunction &applyFunc, const GetFunction &getFunc) : ComponentMemberInfo {register_global_string(name), type, applyFunc, getFunc} {}
