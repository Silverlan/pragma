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
#include <sharedutils/property/util_property.hpp>
#include <pragma/math/sphere.h>
#include <pragma/entities/components/base_render_component.hpp>
#include <mathutil/uvec.h>

namespace prosper {class IUniformResizableBuffer; class IDescriptorSet;};
namespace Intersection {struct LineMeshResult;};
namespace pragma
{
	class CModelComponent;
	class CAnimatedComponent;
	class CLightMapReceiverComponent;
	using RenderMeshIndex = uint32_t;
	class DLLCLIENT CRenderComponent final
		: public BaseRenderComponent,
		public CBaseNetComponent
	{
	public:
		enum class StateFlags : uint8_t
		{
			None = 0u,
			RenderBufferDirty = 1u,
			ExemptFromOcclusionCulling = RenderBufferDirty<<1u,
			HasDepthBias = ExemptFromOcclusionCulling<<1u,
			EnableDepthPass = HasDepthBias<<1u,
			DisableShadows = EnableDepthPass<<1u
		};
		static constexpr auto USE_HOST_MEMORY_FOR_RENDER_DATA = true;

		static ComponentEventId EVENT_ON_UPDATE_RENDER_DATA_MT;
		static ComponentEventId EVENT_ON_RENDER_BUFFERS_INITIALIZED;
		static ComponentEventId EVENT_ON_RENDER_BOUNDS_CHANGED;
		static ComponentEventId EVENT_SHOULD_DRAW;
		static ComponentEventId EVENT_SHOULD_DRAW_SHADOW;
		static ComponentEventId EVENT_ON_UPDATE_RENDER_BUFFERS;
		static ComponentEventId EVENT_ON_UPDATE_RENDER_MATRICES;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		CRenderComponent(BaseEntity &ent);
		std::weak_ptr<prosper::IBuffer> GetRenderBuffer() const;
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

		unsigned long long &GetLastRenderFrame();
		void SetLastRenderFrame(unsigned long long &t);

		void GetAbsoluteRenderBounds(Vector3 &outMin,Vector3 &outMax) const;
		void GetRenderBounds(Vector3 *min,Vector3 *max) const;
		void SetRenderBounds(Vector3 min,Vector3 max);
		Sphere GetRenderSphereBounds() const;
		void GetRotatedRenderBounds(Vector3 *min,Vector3 *max);

		void SetRenderMode(RenderMode mode);
		RenderMode GetRenderMode() const;
		const util::PEnumProperty<RenderMode> &GetRenderModeProperty() const;

		Mat4 &GetTransformationMatrix();
		const umath::ScaledTransform &GetRenderPose() const;

		virtual void ReceiveData(NetPacket &packet) override;

		// Note: Called in render thread
		void UpdateRenderDataMT(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const CSceneComponent &scene,const CCameraComponent &cam,const Mat4 &vp);

		void UpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,bool bForceBufferUpdate=false);

		bool ShouldDraw(const Vector3 &camOrigin) const;
		bool ShouldDrawShadow(const Vector3 &camOrigin) const;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void OnEntitySpawn() override;
		void UpdateRenderBounds();

		util::WeakHandle<CModelComponent> &GetModelComponent() const;
		util::WeakHandle<CAnimatedComponent> &GetAnimatedComponent() const;
		util::WeakHandle<CLightMapReceiverComponent> &GetLightMapReceiverComponent() const;

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
		std::optional<Intersection::LineMeshResult> CalcRayIntersection(const Vector3 &start,const Vector3 &dir,bool precise=false) const;

		void SetRenderOffsetTransform(const umath::ScaledTransform &t);
		void ClearRenderOffsetTransform();
		const umath::ScaledTransform *GetRenderOffsetTransform() const;
	protected:
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
		util::PEnumProperty<RenderMode> m_renderMode = nullptr;

		// Used for quick access to avoid having to do a lookup on the entity's components
		mutable util::WeakHandle<CModelComponent> m_mdlComponent = {};
		mutable util::WeakHandle<CAnimatedComponent> m_animComponent = {};
		mutable util::WeakHandle<CLightMapReceiverComponent> m_lightMapReceiverComponent = {};

		Vector3 m_renderMin = {};
		Vector3 m_renderMax = {};
		Vector3 m_renderMinRot = {};
		Vector3 m_renderMaxRot = {};
		Sphere m_renderSphere = {};

		std::optional<Vector4> m_renderClipPlane {};

		StateFlags m_stateFlags = static_cast<StateFlags>(umath::to_integral(StateFlags::RenderBufferDirty) | umath::to_integral(StateFlags::EnableDepthPass));
		unsigned long long m_lastRender = 0ull;
		std::unordered_map<unsigned int,RenderInstance*> m_renderInstances;
		std::unique_ptr<SortedRenderMeshContainer> m_renderMeshContainer = nullptr;
		static std::vector<CRenderComponent*> s_ocExemptEntities;
	private:
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

	struct DLLCLIENT CEShouldDraw
		: public ComponentEvent
	{
		enum class ShouldDraw : uint8_t
		{
			Undefined = 0u,
			Yes,
			No
		};
		CEShouldDraw(const Vector3 &camOrigin);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		const Vector3 &camOrigin;
		ShouldDraw shouldDraw = ShouldDraw::Undefined;
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
