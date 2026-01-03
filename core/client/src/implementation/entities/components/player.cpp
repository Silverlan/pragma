// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.player;
import :client_state;
import :engine;
import :game;
import :networking.util;

using namespace pragma;

namespace pragma {
	using ::operator<<;
};
std::vector<CPlayerComponent *> CPlayerComponent::s_players;
const std::vector<CPlayerComponent *> &CPlayerComponent::GetAll() { return s_players; }
unsigned int CPlayerComponent::GetPlayerCount() { return CUInt32(s_players.size()); }

Con::c_cout &CPlayerComponent::print(Con::c_cout &os)
{
	auto &ent = GetEntity();
	auto nameC = ent.GetNameComponent();
	os << "CPlayer[" << (nameC.valid() ? nameC->GetName() : "") << "][" << ent.GetIndex() << "]"
	   << "[" << ent.GetClass() << "]"
	   << "[";
	auto &mdl = ent.GetModel();
	if(mdl == nullptr)
		os << "NULL";
	else
		os << mdl->GetName();
	os << "]";
	return os;
}

std::ostream &CPlayerComponent::print(std::ostream &os)
{
	auto &ent = GetEntity();
	auto nameC = ent.GetNameComponent();
	os << "CPlayer[" << (nameC.valid() ? nameC->GetName() : "") << "][" << ent.GetIndex() << "]"
	   << "[" << ent.GetClass() << "]"
	   << "[";
	auto &mdl = ent.GetModel();
	if(mdl == nullptr)
		os << "NULL";
	else
		os << mdl->GetName();
	os << "]";
	return os;
}

CPlayerComponent::CPlayerComponent(ecs::BaseEntity &ent) : BasePlayerComponent(ent), m_crouchViewOffset(nullptr), m_upDirOffset(nullptr)
{
	s_players.push_back(this);

	BindEventUnhandled(submergibleComponent::EVENT_ON_WATER_SUBMERGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto &ent = GetEntity();
		auto pSoundEmitterComponent = ent.GetComponent<CSoundEmitterComponent>();
		if(pSoundEmitterComponent.valid()) {
			if(m_sndUnderwater == nullptr) {
				m_sndUnderwater = get_client_state()->CreateSound("fx.underwater", audio::ALSoundType::Effect, audio::ALCreateFlags::Mono);
				m_sndUnderwater->SetRelative(true);
			}
			if(m_sndUnderwater != nullptr) {
				m_sndUnderwater->SetGain(1.f);
				m_sndUnderwater->FadeIn(0.1f);
			}
		}
	});
	BindEventUnhandled(submergibleComponent::EVENT_ON_WATER_EMERGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		if(m_sndUnderwater != nullptr)
			m_sndUnderwater->FadeOut(0.1f);
	});
}

CPlayerComponent::~CPlayerComponent()
{
	auto it = std::find(s_players.begin(), s_players.end(), this);
	if(it != s_players.end())
		s_players.erase(it);
	if(m_cbCalcOrientationView.IsValid())
		m_cbCalcOrientationView.Remove();
	m_crouchViewOffset = nullptr;
	m_upDirOffset = nullptr;
	if(m_cbUnderwaterDsp.valid() == true)
		m_cbUnderwaterDsp->Remove();
}

void CPlayerComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CPlayerComponent::OnDeployWeapon(ecs::BaseEntity &ent) {}

void CPlayerComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BasePlayerComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(OrientationComponent)) {
		auto &orientC = static_cast<OrientationComponent &>(component);
		auto &pUpDirProp = orientC.GetUpDirectionProperty();
		FlagCallbackForRemoval(pUpDirProp->AddCallback([this](std::reference_wrapper<const Vector3> oldVal, std::reference_wrapper<const Vector3> newVal) { OnSetUpDirection(newVal); }), CallbackType::Component, &orientC);
	}
	else if(typeid(component) == typeid(CObservableComponent))
		m_observableComponent = &static_cast<CObservableComponent &>(component);
}
void CPlayerComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BasePlayerComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(CObservableComponent))
		m_observableComponent = nullptr;
}

