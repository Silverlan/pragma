#ifndef __BASENPC_H__
#define __BASENPC_H__
/*
#include "pragma/networkdefinitions.h"
#include <pragma/math/orientation.h>
#include "pragma/ai/navsystem.h"
#include "pragma/ai/ai_definitions.h"
#include "pragma/model/animation/activities.h"
#include "pragma/entities/prop/prop_base.h"
#include <vector>
#include <atomic>
#include <mutex>

class BaseEntity;
class PhysObj;
class Model;
class DLLNETWORK BaseNPC
	: virtual public BaseCharacter,public BaseProp
{
public:
	enum class DLLNETWORK MoveResult : uint32_t
	{
		TargetUnreachable = 0,
		TargetReached,
		WaitingForPath,
		MovingToTarget
	};
	static const char *MoveResultToString(MoveResult result);
	struct DLLNETWORK MoveInfo
	{
		MoveInfo() {}
		MoveInfo(Activity act);
		MoveInfo(Activity act,bool bMoveOnPath);
		MoveInfo(Activity act,bool bMoveOnPath,const Vector3 &faceTarget,float moveSpeed,float turnSpeed);
		Activity activity = Activity::Run;
		bool moveOnPath = true;
		Vector3 faceTarget = {std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN()};
		float moveSpeed = std::numeric_limits<float>::quiet_NaN();
		float turnSpeed = std::numeric_limits<float>::quiet_NaN();
	};
protected:
	struct PathInfo
	{
		PathInfo(const std::shared_ptr<RcPathResult> &path);
		std::shared_ptr<RcPathResult> path;
		std::array<std::unique_ptr<Vector3>,2> splineNodes; // Antepenult and penultimate
		uint32_t pathIdx;
	};
	// Thread
	static bool s_bThreadInitialized;
	static std::thread s_tAiNav;
	static std::atomic<uint32_t> s_npcCount;
	static std::mutex s_aiQueueMutex;
	struct PathQuery
	{
		PathQuery(const Vector3 &start,const Vector3 &end);
		std::atomic<bool> complete;
		std::shared_ptr<PathInfo> pathInfo;
		Vector3 start;
		Vector3 end;
	};
	static std::queue<std::shared_ptr<PathQuery>> s_aiNavQueue;
	//
protected:
	BaseNPC();
	virtual ~BaseNPC();
	BaseEntity *m_entity;
	enum class DLLNETWORK PathResult : uint32_t
	{
		Idle = 0,
		Updating,
		Success,
		Failed
	};
	
	// Navigation Path
	struct {
		std::shared_ptr<PathQuery> queuedPath;
		std::shared_ptr<PathInfo> pathInfo;
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
	} m_moveInfo;

	struct {
		bool moving = false;
		bool blend = false;
	} m_animMoveInfo;

	struct {
		double nextObstructionCheck;
		bool pathObstructed;
		Vector3 resolveDirection;
		std::vector<EntityHandle> sweepFilter;
	} m_obstruction;

	enum class LookTargetType : uint8_t
	{
		None,
		Position,
		Entity
	};
	struct {
		std::array<float,2> turnSpeed = {{0.f,0.f}};

		LookTargetType lookTargetType = LookTargetType::None;
		bool neckTurned = false;
		float lookTime = std::numeric_limits<float>::max();
		Vector3 lookTarget;
		EntityHandle hEntityLookTarget;
	} m_neckInfo;

	virtual void OnLookTargetChanged();
	void ResetPath();

	int m_seqIdle;
	std::unordered_map<int32_t,float> m_animIdMoveSpeed;
	std::unordered_map<std::string,float> m_animMoveSpeed;
	float m_lastMovementBlendScale = 0.f;
	virtual void InitializePhysObj(PhysObj *phys) override;
	virtual void Think(double tDelta);

	void SetPathNodeIndex(uint32_t nodeIdx,const Vector3 &prevPos);
	void UpdatePath();
	void PathStep(float tDelta);
	void LookAtStep(float tDelta);
	void ResolvePathObstruction(Vector3 &dir);
	bool CanMove() const;

	virtual void InitializeController() override;
	virtual void OnPathNodeChanged(uint32_t nodeIdx);
	void BlendAnimationMovement(std::vector<Orientation> &boneOrientations,std::vector<Vector3> *boneScales);
	float GetMaxSpeed(bool bUseAnimSpeedIfAvailable=true) const;
	virtual Vector2 CalcMovementSpeed() const override;
	virtual float CalcAirMovementModifier() const override;
	virtual float CalcMovementAcceleration() const override;
	virtual Vector3 CalcMovementDirection(const Vector3 &forward,const Vector3 &right) const override;
	virtual bool IsObstruction(const BaseEntity &ent) const;
public:
	virtual void Initialize();
	virtual void Spawn();
	virtual bool TurnStep(const Vector3 &target,float &turnAngle,const float *turnSpeed=nullptr);
	bool TurnStep(const Vector3 &target,const float *turnSpeed=nullptr);
	void ClearLookTarget();
	void SetLookTarget(const Vector3 &pos,float t=std::numeric_limits<float>::max());
	void SetLookTarget(const BaseEntity &ent,float t=std::numeric_limits<float>::max());
	Vector3 GetLookTarget() const;

	void SetMoveSpeed(int32_t animId,float speed);
	void SetMoveSpeed(const std::string &name,float speed);
	float GetMoveSpeed(int32_t animId) const;
	bool GetMoveSpeed(int32_t animId,float &speed) const;
	void ClearMoveSpeed(int32_t animId);
	void ClearMoveSpeed(const std::string &name);
	virtual void SimulateMovement(double tDelta) override;
	virtual bool IsMoving() const override;
	float GetDistanceToMoveTarget() const;
	const Vector3 &GetMoveTarget() const;
	MoveResult MoveTo(const Vector3 &pos,const MoveInfo &info={});
	virtual void OnPathChanged();
	virtual void OnPathDestinationReached();
	bool HasReachedDestination() const;
	Activity GetMoveActivity() const;
	virtual bool UpdateMovement() override;
	void StopMoving();
};
*/
#endif