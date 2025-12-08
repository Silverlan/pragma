// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:types;

export import std.compat;

export {
	namespace pragma {
		class BaseEntityComponent;
		class BaseFilterComponent;
		struct ComponentMemberInfo;
		class Engine;
		class Game;
		class PanimaComponent;
		class NetworkState;

		namespace audio {
			class ALSound;
		}

		namespace asset {
			class ModelManager;
			class Model;
			class ModelMeshGroup;
		}

		namespace animation {
			class Animation;
		}

		namespace geometry {
			class ModelSubMesh;
			class ModelMesh;
		}

		namespace ecs {
			class BaseEntity;
			class EntityIterator;
		}

		namespace game {
			class DamageInfo;
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

			enum class PhysicsType : int32_t;
			enum class CollisionMask : uint32_t;
			enum class RayCastFlags : uint32_t;
			enum class CollisionType : int32_t;
			enum class MoveType : int32_t;
			enum class RayCastHitType : uint8_t;
		}
	}
}