util::EventReply CPlayerComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BasePlayerComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseCharacterComponent::EVENT_ON_DEPLOY_WEAPON)
		OnDeployWeapon(static_cast<const CEOnDeployWeapon &>(evData).weapon);
	else if(eventId == baseCharacterComponent::EVENT_ON_SET_ACTIVE_WEAPON)
		OnSetActiveWeapon(static_cast<const CEOnSetActiveWeapon &>(evData).weapon);
	else if(eventId == baseCharacterComponent::EVENT_ON_CHARACTER_ORIENTATION_CHANGED)
		OnSetCharacterOrientation(static_cast<const CEOnSetCharacterOrientation &>(evData).up);
	return util::EventReply::Unhandled;
}

void CPlayerComponent::OnSetUpDirection(const Vector3 &direction)
{
	m_upDirOffset = nullptr;
	auto charComponent = GetEntity().GetCharacterComponent();
	if(charComponent.expired())
		return;
	//auto &rot = charComponent->NormalizeViewOrientation();
	//m_upDirOffset = std::make_unique<DeltaTransform>(Vector3(0,0,0),rot,4);
}

void CPlayerComponent::OnSetActiveWeapon(ecs::BaseEntity *ent)
{
	auto charComponent = GetEntity().GetCharacterComponent();
	auto *prevWeapon = charComponent.valid() ? charComponent->GetActiveWeapon() : nullptr;
	if(prevWeapon != nullptr && prevWeapon->IsWeapon())
		static_cast<CWeaponComponent &>(*prevWeapon->GetWeaponComponent()).UpdateOwnerAttachment();
}

void CPlayerComponent::OnWaterSubmerged()
{
	if(IsLocalPlayer() == false || m_cbUnderwaterDsp.valid() == true)
		return;
	auto *entDsp = get_cgame()->CreateEntity<CEnvSoundDsp>();
	if(entDsp == nullptr)
		return;
	auto *pDspComponent = static_cast<BaseEnvSoundDspComponent *>(entDsp->FindComponent("sound_dsp").get());
	entDsp->SetKeyValue("spawnflags", std::to_string(math::to_integral(BaseEnvSoundDspComponent::SpawnFlags::All | BaseEnvSoundDspComponent::SpawnFlags::AffectRelative)));
	if(pDspComponent != nullptr)
		pDspComponent->SetDSPEffect("underwater");
	entDsp->Spawn();
	auto *pToggleComponent = static_cast<BaseToggleComponent *>(entDsp->FindComponent("toggle").get());
	if(pToggleComponent != nullptr)
		pToggleComponent->TurnOn();
	m_cbUnderwaterDsp = entDsp->GetHandle();
}

void CPlayerComponent::OnWaterEmerged()
{
	if(m_cbUnderwaterDsp.valid() == false)
		return;
	m_cbUnderwaterDsp->Remove();
}

void CPlayerComponent::ApplyViewRotationOffset(const EulerAngles &ang, float dur)
{
	auto tStart = get_cgame()->CurTime();
	auto cb = FunctionCallback<void, std::reference_wrapper<Vector3>, std::reference_wrapper<Quat>>::Create(nullptr);
	static_cast<Callback<void, std::reference_wrapper<Vector3>, std::reference_wrapper<Quat>> *>(cb.get())->SetFunction([cb, tStart, ang, dur](std::reference_wrapper<Vector3>, std::reference_wrapper<Quat> rot) mutable {
		auto &t = get_cgame()->CurTime();
		auto tDelta = math::min(static_cast<float>(t - tStart), dur);
		auto sc = static_cast<float>(math::sin(tDelta / (dur / 2.f) * math::pi_2));
		EulerAngles rotOffset {static_cast<float>(math::approach_angle(0.f, ang.p, math::abs(ang.p) * sc)), static_cast<float>(math::approach_angle(0.f, ang.y, math::abs(ang.y) * sc)), static_cast<float>(math::approach_angle(0.f, ang.r, math::abs(ang.r) * sc))};
		rot.get() = rot.get() * uquat::create(rotOffset);
		if(tDelta >= dur) {
			cb.Remove();
			/*auto cb = FunctionCallback<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>>::Create(nullptr);
			static_cast<Callback<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>>*>(cb.get())->SetFunction([cb,ang](std::reference_wrapper<Vector3> pos,std::reference_wrapper<Quat> rot) mutable {
				rot.get() = rot.get() *uquat::create(ang);
				cb.Remove();
			});
			pragma::get_cgame()->AddCallback("CalcView",cb);*/ // Makes sure the camera rotation STAYS at the designated rotation (Works, but only reasonable for pitch-axis, and at half the duration -> Useless?)
		}
	});
	get_cgame()->AddCallback("CalcViewOffset", cb);
}

