#ifndef __CBASEENTITY_H__
#define __CBASEENTITY_H__
#include "pragma/clientdefinitions.h"
#include <pragma/entities/baseentity.h>
#include "pragma/rendering/sortedrendermeshcontainer.h"

class CALSound;
class EntityHandle;
class Engine;
class NetPacket;
enum class RenderMode : uint32_t;
class RenderInstance;
class RenderObject;
class Material;
namespace pragma
{
	class BaseEntityComponent;
	class ShaderTextured3DBase;
	class CRenderComponent;
};
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCLIENT CBaseEntity
	: public BaseEntity
{
public:
	CBaseEntity();
	void Construct(unsigned int idx,unsigned int clientIdx);

	virtual util::WeakHandle<pragma::BaseModelComponent> GetModelComponent() const override;
	virtual util::WeakHandle<pragma::BaseAnimatedComponent> GetAnimatedComponent() const override;
	virtual util::WeakHandle<pragma::BaseWeaponComponent> GetWeaponComponent() const override;
	virtual util::WeakHandle<pragma::BaseVehicleComponent> GetVehicleComponent() const override;
	virtual util::WeakHandle<pragma::BaseAIComponent> GetAIComponent() const override;
	virtual util::WeakHandle<pragma::BaseCharacterComponent> GetCharacterComponent() const override;
	virtual util::WeakHandle<pragma::BasePlayerComponent> GetPlayerComponent() const override;
	virtual util::WeakHandle<pragma::BasePhysicsComponent> GetPhysicsComponent() const override;
	virtual util::WeakHandle<pragma::BaseTimeScaleComponent> GetTimeScaleComponent() const override;
	virtual bool IsCharacter() const override;
	virtual bool IsPlayer() const override;
	virtual bool IsWeapon() const override;
	virtual bool IsVehicle() const override;
	virtual bool IsNPC() const override;

	// Returns the server-side representation of this entity (If the entity isn't clientside only)
	// This only works for single-player / listen servers!
	BaseEntity *GetServersideEntity() const;

	util::WeakHandle<pragma::CRenderComponent> GetRenderComponent() const;

	virtual void Initialize() override;
	virtual void ReceiveData(NetPacket &packet);
	virtual void ReceiveSnapshotData(NetPacket &packet);
	virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet);

	virtual void Remove() override;
	virtual void OnRemove() override;
	
	bool IsClientsideOnly() const;
	virtual bool IsNetworkLocal() const override;

	unsigned int GetClientIndex();

	void SendNetEventTCP(UInt32 eventId,NetPacket &data) const;
	void SendNetEventTCP(UInt32 eventId) const;
	void SendNetEventUDP(UInt32 eventId,NetPacket &data) const;
	void SendNetEventUDP(UInt32 eventId) const;

	virtual NetworkState *GetNetworkState() const override final;
protected:
	virtual void DoSpawn() override;
	virtual void OnComponentAdded(pragma::BaseEntityComponent &component) override;
	// TODO: Obsolete? (Also remove from BaseEntity and SBaseEntity)
	void EraseFunction(int function);

	friend pragma::BaseEntityComponent;
	uint32_t m_clientIdx = 0u;
	mutable util::WeakHandle<pragma::CRenderComponent> m_renderComponent = {};

	//std::unique_ptr<std::shared_ptr<prosper::Buffer>> m_softBodyBuffers = nullptr; // prosper TODO
};
#pragma warning(pop)

inline DLLCLIENT Con::c_cout& operator<<(Con::c_cout &os,CBaseEntity &ent) {return ent.print(os);}

#endif