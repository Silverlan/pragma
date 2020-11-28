/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_RENDER_COMPONENT_HPP__
#define __C_RENDER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/rendering/c_rendermode.h"
#include <pragma/util/orientedpoint.h>
#include <pragma/math/sphere.h>
#include <pragma/entities/components/base_render_component.hpp>
#include <mathutil/uvec.h>
#include <mathutil/boundingvolume.h>

#define ENTITY_RENDER_BUFFER_USE_STORAGE_BUFFER 1

namespace prosper {class IUniformResizableBuffer; class IDescriptorSet;};
namespace Intersection {struct LineMeshResult;};
namespace pragma
{
	class CModelComponent;
	class CAnimatedComponent;
	class CAttachableComponent;
	class CLightMapReceiverComponent;
	using RenderMeshIndex = uint32_t;
	using RenderBufferIndex = uint32_t;
	class DLLCLIENT CRenderComponent final
		: public BaseRenderComponent,
		public CBaseNetComponent
	{
	public:
		enum class StateFlags : uint16_t
		{
			None = 0u,
			RenderBufferDirty = 1u,
			ExemptFromOcclusionCulling = RenderBufferDirty<<1u,
			HasDepthBias = ExemptFromOcclusionCulling<<1u,
			EnableDepthPass = HasDepthBias<<1u,
			DisableShadows = EnableDepthPass<<1u,
			IsInstantiable = DisableShadows<<1u,
			InstantiationDisabled = IsInstantiable<<1u,
			RenderBoundsDirty = InstantiationDisabled<<1u,
			ShouldDraw = RenderBoundsDirty<<1u,
			ShouldDrawShadow = ShouldDraw<<1u
		};
		static constexpr auto USE_HOST_MEMORY_FOR_RENDER_DATA = true;

		static ComponentEventId EVENT_ON_UPDATE_RENDER_DATA_MT;
		static ComponentEventId EVENT_ON_RENDER_BUFFERS_INITIALIZED;
		static ComponentEventId EVENT_ON_RENDER_BOUNDS_CHANGED;
		static ComponentEventId EVENT_ON_RENDER_MODE_CHANGED;
		static ComponentEventId EVENT_SHOULD_DRAW;
		static ComponentEventId EVENT_SHOULD_DRAW_SHADOW;
		static ComponentEventId EVENT_ON_UPDATE_RENDER_BUFFERS;
		static ComponentEventId EVENT_ON_UPDATE_RENDER_MATRICES;
		static ComponentEventId EVENT_UPDATE_INSTANTIABILITY;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		CRenderComponent(BaseEntity &ent);
		const std::shared_ptr<prosper::IBuffer> &GetRenderBuffer() const;
		std::optional<RenderBufferIndex> GetRenderBufferIndex() const;
		prosper::IDescriptorSet *GetRenderDescriptorSet() const;

		static const std::vector<CRenderComponent*> &GetEntitiesExemptFromOcclusionCulling();
		static const std::shared_ptr<prosper::IUniformResizableBuffer> &GetInstanceBuffer();
		static void InitializeBuffers();
		static void ClearBuffers();

		virtual void Initialize() override;
		virtual ~CRenderComponent() override;

		std::vector<std::shared_ptr<ModelSubMesh>> &GetRenderMeshes();
		const std::vector<std::shared_ptr<ModelSubMesh>> &GetRenderMeshes() const;
		std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes();
		const std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes() const;

		RenderMeshGroup &GetLodRenderMeshGroup(uint32_t lod);
		const RenderMeshGroup &GetLodRenderMeshGroup(uint32_t lod) const;
		RenderMeshGroup &GetLodMeshGroup(uint32_t lod);
		const RenderMeshGroup &GetLodMeshGroup(uint32_t lod) const;

		uint64_t GetLastRenderFrame() const;
		void SetLastRenderFrame(unsigned long long &t);

		void SetLocalRenderBounds(Vector3 min,Vector3 max);

		const bounding_volume::AABB &GetLocalRenderBounds() const;
		const Sphere &GetLocalRenderSphere() const;
		
		const bounding_volume::AABB &GetAbsoluteRenderBounds() const;
		const Sphere &GetAbsoluteRenderSphere() const;

		// Note: These mustn't be called during rendering!
		const bounding_volume::AABB &GetUpdatedAbsoluteRenderBounds() const;
		const Sphere &GetUpdatedAbsoluteRenderSphere() const;

		bounding_volume::AABB CalcAbsoluteRenderBounds() const;
		Sphere CalcAbsoluteRenderSphere() const;

		void SetRenderMode(RenderMode mode);
		RenderMode GetRenderMode() const;

		Mat4 &GetTransformationMatrix();
		const umath::ScaledTransform &GetRenderPose() const;

		virtual void ReceiveData(NetPacket &packet) override;

		// Note: Called in render thread
		void UpdateRenderDataMT(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const CSceneComponent &scene,const CCameraComponent &cam,const Mat4 &vp);

		void UpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,bool bForceBufferUpdate=false);

		bool ShouldDraw() const;
		bool ShouldDrawShadow() const;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void OnEntitySpawn() override;

		CModelComponent *GetModelComponent() const;
		CAttachableComponent *GetAttachableComponent() const;
		CAnimatedComponent *GetAnimatedComponent() const;
		CLightMapReceiverComponent *GetLightMapReceiverComponent() const;

		void SetExemptFromOcclusionCulling(bool exempt);
		bool IsExemptFromOcclusionCulling() const;

