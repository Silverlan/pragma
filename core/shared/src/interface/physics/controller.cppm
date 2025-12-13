// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:physics.controller;

export import :physics.collision_object;

export {
	namespace pragma::physics {
		class DLLNETWORK IController : public IBase, public IWorldObject {
		  public:
			enum class CollisionFlags : uint32_t { None = 0u, Down = 1u, Sides = Down << 1u, Up = Sides << 1u };

			enum class ShapeType : uint8_t { Capsule = 0, Box };

			virtual void OnRemove() override;

			void Move(Vector3 &disp);
			virtual CollisionFlags GetCollisionFlags() const = 0;
			virtual IShape *GetGroundShape() const = 0;
			virtual IRigidBody *GetGroundBody() const = 0;
			virtual IMaterial *GetGroundMaterial() const = 0;
			virtual bool IsTouchingGround() const = 0;
			virtual std::optional<Vector3> GetGroundTouchPos() const = 0;
			virtual std::optional<Vector3> GetGroundTouchNormal() const = 0;
			virtual Vector3 GetDimensions() const = 0;
			virtual void SetDimensions(const Vector3 &dimensions) = 0;
			virtual void Resize(float newHeight) = 0;
			virtual void SetPos(const Vector3 &pos) = 0;
			virtual Vector3 GetPos() const = 0;
			virtual void SetFootPos(const Vector3 &footPos) = 0;
			virtual Vector3 GetFootPos() const = 0;
			virtual void SetUpDirection(const Vector3 &up) = 0;
			virtual Vector3 GetUpDirection() const = 0;

			void SetMoveVelocity(const Vector3 &vel);
			void AddMoveVelocity(const Vector3 &vel);
			const Vector3 &GetMoveVelocity() const;

			virtual void SetSlopeLimit(math::Degree slopeLimit) = 0;
			virtual math::Degree GetSlopeLimit() const = 0;

			virtual void SetStepHeight(float stepHeight) = 0;
			virtual float GetStepHeight() const = 0;

			virtual Vector3 GetLinearVelocity() const = 0;
			virtual void SetLinearVelocity(const Vector3 &vel) = 0;

			const Vector3 &GetHalfExtents() const { return m_halfExtents; }
			ShapeType GetShapeType() const { return m_shapeType; }

			ICollisionObject *GetCollisionObject();
			const ICollisionObject *GetCollisionObject() const;

			const IConvexShape *GetShape() const;
			IConvexShape *GetShape();
			const Vector3 &GetLastMoveDisplacement() const;
			virtual void InitializeLuaObject(lua::State *lua) override;
		  protected:
			virtual void DoMove(Vector3 &disp) = 0;
			IController(IEnvironment &env, const util::TSharedHandle<ICollisionObject> &collisionObject, const Vector3 &halfExtents, ShapeType shapeType);
			util::TSharedHandle<ICollisionObject> m_collisionObject = nullptr;
			Vector3 m_moveVelocity = {};
			Vector3 m_halfExtents {};
			ShapeType m_shapeType = ShapeType::Capsule;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::physics::IController::CollisionFlags)
};
