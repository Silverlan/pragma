#ifndef __MDL_ANIMATION_H__
#define __MDL_ANIMATION_H__

#include <uvec.h>
#include <filesystem.h>
#include <array>
#include <animation.h>
#include "mdl_bone.h"
#include "quaternion48.h"
#include "quaternion64.h"
#include "vector48.h"

#define STUDIO_LX 0x00000040
#define STUDIO_LY 0x00000080
#define STUDIO_DELTA 0x04

#define STUDIO_ANIM_RAWPOS	0x01 // Vector48
#define STUDIO_ANIM_RAWROT	0x02 // Quaternion48
#define STUDIO_ANIM_ANIMPOS	0x04 // mstudioanim_valueptr_t
#define STUDIO_ANIM_ANIMROT	0x08 // mstudioanim_valueptr_t
#define STUDIO_ANIM_DELTA	0x10
#define STUDIO_ANIM_RAWROT2	0x20 // Quaternion64

#define STUDIO_LOOPING	0x0001		// ending frame should be the same as the starting frame
#define STUDIO_SNAP		0x0002		// do not interpolate between previous animation and this one
#define STUDIO_DELTA	0x0004		// this sequence "adds" to the base sequences, not slerp blends
#define STUDIO_AUTOPLAY	0x0008		// temporary flag that forces the sequence to always play
#define STUDIO_POST		0x0010		// 
#define STUDIO_ALLZEROS	0x0020		// this animation/sequence has no real animation data

#define STUDIO_CYCLEPOSE 0x0080		// cycle index is taken from a pose parameter index
#define STUDIO_REALTIME	0x0100		// cycle index is taken from a real-time clock, not the animations cycle index
#define STUDIO_LOCAL	0x0200		// sequence has a local context sequence
#define STUDIO_HIDDEN	0x0400		// don't show in default selection views
#define STUDIO_OVERRIDE	0x0800		// a forward declared sequence (empty)
#define STUDIO_ACTIVITY	0x1000		// Has been updated at runtime to activity index
#define STUDIO_EVENT	0x2000		// Has been updated at runtime to event index
#define STUDIO_WORLD	0x4000		// sequence blends in worldspace

#define STUDIO_FRAMEANIM 0x0040		// animation is encoded as by frame x bone instead of RLE bone x frame
#define STUDIO_NOFORCELOOP 0x8000	// do not force the animation loop
#define STUDIO_EVENT_CLIENT 0x10000	// Has been updated at runtime to event index on client

#pragma pack(push,1)
namespace import
{
	struct MdlInfo;
	namespace mdl
	{
		struct studiohdr_t;
		Quat euler_angles_to_quaternion(const Vector3 &ang);
		Vector3 quaternion_to_euler_angles(const Quat &q);
		struct mstudioanimblock_t
		{
			int32_t datastart;
			int32_t dataend;
		};

		struct mstudioanimsection_t
		{
			int32_t animBlock;
			int32_t animOffset;
		};

		struct mstudioanim_t
		{
			uint8_t boneIndex;
			uint8_t flags;
			int16_t nextSourceMdlAnimationOffset;
		};

		struct mstudioanimsections_t
		{
			int32_t animblock;
			int32_t animindex;
		};

		struct mstudioanimdesc_t
		{
			int32_t baseHeaderOffset;
			int32_t nameOffset;
			float fps;
			int32_t flags;
			int32_t frameCount;
			int32_t movementCount;
			int32_t movementOffset;
			std::array<int32_t,6> unused1;
			int32_t animBlock;
			int32_t animOffset;
			int32_t ikRuleCount;
			int32_t ikRuleOffset;
			int32_t animblockIkRuleOffset;
			int32_t localHierarchyCount;
			int32_t localHierarchyOffset;
			int32_t sectionOffset;
			int32_t sectionFrameCount;
			int16_t spanFrameCount;
			int16_t spanCount;
			int32_t spanOffset;
			float spanStallTime;
		};

		struct mstudioikrule_t
		{
			int32_t index;

			int32_t type;
			int32_t chain;

			int32_t bone;

			int32_t slot; // iktarget slot.  Usually same as chain.
			float height;
			float radius;
			float floor;
			Vector3 pos;
			Quat q;

			int32_t compressedikerrorindex;
			int32_t unused2;

			int32_t iStart;
			int32_t ikerrorindex;

			float start; // beginning of influence
			float peak; // start of full influence
			float tail; // end of full influence
			float end; // end of all influence

			float unused3;
			float contact; // frame footstep makes ground concact
			float drop; // how far down the foot should drop when reaching for IK
			float top; // top of the foot box

			int32_t unused6;
			int32_t unused7;
			int32_t unused8;

			int32_t szattachmentindex;		// name of world attachment

			std::array<int,7> unused;
		};

		struct mstudiomovement_t
		{
			int32_t endframe;
			int32_t motionflags;
			float v0; // velocity at start of block
			float v1; // velocity at end of block
			float angle; // YAW rotation at end of this blocks movement
			Vector3 vector; // movement vector relative to this blocks initial angle
			Vector3 position; // relative to start of animation???
		};

