#ifndef __C_ANIMATED_COMPONENT_HPP__
#define __C_ANIMATED_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_animated_component.hpp>

struct Eyeball;
namespace pragma
{
	void initialize_articulated_buffers();
	void clear_articulated_buffers();
	const std::shared_ptr<prosper::UniformResizableBuffer> &get_instance_bone_buffer();

	class DLLCLIENT CAnimatedComponent final
		: public BaseAnimatedComponent,
		public CBaseNetComponent
	{
	public:
		enum class StateFlags : uint8_t
		{
			None = 0u,
			BoneBufferDirty = 1u
		};

		struct EyeballState
		{
			Vector3 origin = {};
			Vector3 forward = {};
			Vector3 right = {};
			Vector3 up = {};
			Mat3x4 mat = {};
		};

		struct EyeballConfig
		{
			Vector3 eyeShift = {};
			bool eyeMove = false;
			Vector2 jitter = {};
			float eyeSize = 0.f;
			float dilation = 0.f;
		};

		struct EyeballData
		{
			EyeballState state = {};
			EyeballConfig config = {};
		};

		static ComponentEventId EVENT_ON_SKELETON_UPDATED;
		static ComponentEventId EVENT_ON_BONE_MATRICES_UPDATED;
		static ComponentEventId EVENT_ON_BONE_BUFFER_INITIALIZED;
		static ComponentEventId EVENT_ON_EYEBALLS_UPDATED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		CAnimatedComponent(BaseEntity &ent) : BaseAnimatedComponent(ent) {}

		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}

		std::weak_ptr<prosper::Buffer> GetBoneBuffer() const;
		const std::vector<Mat4> &GetBoneMatrices() const;
		std::vector<Mat4> &GetBoneMatrices();
		void UpdateBoneMatrices();
		void UpdateBoneBuffer(prosper::PrimaryCommandBuffer &commandBuffer);
		void InitializeBoneBuffer();
		virtual std::optional<Mat4> GetVertexTransformMatrix(const ModelSubMesh &subMesh,uint32_t vertexId) const override;

		uint32_t OnSkeletonUpdated();
		bool MaintainAnimations(double dt) override;

		const EyeballConfig *GetEyeballConfig(uint32_t eyeballIndex) const;
		EyeballConfig *GetEyeballConfig(uint32_t eyeballIndex);
		void SetEyeballConfig(const EyeballConfig &eyeballConfig);
		EyeballData *GetEyeballData(uint32_t eyeballIndex);
		const EyeballData *GetEyeballData(uint32_t eyeballIndex) const;
		bool GetEyeballProjectionVectors(uint32_t eyeballIndex,Vector4 &outProjU,Vector4 &outProjV) const;

		const Vector3 &GetViewTarget() const;
		void SetViewTarget(const Vector3 &viewTarget);

		physics::Transform CalcEyeballPose(uint32_t eyeballIndex) const;
		
		void SetBoneBufferDirty();
		void UpdateEyeballs();
	protected:
		Vector3 GetClampedViewTarget() const;
		void UpdateEyeball(const Eyeball &eyeball,uint32_t eyeballIndex);
		virtual void OnModelChanged(const std::shared_ptr<Model> &mdl) override;
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	private:
		std::shared_ptr<prosper::Buffer> m_boneBuffer = nullptr;
		std::vector<Mat4> m_boneMatrices;
		EyeballConfig m_eyeballConfig = {};
		std::vector<EyeballData> m_eyeballData = {};
		Vector3 m_viewTarget = {};
		std::shared_ptr<prosper::DescriptorSetGroup> m_boneDescSetGroup = nullptr;
		StateFlags m_stateFlags = StateFlags::BoneBufferDirty;
	};

	// Events

	struct DLLCLIENT CEOnSkeletonUpdated
		: public ComponentEvent
	{
		CEOnSkeletonUpdated(uint32_t &physRootBoneId);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		uint32_t &physRootBoneId;
	};

	struct DLLCLIENT CEOnBoneBufferInitialized
		: public ComponentEvent
	{
		CEOnBoneBufferInitialized(const std::shared_ptr<prosper::Buffer> &buffer);
		virtual void PushArguments(lua_State *l) override;
		std::shared_ptr<prosper::Buffer> buffer;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CAnimatedComponent::StateFlags)

#endif
