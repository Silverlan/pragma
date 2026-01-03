// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :game;

import :entities;
import :entities.components;
import :server_state;

void pragma::SGame::SendSnapshot(SPlayerComponent *pl)
{
	auto *session = pl ? pl->GetClientSession() : nullptr;
	if(session == nullptr)
		return;
	NetPacket packet;
	packet->Write<uint8_t>(session->SwapSnapshotId());
	packet->Write<double>(CurTime());

	std::vector<SBaseEntity *> *entities;
	GetEntities(&entities);
	auto numEntities = entities->size();
	auto posNumEnts = packet->GetSize();
	packet->Write<unsigned int>((unsigned int)(0));
	size_t numEntitiesValid = 0;
	for(size_t i = 0; i < numEntities; i++) {
		SBaseEntity *ent = (*entities)[i];
		if(ent != nullptr && ent->IsShared() && ent->IsSynchronized() && ent->IsMarkedForSnapshot()) {
			numEntitiesValid++;
			auto pTrComponent = ent->GetTransformComponent();
			auto pVelComponent = ent->GetComponent<VelocityComponent>();
			networking::write_entity(packet, ent);
			networking::write_vector(packet, pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {});
			networking::write_vector(packet, pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3 {});
			networking::write_vector(packet, pVelComponent.valid() ? pVelComponent->GetAngularVelocity() : Vector3 {});
			networking::write_quat(packet, pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity());

			auto offsetEntData = packet->GetSize();
			packet->Write<UInt8>(UInt8(0));
			auto offset = packet->GetSize();
			ent->SendSnapshotData(packet, *pl);
			auto entDataSize = packet->GetSize() - offset;
#ifdef _DEBUG
			assert(entDataSize <= std::numeric_limits<UInt8>::max());
#endif
			packet->Write<UInt8>(CUInt8(entDataSize), &offsetEntData);

			auto flags = SnapshotFlags::None;
			auto offsetSnapshotFlags = packet->GetOffset();
			packet->Write<decltype(flags)>(flags);

			auto pPhysComponent = ent->GetPhysicsComponent();
			physics::PhysObj *physObj = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
			if(physObj != nullptr && !physObj->IsStatic()) {
				flags |= SnapshotFlags::PhysicsData;
				if(physObj->IsController()) {
					packet->Write<uint8_t>(1u);
					auto *physController = static_cast<physics::ControllerPhysObj *>(physObj);
					packet->Write<Vector3>(physController->GetPosition());
					packet->Write<Quat>(physController->GetOrientation());
					packet->Write<Vector3>(physController->GetLinearVelocity());
					packet->Write<Vector3>(physController->GetAngularVelocity());
				}
				else {
					auto colObjs = physObj->GetCollisionObjects();
					packet->Write<uint8_t>(static_cast<uint8_t>(colObjs.size()));
					//auto i = 0;
					for(auto &hObj : colObjs) {
						Vector3 pos {0.f, 0.f, 0.f};
						auto rot = uquat::identity();
						Vector3 vel {0.f, 0.f, 0.f};
						Vector3 angVel {0.f, 0.f, 0.f};
						if(hObj.IsValid()) {
							auto *o = hObj.Get();
							pos = o->GetPos();
							rot = o->GetRotation();
							if(o->IsRigid()) {
								auto *rigid = o->GetRigidBody();
								vel = rigid->GetLinearVelocity();
								angVel = rigid->GetAngularVelocity();
							}
						}
						packet->Write<Vector3>(pos);
						packet->Write<Quat>(rot);
						packet->Write<Vector3>(vel);
						packet->Write<Vector3>(angVel);
					}
				}
			}

			auto offsetNumComponents = 0u;
			auto numComponents = 0u;
			auto bFirst = true;
			for(auto &pComponent : ent->GetComponents()) {
				if(pComponent.expired() || pComponent->ShouldTransmitSnapshotData() == false)
					continue;
				auto *pSnapshotComponent = dynamic_cast<SBaseSnapshotComponent *>(pComponent.get());
				if(pSnapshotComponent == nullptr)
					throw std::logic_error("Component must be derived from SBaseSnapshotComponent if snapshot data is enabled!");
				if(bFirst) {
					bFirst = false;
					flags |= SnapshotFlags::ComponentData;
					offsetNumComponents = packet->GetOffset();
					packet->Write<uint8_t>(static_cast<uint8_t>(0u));
				}
				packet->Write<ComponentId>(pComponent->GetComponentId());
				auto offsetComponentSize = packet->GetOffset();
				packet->Write<uint8_t>(static_cast<uint8_t>(0u));

				auto offsetComponentDataStart = packet->GetOffset();
				pSnapshotComponent->SendSnapshotData(packet, *pl);
				auto szComponent = packet->GetOffset() - offsetComponentDataStart;
				if(szComponent > std::numeric_limits<uint8_t>::max())
					throw std::runtime_error("Component size mustn't exceed " + std::to_string(std::numeric_limits<uint8_t>::max()) + " bytes!");
				packet->Write<uint8_t>(szComponent, &offsetComponentSize);

				if(++numComponents == std::numeric_limits<uint8_t>::max()) {
					Con::CWAR << Con::PREFIX_SERVER << "Attempted to send data for more than " << std::numeric_limits<uint8_t>::max() << " components for a single entity! This is not allowed!" << Con::endl;
					break;
				}
			}
			packet->Write<decltype(flags)>(flags, &offsetSnapshotFlags);
			if((flags & SnapshotFlags::ComponentData) != SnapshotFlags::None)
				packet->Write<uint8_t>(numComponents, &offsetNumComponents);
		}
	}
	packet->Write<UInt32>(CUInt32(numEntitiesValid), &posNumEnts);

	//unsigned int numPlayers = Player::GetPlayerCount();
	auto posNumPls = packet->GetSize();
	packet->Write<unsigned char>((unsigned char)(0));
	auto &players = SPlayerComponent::GetAll();
	unsigned char numPlayersValid = 0;
	for(auto *plComponent : players) {
		if(plComponent != nullptr && plComponent != pl) {
			auto *ent = static_cast<Player *>(plComponent->GetBasePlayer());
			if(ent != nullptr) {
				numPlayersValid++;
				networking::write_player(packet, plComponent);
				auto charComponent = ent->GetCharacterComponent();
				networking::write_quat(packet, charComponent.valid() ? charComponent->GetViewOrientation() : uquat::identity());
				std::vector<InputAction> &keyStack = pl->GetKeyStack();
				auto sz = CUChar(keyStack.size());
				packet->Write<UChar>(sz);
				for(UChar k = 0; k < sz; k++) // TODO: Same as above
				{
					InputAction &ka = keyStack[k];
					packet->Write<unsigned short>(CUInt16(ka.action));
					packet->Write<char>(ka.task == KEY_PRESS);
				}
			}
		}
	}
	packet->Write<unsigned char>(numPlayersValid, &posNumPls);
	ServerState::Get()->SendPacket(networking::net_messages::client::SNAPSHOT, packet, networking::Protocol::FastUnreliable, *session);
}

void pragma::SGame::SendSnapshot()
{
	//Con::CSV<<"Sending snapshot.."<<Con::endl;
	auto &players = SPlayerComponent::GetAll();
	//unsigned char numPlayersValid = 0;
	for(auto *plComponent : players) {
		if(plComponent != nullptr && plComponent->IsGameReady())
			SendSnapshot(plComponent);
	}
	std::vector<SBaseEntity *> *entities;
	GetEntities(&entities);
	for(unsigned int i = 0; i < entities->size(); i++) {
		SBaseEntity *ent = (*entities)[i];
		if(ent != nullptr && ent->IsMarkedForSnapshot())
			ent->MarkForSnapshot(false);
	}
	for(auto *plComponent : players) {
		if(plComponent != nullptr)
			plComponent->ClearKeyStack();
	}
}
