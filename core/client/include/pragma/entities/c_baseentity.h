/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __CBASEENTITY_H__
#define __CBASEENTITY_H__
#include "pragma/clientdefinitions.h"
#include <pragma/entities/baseentity.h>
#include "pragma/rendering/sortedrendermeshcontainer.h"
#include <sharedutils/property/util_property.hpp>

class CALSound;
class Engine;
class NetPacket;
enum class RenderMode : uint32_t;
class RenderInstance;
class RenderObject;
class Material;
namespace pragma
{
	class BaseEntityComponent;
	class ShaderGameWorldLightingPass;
	class CRenderComponent;
	class CPhysicsComponent;
	class CSceneComponent;
};
namespace bounding_volume {class AABB;};
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCLIENT CBaseEntity
	: public BaseEntity
{
public:
	static pragma::ComponentEventId EVENT_ON_SCENE_FLAGS_CHANGED;
	static void RegisterEvents(pragma::EntityComponentManager &componentManager);

	CBaseEntity();
	void Construct(unsigned int idx,unsigned int clientIdx);
	virtual void InitializeLuaObject(lua_State *lua) override;

	virtual pragma::ComponentHandle<pragma::BaseAnimatedComponent> GetAnimatedComponent() const override;
	virtual pragma::ComponentHandle<pragma::BaseWeaponComponent> GetWeaponComponent() const override;
	virtual pragma::ComponentHandle<pragma::BaseVehicleComponent> GetVehicleComponent() const override;
	virtual pragma::ComponentHandle<pragma::BaseAIComponent> GetAIComponent() const override;
	virtual pragma::ComponentHandle<pragma::BaseCharacterComponent> GetCharacterComponent() const override;
	virtual pragma::ComponentHandle<pragma::BasePlayerComponent> GetPlayerComponent() const override;
	virtual pragma::ComponentHandle<pragma::BaseTimeScaleComponent> GetTimeScaleComponent() const override;
	virtual pragma::ComponentHandle<pragma::BaseNameComponent> GetNameComponent() const override;
	virtual bool IsCharacter() const override;
	virtual bool IsPlayer() const override;
	virtual bool IsWeapon() const override;
	virtual bool IsVehicle() const override;
	virtual bool IsNPC() const override;

	// Returns the server-side representation of this entity (If the entity isn't clientside only)
	// This only works for single-player / listen servers!
	BaseEntity *GetServersideEntity() const;

	pragma::CRenderComponent *GetRenderComponent() const;

	virtual void Initialize() override;
	virtual void ReceiveData(NetPacket &packet);
	virtual void ReceiveSnapshotData(NetPacket &packet);
	virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet);

	virtual void Remove() override;
	virtual void OnRemove() override;
	
	bool IsClientsideOnly() const;
	virtual bool IsNetworkLocal() const override;

	unsigned int GetClientIndex();
	virtual uint32_t GetLocalIndex() const override;

	uint32_t GetSceneFlags() const;
	const util::PUInt32Property &GetSceneFlagsProperty() const;
	void AddToScene(pragma::CSceneComponent &scene);
	void RemoveFromScene(pragma::CSceneComponent &scene);
	void RemoveFromAllScenes();
	bool IsInScene(const pragma::CSceneComponent &scene) const;
	std::vector<pragma::CSceneComponent*> GetScenes() const;

	void AddChild(CBaseEntity &ent);

	// Quick-access
	const bounding_volume::AABB &GetLocalRenderBounds() const;
	const bounding_volume::AABB &GetAbsoluteRenderBounds(bool updateBounds=true) const;
	//

	void SendNetEventTCP(UInt32 eventId,NetPacket &data) const;
	void SendNetEventTCP(UInt32 eventId) const;
	void SendNetEventUDP(UInt32 eventId,NetPacket &data) const;
	void SendNetEventUDP(UInt32 eventId) const;

	virtual NetworkState *GetNetworkState() const override final;
protected:
	virtual void DoSpawn() override;
	virtual void OnComponentAdded(pragma::BaseEntityComponent &component) override;
	virtual void OnComponentRemoved(pragma::BaseEntityComponent &component) override;
	// TODO: Obsolete? (Also remove from BaseEntity and SBaseEntity)
	void EraseFunction(int function);

	friend pragma::BaseEntityComponent;
	uint32_t m_clientIdx = 0u;
	util::PUInt32Property m_sceneFlags = nullptr;
	pragma::CRenderComponent *m_renderComponent = nullptr;
};
#pragma warning(pop)

inline DLLCLIENT Con::c_cout& operator<<(Con::c_cout &os,CBaseEntity &ent) {return ent.print(os);}

#endif