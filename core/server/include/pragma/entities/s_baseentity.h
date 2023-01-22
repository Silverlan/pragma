/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_BASEENTITY_H__
#define __S_BASEENTITY_H__

#include "pragma/serverdefinitions.h"
#include <pragma/entities/baseentity.h>

class Engine;
class NetPacket;
class TimerHandle;
namespace pragma
{
	namespace networking {enum class Protocol : uint8_t; class ClientRecipientFilter;};
};
class DLLSERVER SBaseEntity
	: public BaseEntity
{
public:
	SBaseEntity();
	virtual pragma::ComponentHandle<pragma::BaseEntityComponent> AddNetworkedComponent(const std::string &name) override;
protected:
	bool m_bShared;
	Bool m_bSynchronized;
	void EraseFunction(int function);
	virtual void OnComponentAdded(pragma::BaseEntityComponent &component) override;
	virtual void OnComponentRemoved(pragma::BaseEntityComponent &component) override;
public:
	virtual void DoSpawn() override;

	virtual void Remove() override;
	virtual void Initialize() override;
	virtual void InitializeLuaObject(lua_State *lua) override;
	virtual void SendSnapshotData(NetPacket &packet,pragma::BasePlayerComponent &pl);
	virtual void SendData(NetPacket &packet,pragma::networking::ClientRecipientFilter &rp);
	bool IsShared() const;
	void SetShared(bool b);
	Bool IsNetworked();
	// Alias for !IsShared
	bool IsServersideOnly() const;
	virtual bool IsNetworkLocal() const override;
	virtual NetworkState *GetNetworkState() const override final;
	// Returns the client-side representation of this entity (If the entity isn't serverside only)
	// This only works for single-player / listen servers!
	BaseEntity *GetClientsideEntity() const;
	Bool IsSynchronized() const;
	void SetSynchronized(Bool b);

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

	pragma::NetEventId RegisterNetEvent(const std::string &name) const;

	// Net Events
	void SendNetEvent(pragma::NetEventId eventId,NetPacket &packet,pragma::networking::Protocol protocol,const pragma::networking::ClientRecipientFilter &rf);
	void SendNetEvent(pragma::NetEventId eventId,NetPacket &packet,pragma::networking::Protocol protocol);
	void SendNetEvent(pragma::NetEventId eventId,NetPacket &packet);
	void SendNetEvent(pragma::NetEventId eventId,pragma::networking::Protocol protocol);

	virtual Bool ReceiveNetEvent(pragma::BasePlayerComponent &pl,pragma::NetEventId,NetPacket &packet);
	//
};

inline DLLSERVER Con::c_cout& operator<<(Con::c_cout &os,SBaseEntity &ent) {return ent.print(os);}

#ifdef _WIN32
template<>
struct std::formatter<SBaseEntity> : std::formatter<std::string>
{
    auto format(SBaseEntity &ent, format_context &ctx) -> decltype(ctx.out())
    {
		std::stringstream ss;
		ent.print(ss);
        return std::format_to(ctx.out(), "{}",ss.str());
    }
};
#endif

#endif