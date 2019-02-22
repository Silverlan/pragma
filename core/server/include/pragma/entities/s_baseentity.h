#ifndef __S_BASEENTITY_H__
#define __S_BASEENTITY_H__

#include "pragma/serverdefinitions.h"
#include <pragma/entities/baseentity.h>

class EntityHandle;
class Engine;
class NetPacket;
class TimerHandle;
namespace nwm {class RecipientFilter;};
class DLLSERVER SBaseEntity
	: public BaseEntity
{
public:
	SBaseEntity();
protected:
	bool m_bShared;
	Bool m_bSynchronized;
	void EraseFunction(int function);
	// COMPONENT TODO
	//virtual uint32_t RegisterNetworkedVariableName(const std::string &name) override;
public:
	virtual void DoSpawn() override;

	virtual void Remove() override;
	virtual void Initialize() override;
	virtual void SendSnapshotData(NetPacket &packet,pragma::BasePlayerComponent &pl);
	virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp);
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

	// COMPONENT TODO
	/*
	template<typename T>
		bool SetNetworkedVariable(const std::string &name,NetworkedVariable::Type type,T val,const std::function<bool()> &baseSetVar,bool bTCP);
	bool SetNetworkedBool(const std::string &name,bool v,bool bTCP);
	bool SetNetworkedChar(const std::string &name,char v,bool bTCP);
	bool SetNetworkedDouble(const std::string &name,double v,bool bTCP);
	bool SetNetworkedFloat(const std::string &name,float v,bool bTCP);
	bool SetNetworkedInt8(const std::string &name,int8_t v,bool bTCP);
	bool SetNetworkedInt16(const std::string &name,int16_t v,bool bTCP);
	bool SetNetworkedInt32(const std::string &name,int32_t v,bool bTCP);
	bool SetNetworkedInt64(const std::string &name,int64_t v,bool bTCP);
	bool SetNetworkedLongDouble(const std::string &name,long double v,bool bTCP);
	bool SetNetworkedString(const std::string &name,const std::string &v,bool bTCP);
	bool SetNetworkedUInt8(const std::string &name,uint8_t v,bool bTCP);
	bool SetNetworkedUInt16(const std::string &name,uint16_t v,bool bTCP);
	bool SetNetworkedUInt32(const std::string &name,uint32_t v,bool bTCP);
	bool SetNetworkedUInt64(const std::string &name,uint64_t v,bool bTCP);
	bool SetNetworkedAngles(const std::string &name,const EulerAngles &v,bool bTCP);
	bool SetNetworkedColor(const std::string &name,const Color &v,bool bTCP);
	bool SetNetworkedVector(const std::string &name,const Vector3 &v,bool bTCP);
	bool SetNetworkedVector2(const std::string &name,const Vector2 &v,bool bTCP);
	bool SetNetworkedVector4(const std::string &name,const Vector4 &v,bool bTCP);
	bool SetNetworkedEntity(const std::string &name,const BaseEntity *ent,bool bTCP);

	virtual bool SetNetworkedBool(const std::string &name,bool v) override;
	virtual bool SetNetworkedChar(const std::string &name,char v) override;
	virtual bool SetNetworkedDouble(const std::string &name,double v) override;
	virtual bool SetNetworkedFloat(const std::string &name,float v) override;
	virtual bool SetNetworkedInt8(const std::string &name,int8_t v) override;
	virtual bool SetNetworkedInt16(const std::string &name,int16_t v) override;
	virtual bool SetNetworkedInt32(const std::string &name,int32_t v) override;
	virtual bool SetNetworkedInt64(const std::string &name,int64_t v) override;
	virtual bool SetNetworkedLongDouble(const std::string &name,long double v) override;
	virtual bool SetNetworkedString(const std::string &name,const std::string &v) override;
	virtual bool SetNetworkedUInt8(const std::string &name,uint8_t v) override;
	virtual bool SetNetworkedUInt16(const std::string &name,uint16_t v) override;
	virtual bool SetNetworkedUInt32(const std::string &name,uint32_t v) override;
	virtual bool SetNetworkedUInt64(const std::string &name,uint64_t v) override;
	virtual bool SetNetworkedAngles(const std::string &name,const EulerAngles &v) override;
	virtual bool SetNetworkedColor(const std::string &name,const Color &v) override;
	virtual bool SetNetworkedVector(const std::string &name,const Vector3 &v) override;
	virtual bool SetNetworkedVector2(const std::string &name,const Vector2 &v) override;
	virtual bool SetNetworkedVector4(const std::string &name,const Vector4 &v) override;
	virtual bool SetNetworkedEntity(const std::string &name,const BaseEntity *ent) override;
	*/
	pragma::NetEventId RegisterNetEvent(const std::string &name) const;

	// Net Events
	void SendNetEventTCP(pragma::NetEventId eventId,NetPacket &data) const;
	void SendNetEventTCP(pragma::NetEventId eventId,NetPacket &data,nwm::RecipientFilter &rp) const;
	void SendNetEventTCP(pragma::NetEventId eventId) const;
	void SendNetEventUDP(pragma::NetEventId eventId,NetPacket &data) const;
	void SendNetEventUDP(pragma::NetEventId eventId,NetPacket &data,nwm::RecipientFilter &rp) const;
	void SendNetEventUDP(pragma::NetEventId eventId) const;
	virtual Bool ReceiveNetEvent(pragma::BasePlayerComponent &pl,pragma::NetEventId,NetPacket &packet);
	//
};

inline DLLSERVER Con::c_cout& operator<<(Con::c_cout &os,SBaseEntity &ent) {return ent.print(os);}

#endif