#ifndef __PHYS_CONTROLLER_HPP__
#define __PHYS_CONTROLLER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/base.hpp"
#include "pragma/lua/baseluaobj.h"
#include <memory>
#include <sharedutils/def_handle.h>
#include <mathutil/uvec.h>

namespace pragma::physics
{
	class IConvexShape;
	class IEnvironment;
	class Transform;
	class IGhostObject;
	class ICollisionObject;
	class DLLNETWORK IController
		: public IBase
	{
	public:
		enum class CollisionFlags : uint32_t
		{
			None = 0u,
			Down = 1u,
			Sides = Down<<1u,
			Up = Sides<<1u
		};

		virtual void OnRemove() override;

		CollisionFlags Move(Vector3 &disp);
		virtual Vector3 GetDimensions() const=0;
		virtual void SetDimensions(const Vector3 &dimensions)=0;
		virtual void Resize(float newHeight)=0;

		ICollisionObject *GetCollisionObject();
		const ICollisionObject *GetCollisionObject() const;

		const pragma::physics::IConvexShape *GetShape() const;
		pragma::physics::IConvexShape *GetShape();
		const Vector3 &GetLastMoveDisplacement() const;
	protected:
		virtual CollisionFlags DoMove(Vector3 &disp)=0;
		IController(IEnvironment &env,const util::TSharedHandle<ICollisionObject> &collisionObject);
		util::TSharedHandle<ICollisionObject> m_collisionObject = nullptr;
		Vector3 m_moveDisplacement = {};
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::physics::IController::CollisionFlags)

#endif
