// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.base_ai;

export import :entities.components.base;

import :ai.nav_system;
export import :model.animation.enums;
import :entities.components.movement;
import :physics.raycast_filter;
export import :types;

export {
	namespace pragma {
		namespace ai {
			namespace navigation {
				struct PathInfo {
					PathInfo(const std::shared_ptr<RcPathResult> &path);
					std::shared_ptr<RcPathResult> path;
					std::array<std::unique_ptr<Vector3>, 2> splineNodes; // Antepenult and penultimate
					uint32_t pathIdx;
				};
				struct PathQuery {
					PathQuery(const Vector3 &start, const Vector3 &end);
					std::atomic<bool> complete;
					std::shared_ptr<PathInfo> pathInfo;
					Vector3 start;
					Vector3 end;
					util::WeakHandle<BaseAIComponent> npc = {};
				};
				struct NavThread {
					std::thread thread;
					std::atomic<bool> running = true;
					CallbackHandle releaseCallback = {};
					std::queue<std::shared_ptr<PathQuery>> queryQueue {};

					std::mutex pendingQueueMutex = {};
					std::queue<std::shared_ptr<PathQuery>> pendingQueue {};
				};
			};
		};

		class DLLNETWORK BaseAIComponent : public BaseEntityComponent {
		  public:
			enum class MoveResult : uint32_t { TargetUnreachable = 0, TargetReached, WaitingForPath, MovingToTarget };
			enum class SnapshotFlags : uint8_t { None = 0u, Moving = 1u, MoveSpeed = Moving << 1, TurnSpeed = MoveSpeed << 1u, FaceTarget = TurnSpeed << 1u };
			static const char *MoveResultToString(MoveResult result);
			static void ReloadNavThread(Game &game);
			static void ReleaseNavThread();
			struct DLLNETWORK MoveInfo {
				MoveInfo() {}
				MoveInfo(Activity act);
				MoveInfo(Activity act, bool bMoveOnPath);
				MoveInfo(Activity act, bool bMoveOnPath, const Vector3 &faceTarget, float moveSpeed, float turnSpeed);
				Activity activity = Activity::Run;
				bool moveOnPath = true;
				Vector3 faceTarget = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
				float moveSpeed = std::numeric_limits<float>::quiet_NaN();
				float turnSpeed = std::numeric_limits<float>::quiet_NaN();
				float destinationTolerance = 10.f; // The moveTarget will be considered 'reached', if the NPC is at most this distance away from it
			};
			virtual void Initialize() override;
			virtual void Spawn();
			virtual bool TurnStep(const Vector3 &target, float &turnAngle, const float *turnSpeed = nullptr);
			virtual void OnEntitySpawn() override;
			bool TurnStep(const Vector3 &target, const float *turnSpeed = nullptr);
			void ClearLookTarget();
			void SetLookTarget(const Vector3 &pos, float t = std::numeric_limits<float>::max());
			void SetLookTarget(const ecs::BaseEntity &ent, float t = std::numeric_limits<float>::max());
			Vector3 GetLookTarget() const;

			void SetMoveSpeed(int32_t animId, float speed);
			void SetMoveSpeed(const std::string &name, float speed);
			float GetMoveSpeed(int32_t animId) const;
			bool GetMoveSpeed(int32_t animId, float &speed) const;
			void ClearMoveSpeed(int32_t animId);
			void ClearMoveSpeed(const std::string &name);
			bool IsMoving() const;
			float GetDistanceToMoveTarget() const;
			const Vector3 &GetMoveTarget() const;
			MoveResult MoveTo(const Vector3 &pos, const MoveInfo &info = {});
			virtual void OnPathChanged();
			virtual void OnPathDestinationReached();
			bool HasReachedDestination() const;
			Activity GetMoveActivity() const;
			void StopMoving();
			Vector3 GetUpDirection() const;
			bool CanMove() const;

			float GetMaxSpeed(bool bUseAnimSpeedIfAvailable = true) const;
		  protected:
			virtual void OnModelChanged(const std::shared_ptr<asset::Model> &model);
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
			static std::atomic<uint32_t> s_npcCount;
			static std::shared_ptr<ai::navigation::NavThread> s_navThread;
			//
		  protected:
			BaseAIComponent(ecs::BaseEntity &ent);
			virtual ~BaseAIComponent() override;
			void UpdateMovementProperties();
			virtual void UpdateMovementProperties(MovementComponent &movementC);
			virtual void OnPhysicsInitialized();
			enum class PathResult : uint32_t { Idle = 0, Updating, Success, Failed };

			// Navigation Path
			struct {
				std::shared_ptr<ai::navigation::PathQuery> queuedPath;
				std::shared_ptr<ai::navigation::PathInfo> pathInfo;
				Vector3 pathTarget;
				bool bPathUpdateRequired = false;
				bool bTargetReached = false;
				PathResult pathState = PathResult::Idle;
			} m_navInfo;

			struct {
				Vector3 moveDir;
				Vector3 moveTarget;
				bool moving = false;
				std::unique_ptr<float> moveSpeed = nullptr;
				std::unique_ptr<float> turnSpeed = nullptr;
				bool moveOnPath = false;
				Activity moveActivity = Activity::Run;
				std::unique_ptr<Vector3> faceTarget = nullptr;
				float destinationTolerance = 10.f;
			} m_moveInfo;

			struct {
				bool moving = false;
				bool blend = false;
			} m_animMoveInfo;

			struct {
				double nextObstructionCheck;
				bool pathObstructed;
				Vector3 resolveDirection;
				std::shared_ptr<physics::IRayCastFilterCallback> sweepFilter;
			} m_obstruction;

			enum class LookTargetType : uint8_t { None, Position, Entity };
			struct {
				std::array<float, 2> turnSpeed = {{0.f, 0.f}};

				LookTargetType lookTargetType = LookTargetType::None;
				bool neckTurned = false;
				float lookTime = std::numeric_limits<float>::max();
				Vector3 lookTarget;
				EntityHandle hEntityLookTarget;
			} m_neckInfo;

			virtual void OnLookTargetChanged();
			void ResetPath();

			NetEventId m_netEvSetLookTarget = INVALID_NET_EVENT;
			int m_seqIdle;
			std::unordered_map<int32_t, float> m_animIdMoveSpeed;
			std::unordered_map<std::string, float> m_animMoveSpeed;
			float m_lastMovementBlendScale = 0.f;

			virtual void OnTick(double tDelta) override;

			void SetPathNodeIndex(uint32_t nodeIdx, const Vector3 &prevPos);
			void UpdatePath();
			void PathStep(float tDelta);
			void LookAtStep(float tDelta);
			void ResolvePathObstruction(Vector3 &dir);

			virtual void OnPathNodeChanged(uint32_t nodeIdx);
			void BlendAnimationMovementMT(std::vector<math::Transform> &bonePoses, std::vector<Vector3> *boneScales);
			Vector2 CalcMovementSpeed() const;
			float CalcAirMovementModifier() const;
			float CalcMovementAcceleration() const;
			Vector3 CalcMovementDirection() const;
			virtual bool IsObstruction(const ecs::BaseEntity &ent) const;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseAIComponent::SnapshotFlags)
};
