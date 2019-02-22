#ifndef __BASE_SHOOTER_COMPONENT_HPP__
#define __BASE_SHOOTER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/entities/baseentity_net_event_manager.hpp"
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <typeindex>
#include <mathutil/uvec.h>

struct BulletInfo;
struct TraceResult;
class TraceData;
class NetPacket;
class PhysObj;
class PhysCollisionObject;
namespace pragma
{
	struct DLLNETWORK CEOnBulletsFired
		: public ComponentEvent
	{
		CEOnBulletsFired(const BulletInfo &bulletInfo,const std::vector<TraceResult> &hitTargets);
		virtual void PushArguments(lua_State *l) override;
		const BulletInfo &bulletInfo;
		const std::vector<TraceResult> &hitTargets;
	};
	struct DLLNETWORK CEOnFireBullets
		: public ComponentEvent
	{
		CEOnFireBullets(const BulletInfo &bulletInfo,Vector3 &bulletOrigin,Vector3 &bulletDir,Vector3 *effectsOrigin);
		virtual void PushArguments(lua_State *l) override;
		const BulletInfo &bulletInfo;
		Vector3 &bulletOrigin;
		Vector3 &bulletDir;
		Vector3 *effectsOrigin;

		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
	};
	class BasePlayerComponent;
	class DLLNETWORK BaseShooterComponent
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_ON_FIRE_BULLETS;
		static ComponentEventId EVENT_ON_BULLETS_FIRED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		virtual void Initialize() override;

		void GetBulletTraceData(const BulletInfo &bulletInfo,TraceData &data) const;

		virtual void OnFireBullets(const BulletInfo &bulletInfo,Vector3 &bulletOrigin,Vector3 &bulletDir,Vector3 *effectsOrigin=nullptr);
		virtual void FireBullets(const BulletInfo &bulletInfo,std::vector<TraceResult> &outHitTargets,bool bMaster=true)=0;
	protected:
		BaseShooterComponent(BaseEntity &ent);
		struct DLLNETWORK NextBulletInfo
		{
			std::vector<Vector3> destinations;
			EntityHandle source;
		};
		mutable std::unique_ptr<NextBulletInfo> m_nextBullet = nullptr;
		void ReceiveBulletEvent(NetPacket &packet,pragma::BasePlayerComponent *pl=nullptr);
		std::vector<Vector3> GetBulletDestinations(const Vector3 &origin,const Vector3 &dir,const BulletInfo &bulletInfo);
		virtual bool OnBulletHit(const BulletInfo &bulletInfo,const TraceData &data,PhysObj *phys,PhysCollisionObject *col,const btCollisionWorld::LocalRayResult &result);
		
		pragma::NetEventId m_netEvFireBullets = pragma::INVALID_NET_EVENT;
	};
};

#endif
