#ifndef __FRAME_H__
#define __FRAME_H__
#include "pragma/networkdefinitions.h"
#include "pragma/model/animation/skeleton.h"
#include <mathutil/eulerangles.h>
#include <pragma/util/orientedpoint.h>
#include <vector>
#include <unordered_map>

class Animation;
class Model;
namespace pragma::physics {class Transform; class ScaledTransform;};
class DLLNETWORK Frame
	: public std::enable_shared_from_this<Frame>
{
public:
	static std::shared_ptr<Frame> Create(unsigned int numBones);
	static std::shared_ptr<Frame> Create(const Frame &other);

	void SetBonePosition(unsigned int boneID,const Vector3 &pos);
	void SetBoneOrientation(unsigned int boneID,const Quat &orientation);
	void SetBoneScale(uint32_t boneId,const Vector3 &scale);
	Vector3 *GetBonePosition(unsigned int boneID);
	Quat *GetBoneOrientation(unsigned int boneID);
	Vector3 *GetBoneScale(uint32_t boneId);
	const Vector3 *GetBonePosition(unsigned int boneID) const;
	const Quat *GetBoneOrientation(unsigned int boneID) const;
	const Vector3 *GetBoneScale(uint32_t boneId) const;
	bool GetBonePose(uint32_t boneId,pragma::physics::ScaledTransform &outTransform) const;
	void SetBonePose(uint32_t boneId,const pragma::physics::ScaledTransform &pose);
	void SetBonePose(uint32_t boneId,const pragma::physics::Transform &pose);
	bool GetBoneMatrix(unsigned int boneID,Mat4 *mat);
	Vector2 *GetMoveOffset();
	void GetMoveOffset(float *x,float *z);
	void SetMoveOffset(float x,float z=0);
	void SetMoveOffset(Vector2 move);
	void Localize(const Animation &anim,const Skeleton &skeleton);
	void Globalize(const Animation &anim,const Skeleton &skeleton);
	uint32_t GetBoneCount() const;
	void SetBoneCount(uint32_t numBones);
	std::pair<Vector3,Vector3> CalcRenderBounds(const Animation &anim,const Model &mdl) const;
	void Rotate(const Quat &rot);
	void Translate(const Vector3 &t);
	void Rotate(const Skeleton &skeleton,const Quat &rot);
	void Translate(const Skeleton &skeleton,const Vector3 &t);
	void Rotate(const Animation &anim,const Skeleton &skeleton,const Quat &rot);
	void Translate(const Animation &anim,const Skeleton &skeleton,const Vector3 &t);
	void Scale(const Vector3 &scale);

	bool HasScaleTransforms() const;

	const std::vector<OrientedPoint> &GetBoneTransforms() const;
	const std::vector<Vector3> &GetBoneScales() const;
	std::vector<OrientedPoint> &GetBoneTransforms();
	std::vector<Vector3> &GetBoneScales();
private:
	Frame(unsigned int numBones);
	Frame(const Frame &other);
	std::vector<OrientedPoint> m_bones;
	std::vector<Vector3> m_scales;
	std::unique_ptr<Vector2> m_move;
	std::vector<uint32_t> GetLocalRootBoneIds(const Animation &anim,const Skeleton &skeleton) const;

	void UpdateScales();
};
#endif