		void GetDepthBias(float &outConstantFactor,float &outBiasClamp,float &outSlopeFactor) const;
		void SetDepthBias(float constantFactor,float biasClamp,float slopeFactor);
		StateFlags GetStateFlags() const;

		void SetDepthPassEnabled(bool enabled);
		bool IsDepthPassEnabled() const;

		void SetRenderClipPlane(const Vector4 &plane);
		void ClearRenderClipPlane();
		const Vector4 *GetRenderClipPlane() const;

		void SetReceiveShadows(bool enabled);
		bool IsReceivingShadows() const;

		void SetRenderBufferDirty();
		void SetRenderBoundsDirty();
		std::optional<Intersection::LineMeshResult> CalcRayIntersection(const Vector3 &start,const Vector3 &dir,bool precise=false) const;

		bool IsInstantiable() const;
		void SetInstaniationEnabled(bool enabled);
		void UpdateInstantiability();

		void UpdateShouldDrawState();

		void SetRenderOffsetTransform(const umath::ScaledTransform &t);
		void ClearRenderOffsetTransform();
		const umath::ScaledTransform *GetRenderOffsetTransform() const;
	protected:
		void UpdateShouldDrawShadowState();
		void UpdateRenderBuffer() const;
		void UpdateMatrices();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		void ClearRenderObjects();
		static bool RenderCallback(RenderObject *o,CBaseEntity *ent,pragma::CCameraComponent *cam,pragma::ShaderTextured3DBase *shader,Material *mat);
		bool RenderCallback(RenderObject *o,pragma::CCameraComponent *cam,pragma::ShaderTextured3DBase *shader,Material *mat);
		void UpdateRenderMeshes();

		void ClearRenderBuffers();
		void InitializeRenderBuffers();
		void UpdateBoneBuffer();

		std::optional<umath::ScaledTransform> m_renderOffset {};
		umath::ScaledTransform m_renderPose {};
		Mat4 m_matTransformation = umat::identity();
		RenderMode m_renderMode = RenderMode::World;

		// Used for quick access to avoid having to do a lookup on the entity's components
		mutable CAttachableComponent *m_attachableComponent = nullptr;
		mutable CAnimatedComponent *m_animComponent = nullptr;
		mutable CLightMapReceiverComponent *m_lightMapReceiverComponent = nullptr;

		bounding_volume::AABB m_localRenderBounds {};
		Sphere m_localRenderSphere {};

		bounding_volume::AABB m_absoluteRenderBounds {};
		Sphere m_absoluteRenderSphere {};

		std::optional<Vector4> m_renderClipPlane {};

		StateFlags m_stateFlags = static_cast<StateFlags>(umath::to_integral(StateFlags::RenderBufferDirty) | umath::to_integral(StateFlags::EnableDepthPass) | umath::to_integral(StateFlags::RenderBoundsDirty) |
			 umath::to_integral(StateFlags::ShouldDraw) | umath::to_integral(StateFlags::ShouldDrawShadow));
		std::atomic<uint64_t> m_lastRender = 0ull;
		std::mutex m_renderDataMutex;
		std::unordered_map<unsigned int,RenderInstance*> m_renderInstances;
		std::unique_ptr<SortedRenderMeshContainer> m_renderMeshContainer = nullptr;
		static std::vector<CRenderComponent*> s_ocExemptEntities;
	private:
		void UpdateAbsoluteRenderBounds();
		void UpdateAbsoluteSphereRenderBounds();
		void UpdateAbsoluteAABBRenderBounds();
		struct
		{
			float constantFactor = 0.f;
			float biasClamp = 0.f;
			float slopeFactor = 0.f;
		} m_depthBias;
		std::shared_ptr<prosper::IBuffer> m_renderBuffer = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_renderDescSetGroup = nullptr;
	};

	// Events
	
	struct DLLCLIENT CEUpdateInstantiability
		: public ComponentEvent
	{
		CEUpdateInstantiability(bool &instantiable);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		bool &instantiable;
	};

	struct DLLCLIENT CEShouldDraw
		: public ComponentEvent
	{
		CEShouldDraw(bool &shouldDraw);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		bool &shouldDraw;
	};

	struct DLLCLIENT CEOnUpdateRenderMatrices
		: public ComponentEvent
	{
		CEOnUpdateRenderMatrices(umath::ScaledTransform &pose,Mat4 &transformation);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		umath::ScaledTransform &pose;
		Mat4 &transformation;
	};

	struct DLLCLIENT CEOnUpdateRenderData
		: public ComponentEvent
	{
		CEOnUpdateRenderData(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &commandBuffer);
		virtual void PushArguments(lua_State *l) override;
		std::shared_ptr<prosper::IPrimaryCommandBuffer> commandBuffer;
	};

	struct DLLCLIENT CEOnUpdateRenderBuffers
		: public ComponentEvent
	{
		CEOnUpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &commandBuffer);
		virtual void PushArguments(lua_State *l) override;
		std::shared_ptr<prosper::IPrimaryCommandBuffer> commandBuffer;
	};

	struct DLLCLIENT CEOnRenderBoundsChanged
		: public ComponentEvent
	{
		CEOnRenderBoundsChanged(const Vector3 &min,const Vector3 &max,const Sphere &sphere);
		virtual void PushArguments(lua_State *l) override;
		const Vector3 &min;
		const Vector3 &max;
		const Sphere &sphere;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CRenderComponent::StateFlags)

#endif