bool CPlayerComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvApplyViewRotationOffset) {
		auto ang = networking::read_angles(packet);
		auto dur = packet->Read<float>();
		ApplyViewRotationOffset(ang, dur);
	}
	else if(eventId == m_netEvPrintMessage) {
		auto msg = packet->ReadString();
		auto type = static_cast<console::MESSAGE>(packet->Read<std::underlying_type_t<console::MESSAGE>>());
		PrintMessage(msg, type);
	}
	else if(eventId == m_netEvRespawn) {
		auto charComponent = GetEntity().GetCharacterComponent();
		if(charComponent.valid())
			charComponent->Respawn();
	}
	else if(eventId == m_netEvSetViewOrientation) {
		auto charComponent = GetEntity().GetCharacterComponent();
		if(charComponent.valid()) {
			auto rot = packet->Read<Quat>();
			charComponent->SetViewOrientation(rot);
		}
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}

void CPlayerComponent::Initialize()
{
	BasePlayerComponent::Initialize();

	BindEventUnhandled(submergibleComponent::EVENT_ON_WATER_SUBMERGED, [this](std::reference_wrapper<ComponentEvent> evData) { OnWaterSubmerged(); });
	BindEventUnhandled(submergibleComponent::EVENT_ON_WATER_EMERGED, [this](std::reference_wrapper<ComponentEvent> evData) { OnWaterEmerged(); });
	BindEvent(cRenderComponent::EVENT_SHOULD_DRAW_SHADOW, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &shouldDrawData = static_cast<CEShouldDraw &>(evData.get());
		if(ShouldDrawShadow() == false) {
			shouldDrawData.shouldDraw = false;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(cRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES, [this](std::reference_wrapper<ComponentEvent> evData) { OnUpdateMatrices(static_cast<CEOnUpdateRenderMatrices &>(evData.get()).transformation); });

	auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
	auto pRenderComponent = ent.GetRenderComponent();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->SetCollisionType(physics::CollisionType::AABB);

	GetEntity().AddComponent<CObservableComponent>();
	if(m_observableComponent)
		m_observableComponent->SetViewOffset(Vector3 {0, m_standEyeLevel, 0});
}

bool CPlayerComponent::IsInFirstPersonMode() const
{
	if(!m_observableComponent)
		return false;
	auto *observer = m_observableComponent->GetObserver();
	if(!observer)
		return false;
	return static_cast<CObserverComponent *>(observer)->IsInFirstPersonMode();
}

void CPlayerComponent::OnUpdateMatrices(Mat4 &transformMatrix)
{
	if(IsLocalPlayer() && IsInFirstPersonMode()) {
		auto pTrComponent = GetEntity().GetTransformComponent();
		auto t = (pTrComponent != nullptr ? pTrComponent->GetForward() : uvec::PRM_FORWARD) * VIEW_BODY_OFFSET;
		transformMatrix = glm::gtc::translate(umat::identity(), t) * transformMatrix; // Translate to align shadow with view body
	}
}

void CPlayerComponent::UpdateViewModelTransform()
{
	auto *vm = get_cgame()->GetViewModel<CViewModelComponent>();
	if(vm == nullptr)
		return;
	auto &vmEnt = vm->GetEntity();
	auto &ent = GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentVm = vmEnt.GetTransformComponent();
	if(pTrComponentVm && (charComponent.valid() || pTrComponent != nullptr)) {
		auto pos = charComponent.valid() ? charComponent->GetEyePosition() : pTrComponent->GetPosition();
		auto &rot = charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent->GetRotation();
		auto offset = vm->GetViewModelOffset();
		uvec::local_to_world(pos, rot, offset);
		pTrComponentVm->SetPosition(offset);
		pTrComponentVm->SetRotation(rot);
	}
	auto pAttComponent = vmEnt.AddComponent<CAttachmentComponent>();
	if(pAttComponent.valid()) {
		AttachmentInfo attInfo {};
		attInfo.flags |= FAttachmentMode::PlayerView | FAttachmentMode::UpdateEachFrame;
		pAttComponent->AttachToEntity(&ent, attInfo);
	}
}

void CPlayerComponent::UpdateViewFOV()
{
	auto *vm = get_cgame()->GetViewModel<CViewModelComponent>();
	if(vm == nullptr)
		return;
	get_cgame()->SetViewModelFOV(vm->GetViewFOV());
}

void CPlayerComponent::SetLocalPlayer(bool b)
{
	BasePlayerComponent::SetLocalPlayer(b);

	auto renderC = GetEntity().GetComponent<CRenderComponent>();
	if(renderC.valid()) {
		if(b)
			renderC->AddToRenderGroup("thirdperson");
		else
			renderC->RemoveFromRenderGroup("thirdperson");
	}

	if(b == false)
		return;
	auto *vm = get_cgame()->GetViewModel<CViewModelComponent>();
	if(vm != nullptr) {
		auto &vmEnt = vm->GetEntity();
		UpdateViewModelTransform();
		UpdateViewFOV();
		if(!vmEnt.IsSpawned())
			vmEnt.Spawn();
	}
	auto &ent = GetEntity();
	auto *body = get_cgame()->GetViewBody<CViewBodyComponent>();
	if(body != nullptr) {
		auto &entBody = body->GetEntity();
		auto pTrComponent = ent.GetTransformComponent();
		auto pTrComponentBody = body->GetEntity().GetTransformComponent();
		if(pTrComponent != nullptr && pTrComponentBody) {
			Vector3 pos = pTrComponent->GetPosition() + pTrComponent->GetForward() * VIEW_BODY_OFFSET;
			auto &rot = pTrComponent->GetRotation();
			pTrComponentBody->SetPosition(pos);
			pTrComponentBody->SetRotation(Quat(rot));
		}
		auto pAttComponent = entBody.AddComponent<CAttachmentComponent>();
		if(pAttComponent.valid()) {
			AttachmentInfo attInfo {};
			attInfo.flags |= FAttachmentMode::PlayerViewYaw | FAttachmentMode::BoneMerge | FAttachmentMode::UpdateEachFrame;
			pAttComponent->AttachToEntity(&ent, attInfo);
		}
		//body->SetRenderMode(RenderMode::None);
		if(!entBody.IsSpawned())
			entBody.Spawn();
	}
	auto *listener = get_cgame()->GetListener<CListenerComponent>();
	if(listener != nullptr) {
		auto &entListener = listener->GetEntity();
		auto pTrComponent = ent.GetTransformComponent();
		auto pTrComponentListener = entListener.GetTransformComponent();
		if(pTrComponent != nullptr && pTrComponentListener) {
			pTrComponentListener->SetPosition(pTrComponent->GetPosition());
			pTrComponentListener->SetRotation(pTrComponent->GetRotation());
		}
		auto pAttComponent = entListener.AddComponent<CAttachmentComponent>();
		if(pAttComponent.valid()) {
			AttachmentInfo attInfo {};
			attInfo.flags |= FAttachmentMode::PlayerView;
			pAttComponent->AttachToEntity(&ent, attInfo);
		}
		if(!entListener.IsSpawned())
			entListener.Spawn();
	}
}

bool CPlayerComponent::ShouldDraw() const
{
	if(!IsLocalPlayer())
		return true;
#pragma message("TODO: Find a better way to enable rendering, if being rendered through anything but the main camera (e.g. reflections)!")
	auto *scene = get_cgame()->GetScene<CSceneComponent>();
	if(get_cgame()->GetRenderScene<CSceneComponent>() != scene)
		return true;
	if(!m_observableComponent)
		return true;
	auto *observer = m_observableComponent->GetObserver();
	if(!observer)
		return true;
	return (observer->GetObserverMode() != ObserverMode::FirstPerson) ? true : false;
}

bool CPlayerComponent::ShouldDrawShadow() const
{
	auto pRenderComponent = static_cast<const ecs::CBaseEntity &>(GetEntity()).GetRenderComponent();
	return pRenderComponent ? pRenderComponent->GetCastShadows() : false;
}

void CPlayerComponent::OnTick(double tDelta)
{
	BasePlayerComponent::OnTick(tDelta);

	if(m_crouchViewOffset != nullptr) {
		math::DeltaOffset &doffset = *m_crouchViewOffset;
		if(doffset.time <= 0)
			m_crouchViewOffset = nullptr;
		else {
			doffset.delta = math::min(doffset.delta + tDelta / 0.2, 1.0); // 0.2 seconds to reach full speed
			float scale = CFloat((doffset.delta * tDelta) / doffset.time);
			scale = math::min(scale, 1.0f);
			Vector3 mv = doffset.offset * scale;
			doffset.offset -= mv;
			if(m_observableComponent)
				m_observableComponent->SetViewOffset(m_observableComponent->GetViewOffset() + mv);
			doffset.time -= tDelta;
			if(doffset.time <= 0)
				m_crouchViewOffset = nullptr;
		}
	}
	if(m_upDirOffset != nullptr) {
		math::DeltaTransform &dtrans = *m_upDirOffset;
		if(dtrans.time <= 0)
			m_upDirOffset = nullptr;
		else {
			dtrans.delta = math::min(dtrans.delta + tDelta / 0.2, 1.0); // 0.2 seconds to reach full speed
			float scale = CFloat((dtrans.delta * tDelta) / dtrans.time);
			scale = math::min(scale, 1.0f);

			Vector3 mv = dtrans.offset * scale;
			dtrans.offset -= mv;
			if(m_observableComponent)
				m_observableComponent->SetViewOffset(m_observableComponent->GetViewOffset() + mv);

			dtrans.time -= tDelta;
			if(dtrans.time <= 0)
				m_upDirOffset = nullptr;
		}
	}
}
void CPlayerComponent::OnCrouch()
{
	Vector3 viewOffset {};
	if(m_observableComponent)
		viewOffset = m_observableComponent->GetViewOffset();
	m_crouchViewOffset = std::make_unique<math::DeltaOffset>(Vector3(0, m_crouchEyeLevel - viewOffset.y, 0), 0.2f);
}
void CPlayerComponent::OnUnCrouch()
{
	Vector3 viewOffset {};
	if(m_observableComponent)
		viewOffset = m_observableComponent->GetViewOffset();
	m_crouchViewOffset = std::make_unique<math::DeltaOffset>(Vector3(0, m_standEyeLevel - viewOffset.y, 0), 0.4f);
}
void CPlayerComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BasePlayerComponent)); }
void CPlayerComponent::ReceiveData(NetPacket &packet)
{
	m_timeConnected = packet->Read<double>();
	auto hThis = GetHandle();
	networking::read_unique_entity(packet, [hThis, this](ecs::BaseEntity *ent) {
		if(ent == nullptr || hThis.expired())
			return;
		m_entFlashlight = ent->GetHandle();
		auto &entThis = GetEntity();
		auto charComponent = entThis.GetCharacterComponent();
		auto pTrComponentEnt = ent->GetTransformComponent();
		auto pTrComponent = entThis.GetTransformComponent();
		if(pTrComponentEnt && (charComponent.valid() || pTrComponent != nullptr)) {
			pTrComponentEnt->SetPosition(charComponent.valid() ? (charComponent->GetEyePosition() + charComponent->GetViewRight() * 12.f + charComponent->GetViewForward() * 5.f) : pTrComponent->GetPosition());
			pTrComponentEnt->SetRotation(charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent->GetRotation());
		}
		auto pAttComponent = ent->AddComponent<CAttachmentComponent>();
		if(pAttComponent.valid()) {
			AttachmentInfo attInfo {};
			attInfo.flags |= FAttachmentMode::PlayerView | FAttachmentMode::UpdateEachFrame;
			pAttComponent->AttachToEntity(&entThis, attInfo);
		}
	});
}