		using mstudioanim_valueptr_t = int16_t;

		union mstudioanimvalue_t
		{
			struct 
			{
				uint8_t valid;
				uint8_t total;
			} num;
			int16_t value;
		};
		class Animation;
		class AnimationDesc
		{
		private:
			mstudioanimdesc_t m_stdDesc;
			int32_t m_index;
			int32_t m_frameCount;
			float m_fps;
			std::string m_name;
			std::shared_ptr<Animation> m_animation;
			std::vector<mdl::mstudioanimsection_t> m_sections;
			std::vector<mstudiomovement_t> m_movements;
			mutable std::vector<std::shared_ptr<Animation>> m_sectionAnimations;

			void AdjustPositionAndRotationByPiecewiseMovement(const import::MdlInfo &info,uint32_t frameIndex,uint32_t boneIndex,const Vector3 &position,const Vector3 &rotation,Vector3 &outPosition,Vector3 &outRotation);
			void AdjustPositionAndRotation(const import::MdlInfo &info,uint32_t boneIndex,const Vector3 &position,const Vector3 &rotation,Vector3 &outPosition,Vector3 &outRotation);
		public:
			AnimationDesc(int32_t idx,const VFilePtr &f);
			const mstudioanimdesc_t &GetStudioDesc() const;
			const std::vector<mdl::mstudioanimsection_t> &GetStudioSections() const;
			std::vector<std::shared_ptr<Animation>> &GetSectionAnimations() const;
			int32_t GetFrameCount() const;
			float GetFPS() const;
			const std::string &GetName() const;
			const std::shared_ptr<Animation> &GetAnimation() const;
			void ReadMovements(const mdl::studiohdr_t &mdlHeader,uint64_t offset,uint64_t relOffset,const VFilePtr &f);
			void ReadAnimation(const mdl::studiohdr_t &mdlHeader,uint64_t offset,uint64_t relOffset,const VFilePtr &f);
			void ReadAnimationSections(const mdl::studiohdr_t &mdlHeader,uint64_t offset,const VFilePtr &f);
			std::shared_ptr<::Animation> CalcAnimation(const import::MdlInfo &info);

			void DebugPrint(const std::string &t="");
		};
		class Animation
		{
		public:
			/*class Frame
			{
			public:
				struct BoneTransform
				{
					Vector3 pos;
					Quat rot;
				};
			private:
				std::unordered_map<int8_t,BoneTransform> m_transforms;
			public:
				Frame()=default;
				void SetBonePos(int8_t boneId,const Vector3 &pos);
				void SetBoneRot(int8_t boneId,const Quat &rot);
				const Vector3 *GetBonePos(int8_t boneId) const;
				const Quat *GetBoneRot(int8_t boneId) const;
				const std::unordered_map<int8_t,BoneTransform> &GetBoneTransforms() const;
			};*/
			class Data
			{
			private:
				mstudioanim_t m_stdAnim;
				std::array<std::vector<mstudioanimvalue_t>,3> m_rotValues;
				std::array<std::vector<mstudioanimvalue_t>,3> m_posValues;
				Quaternion48 m_rot48;
				Quaternion64 m_rot64;
				Vector48 m_pos48;
			protected:
				Data(const VFilePtr &f);
			public:
				friend Animation;
				const mstudioanim_t &GetStudioAnim() const;
				const std::array<std::vector<mstudioanimvalue_t>,3> &GetRotValues() const;
				const std::array<std::vector<mstudioanimvalue_t>,3> &GetPosValues() const;
				const Quaternion48 GetRot48() const;
				const Quaternion64 GetRot64() const;
				const Vector48 GetPos48() const;
				Vector3 CalcBonePosition(uint32_t frameIndex,const Bone &bone) const;
				Vector3 CalcBoneRotation(uint32_t frameIndex,const Bone &bone) const;
			};
		private:
			std::vector<std::shared_ptr<Data>> m_data;
			//std::vector<std::shared_ptr<Frame>> m_frames;
			Vector3 CalcBonePosition(const mdl::Bone &bone,const std::array<mstudioanimvalue_t,3> &animValues,int32_t frame,float s=0.f);
			Quat CalcBoneRotation(const mdl::mstudiobone_t &bone,const mstudioanim_t &panim,const std::array<mstudioanimvalue_t,3> &animValues,int32_t frame,float s=1.f);
			void ReadMdlAnimValues(const mdl::studiohdr_t &header,const import::mdl::mstudioanimdesc_t &stdAnimDesc,std::vector<mstudioanimvalue_t> &stdAnimValues,const VFilePtr &f);
		public:
			Animation(const mdl::studiohdr_t &mdlHeader,const import::mdl::mstudioanimdesc_t &stdAnimDesc,const VFilePtr &f);
			Animation(const std::vector<std::shared_ptr<mdl::Bone>> &bones,uint32_t animBlockOffset,const std::vector<mdl::mstudioanimblock_t> &animBlocks,const VFilePtr &f);
			const std::vector<std::shared_ptr<Data>> &GetData() const;
		};
	};
};
#pragma pack(pop)

#endif
