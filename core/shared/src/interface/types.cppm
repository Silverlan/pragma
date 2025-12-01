// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:types;

export import std.compat;

export {
	class ALSound;
	namespace pragma {
		class BaseEntityComponent;
		class BaseFilterComponent;
		struct ComponentMemberInfo;
		class Engine;
		class Game;
		class Model;
		class ModelMeshGroup;
		class ModelSubMesh;
		class PanimaComponent;

		namespace asset {
			class ModelManager;
		}

		namespace animation {
			class Animation;
		}

		namespace ecs {
			class BaseEntity;
			class EntityIterator;
		}

		class BaseTransformComponent;
		class BasePhysicsComponent;
		class BaseModelComponent;
		class BaseGenericComponent;
		class BaseChildComponent;
		class BaseAnimatedComponent;
		class BaseWeaponComponent;
		class BaseVehicleComponent;
		class BaseAIComponent;
		class BaseCharacterComponent;
		class BasePlayerComponent;
		class BaseTimeScaleComponent;
		class BaseNameComponent;

		namespace physics {
			class IEnvironment;
			class IRigidBody;
			class IShape;
			class PhysObj;

			enum class PHYSICSTYPE : int;
			enum class CollisionMask : uint32_t;
			enum class RayCastFlags : uint32_t;
			enum class COLLISIONTYPE : int;
			enum class MOVETYPE : int;
			enum class RayCastHitType : uint8_t;
		}
	}
	class DamageInfo;
	class ModelMesh;
	class NetworkState;
}