void CPlayerComponent::PrintMessage(std::string message, console::MESSAGE type)
{
	switch(type) {
	case console::MESSAGE::PRINTCONSOLE:
		Con::COUT << message << Con::endl;
		break;
	case console::MESSAGE::PRINTCHAT:
		{
			// TODO
			//auto *l = client->GetLuaState();
			//DLLLUA void GetGlobal(lua::State *lua,const std::string &name);
			break;
		}
	}
}

void CPlayerComponent::OnSetCharacterOrientation(const Vector3 &up)
{
	auto charComponent = GetEntity().GetCharacterComponent();
	if(IsLocalPlayer() == false)
		return;
	if(m_cbCalcOrientationView.IsValid())
		m_cbCalcOrientationView.Remove();

	// Update camera rotation
	auto rotCur = charComponent->GetViewOrientation();
	auto rotRel = charComponent->GetOrientationAxesRotation();

	auto rotDst = rotRel * rotCur;
	//auto ang = EulerAngles{rotDst};
	// to euler angles
	auto m = glm::mat4_cast(rotDst);
	EulerAngles ang;
	glm::gtx::extractEulerAngleYXZ(m, ang.y, ang.p, ang.r);
	ang.p = math::rad_to_deg(ang.p);
	ang.y = math::rad_to_deg(ang.y);
	ang.r = math::rad_to_deg(ang.r);
	//

	auto fToQuat = [](const EulerAngles &ang) {
		auto m = umat::identity();
		m = glm::gtx::eulerAngleYXZ(math::deg_to_rad(ang.y), math::deg_to_rad(ang.p), math::deg_to_rad(ang.r));
		auto q = glm::gtc::quat_cast(m);
		return q;
	};

	ang.r = 0.f;
	Quat rotCurNoRoll;
	if(ang.p < -135.f || ang.p > 135.f)
		rotCurNoRoll = fToQuat(ang) * fToQuat(EulerAngles {0.f, 0.f, 180.f});
	else {
		if(ang.p < -90.f)
			ang.p = -90.f;
		else if(ang.p > 90.f)
			ang.p = 90.f;
		rotCurNoRoll = fToQuat(ang);
	}
	rotDst = uquat::get_inverse(rotRel) * rotCurNoRoll;
	charComponent->SetViewOrientation(rotDst);
	//

	/*m_cbCalcOrientationView = pragma::get_cgame()->AddCallback("CalcView",FunctionCallback<void,std::reference_wrapper<Vector3>,std::reference_wrapper<Quat>>::Create([this,up](std::reference_wrapper<Vector3> refPos,std::reference_wrapper<Quat> refRot) {
		//SetUpDirection(up);
		auto &ent = GetEntity();
		auto charComponent = ent.GetCharacterComponent();
		auto pTrComponent = ent.GetTransformComponent();
		auto rotCur = charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent != nullptr ? pTrComponent->GetOrientation() : uquat::identity();
		//auto &up = GetUpDirection();

		auto rotRel = charComponent.valid() ? charComponent->GetOrientationAxesRotation() : uquat::identity();
		auto newRotCur = rotRel *rotCur;
		auto ang = EulerAngles(newRotCur);
		ang.r = 0.f;
		auto rotDst = uquat::identity(); // rotCurNoRoll
		if(ang.p < -135.f || ang.p > 135.f)
		{
			rotDst = uquat::create(ang) *uquat::create(EulerAngles(0.f,0.f,180.f)); // This will effectively flip pitch around
			ang.p = pragma::math::clamp(ang.p,-90.f,90.f);
		}
		else
		{
			ang.p = pragma::math::clamp(ang.p,-90.f,90.f);
			rotDst = uquat::create(ang);
		}

		//refRot.get() = uquat::get_inverse(rotRel) *uquat::slerp(newRotCur,rotDst,pragma::get_cengine()->GetDeltaFrameTime() *4.f);
		// TODO: Remove callback on complete!
	}));*/
}

void CPlayerComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	BasePlayerComponent::RegisterLuaBindings(l, modEnts);
	auto def = pragma::LuaCore::create_entity_component_class<CPlayerComponent, BasePlayerComponent>("PlayerComponent");
	def.def("IsInFirstPersonMode", &CPlayerComponent::IsInFirstPersonMode);
	modEnts[def];
}
