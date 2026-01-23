// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.liquid;

import :entities.components;
import :game;

using namespace pragma;

SLiquidComponent::SLiquidComponent(ecs::BaseEntity &ent) : BaseFuncLiquidComponent(ent) {}
SLiquidComponent::~SLiquidComponent()
{
	if(m_cbGameInitialized.IsValid())
		m_cbGameInitialized.Remove();
}

void SLiquidComponent::Initialize()
{
	BaseFuncLiquidComponent::Initialize();

	BindEventUnhandled(sModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		// TODO: Move this to shared
		auto &ent = GetEntity();
		if(ent.IsSpawned() == false || m_cbGameInitialized.IsValid() == true)
			return;
		InitializeWaterSurface();

		/*if(IsShared() == false)
			return;
		NetPacket packet {};
		packet->Write<Vector3>(m_waterPlane.GetNormal());
		packet->Write<double>(m_waterPlane.GetDistance());
		SendNetEventTCP(m_netEvSetWaterPlane,packet);*/
	});
	SetTickPolicy(TickPolicy::Always); // TODO
}

void SLiquidComponent::OnEntitySpawn()
{
	BaseFuncLiquidComponent::OnEntitySpawn();
	// TODO: Move this to shared
	auto &ent = GetEntity();
	auto *game = ent.GetNetworkState()->GetGameState();
	if(game->IsMapLoaded() == false) {
		// Need to wait until the game is initialized, to be sure the entity exists clientside
		m_cbGameInitialized = game->AddCallback("OnMapLoaded", FunctionCallback<void>::Create([this]() { InitializeWaterSurface(); }));
		return;
	}
	auto &mdl = ent.GetModel();
	if(mdl == nullptr)
		return;
	InitializeWaterSurface();
}

void SLiquidComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) {}

///////////////

void SLiquidComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FuncWater::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLiquidComponent>();
}
