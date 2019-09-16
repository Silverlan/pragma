#ifndef __PHYS_CONTROLLER_HPP__
#define __PHYS_CONTROLLER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/base.hpp"
#include "pragma/lua/baseluaobj.h"
#include <memory>
#include <optional>
#include <sharedutils/def_handle.h>
#include <mathutil/uvec.h>

namespace pragma::physics
{
	class IShape;
	class IConvexShape;
	class IEnvironment;
	class Transform;
	class IGhostObject;
	class IRigidBody;
	class IMaterial;
	class ICollisionObject;
	class DLLNETWORK IController
		: public IBase,public IWorldObject
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

		void Move(Vector3 &disp);
		virtual CollisionFlags GetCollisionFlags() const=0;
		virtual IShape *GetGroundShape() const=0;
		virtual IRigidBody *GetGroundBody() const=0;
		virtual IMaterial *GetGroundMaterial() const=0;
		virtual bool IsTouchingGround() const=0;
		virtual std::optional<Vector3> GetGroundTouchPos() const=0;
		virtual std::optional<Vector3> GetGroundTouchNormal() const=0;
		virtual Vector3 GetDimensions() const=0;
		virtual void SetDimensions(const Vector3 &dimensions)=0;
		virtual void Resize(float newHeight)=0;
		virtual void SetPos(const Vector3 &pos)=0;
		virtual Vector3 GetPos() const=0;
		virtual void SetFootPos(const Vector3 &footPos)=0;
		virtual Vector3 GetFootPos() const=0;
		virtual void SetUpDirection(const Vector3 &up)=0;
		virtual Vector3 GetUpDirection() const=0;

		void SetMoveVelocity(const Vector3 &vel);
		void AddMoveVelocity(const Vector3 &vel);
		const Vector3 &GetMoveVelocity() const;

		virtual void SetSlopeLimit(umath::Degree slopeLimit)=0;
		virtual umath::Degree GetSlopeLimit() const=0;

		virtual void SetStepHeight(float stepHeight)=0;
		virtual float GetStepHeight() const=0;

		virtual Vector3 GetLinearVelocity() const=0;
		virtual void SetLinearVelocity(const Vector3 &vel)=0;

		ICollisionObject *GetCollisionObject();
		const ICollisionObject *GetCollisionObject() const;

		const pragma::physics::IConvexShape *GetShape() const;
		pragma::physics::IConvexShape *GetShape();
		const Vector3 &GetLastMoveDisplacement() const;
		virtual void InitializeLuaObject(lua_State *lua) override;
	protected:
		virtual void DoMove(Vector3 &disp)=0;
		IController(IEnvironment &env,const util::TSharedHandle<ICollisionObject> &collisionObject);
		util::TSharedHandle<ICollisionObject> m_collisionObject = nullptr;
		Vector3 m_moveVelocity = {};
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::physics::IController::CollisionFlags)

#endif
