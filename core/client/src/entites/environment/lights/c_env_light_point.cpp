#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_light_point.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/lighting/c_light_point.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/rendering/c_cubemapside.h"
#include "pragma/rendering/lighting/shadows/c_shadowmapcube.h"
#include <pragma/math/intersection.h>
#include <pragma/entities/components/base_transform_component.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_light_point,CEnvLightPoint);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

static const std::array<Vector3,6> directions = {
	Vector3(1.f,0.f,0.f),
	Vector3(-1.f,0.f,0.f),
	Vector3(0.f,1.f,0.f),
	Vector3(0.f,-1.f,0.f),
	Vector3(0.f,0.f,1.f),
	Vector3(0.f,0.f,-1.f)
};

void CLightPointComponent::Initialize()
{
	BaseEnvLightPointComponent::Initialize();
	BindEvent(CLightComponent::EVENT_SHOULD_PASS_ENTITY,[this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &shouldPassData = static_cast<CEShouldPassEntity&>(evData.get());
		auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
		if(pLightComponent.expired() || pLightComponent->IsInRange(shouldPassData.entity) == false)
		{
			shouldPassData.shouldPass = false;
			return util::EventReply::Handled;
		}
		const auto ang = 0.7853982f; // 45 Degree

		for(auto i=decltype(directions.size()){0};i<directions.size();++i)
		{
			auto &dir = directions[i];
			auto &ent = shouldPassData.entity;
			auto pRenderComponent = ent.GetRenderComponent();
			auto pTrComponent = ent.GetTransformComponent();
			auto pTrComponentThis = GetEntity().GetTransformComponent();
			if(pRenderComponent.expired() || pTrComponent.expired() || pTrComponentThis.expired())
			{
				shouldPassData.shouldPass = false;
				return util::EventReply::Handled;
			}
			auto &start = pTrComponentThis->GetPosition();
			auto sphere = pRenderComponent->GetRenderSphereBounds();

			if(Intersection::SphereInPlaneMesh(pTrComponent->GetPosition() +sphere.pos -this->GetEntity().GetPosition(),sphere.radius,m_frustumPlanes.at(i),true) != INTERSECT_OUTSIDE)
				shouldPassData.renderFlags |= 1<<i;
			//if(pLightComponent->IsInCone(shouldPassData.entity,dir,ang) == true)
			//	shouldPassData.renderFlags |= 1<<i;
		}
		shouldPassData.shouldPass = (shouldPassData.renderFlags != 0) ? true : false;
		return util::EventReply::Handled;
	});
	BindEvent(CLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH,[this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &shouldPassData = static_cast<CEShouldPassEntityMesh&>(evData.get());
		auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
		shouldPassData.shouldPass = pLightComponent.valid() && pLightComponent->IsInRange(shouldPassData.entity,shouldPassData.mesh);
		return util::EventReply::Handled;
	});
	BindEvent(CLightComponent::EVENT_GET_TRANSFORMATION_MATRIX,[this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &trData = static_cast<CEGetTransformationMatrix&>(evData.get());
		trData.transformation = &MVPBias<6>::GetTransformationMatrix(trData.index);
		return util::EventReply::Handled;
	});
	BindEvent(CLightComponent::EVENT_HANDLE_SHADOW_MAP,[this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		static_cast<CEHandleShadowMap&>(evData.get()).shadowMap = std::make_unique<ShadowMapCube>();
		return util::EventReply::Handled;
	});
	BindEventUnhandled(CRadiusComponent::EVENT_ON_RADIUS_CHANGED,[this](std::reference_wrapper<ComponentEvent> evData) {
		UpdateFrustumPlanes();
	});

	m_bSkipMatrixUpdate = true;
	UpdateProjectionMatrix();
	m_bSkipMatrixUpdate = false;
	//SetProjectionMatrix(glm::perspective<float>(35.f *2.f,1.f,2.f,m_distance));
	//SetViewMatrix(glm::lookAt(GetPosition(),GetPosition() +Vector3(1,0,0),Vector3(0,1,0)));
	for(auto i=0;i<6;i++)
		UpdateTransformationMatrix(i);

	auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
	if(pLightComponent.valid())
		pLightComponent->UpdateTransformationMatrix(GetBiasTransformationMatrix(),GetViewMatrix(),GetProjectionMatrix());
	UpdateFrustumPlanes();
}
void CLightPointComponent::UpdateFrustumPlanes()
{
	// Note: Up direction doesn't matter, as long as it's axis-aligned
	// and not the same as the respective directions from the 'directions' array
	static const std::array<Vector3,6> upDirs = {
		Vector3(0.f,1.f,0.f),
		Vector3(0.f,1.f,0.f),
		Vector3(0.f,0.f,1.f),
		Vector3(0.f,0.f,1.f),
		Vector3(0.f,1.f,0.f),
		Vector3(0.f,1.f,0.f)
	};
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto radius = pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f;
	for(auto i=decltype(directions.size()){0};i<directions.size();++i)
	{
		auto cam = Camera::Create(
			90.f /* fov */,90.f /* viewFov */,1.f /* aspectRatio */,
			2.f /* nearZ */,radius /* farZ */
		);
		cam->SetForward(directions.at(i));
		cam->SetUp(upDirs.at(i));
		cam->UpdateViewMatrix();
		cam->UpdateFrustumPlanes();
		m_frustumPlanes.at(i) = cam->GetFrustumPlanes();
	}
}
void CLightPointComponent::UpdateProjectionMatrix()
{
	auto scale = Vector3(-1.f,-1.f,1.f); // Vulkan TODO
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto p = glm::perspectiveRH<float>(CFloat(umath::deg_to_rad(90.f)),1.f,2.f,pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f); // Vulkan TODO
	p = glm::scale(p,scale);
	SetProjectionMatrix(p);
	UpdateViewMatrices();
}
void CLightPointComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEnvLightPointComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CRadiusComponent))
	{
		static_cast<CRadiusComponent&>(component).GetRadiusProperty()->AddCallback([this](std::reference_wrapper<const float> oldRadius,std::reference_wrapper<const float> radius) {
			UpdateProjectionMatrix();
		});
	}
	else if(typeid(component) == typeid(CTransformComponent))
	{
		FlagCallbackForRemoval(static_cast<CTransformComponent&>(component).GetPosProperty()->AddCallback([this](std::reference_wrapper<const Vector3> oldPos,std::reference_wrapper<const Vector3> pos) {
			UpdateViewMatrices();
		}),CallbackType::Component,&component);
	}
	else if(typeid(component) == typeid(CLightComponent))
		static_cast<CLightComponent&>(component).SetLight(*this);
}
luabind::object CLightPointComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CLightPointComponentHandleWrapper>(l);}

/////////////

void CEnvLightPoint::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CLightComponent>();
	AddComponent<CLightPointComponent>();
}

/////////////

void CLightPointComponent::UpdateViewMatrices()
{
	auto b = m_bSkipMatrixUpdate;
	m_bSkipMatrixUpdate = true;
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	SetViewMatrix(glm::lookAtRH(pos,pos +directions[umath::to_integral(CubeMapSide::Left)],Vector3(0,1,0)),umath::to_integral(CubeMapSide::Left));//umat::look_at(pos,pos +Vector3(1,0,0),Vector3(0,1,0)),1); // Vulkan TODO
	SetViewMatrix(glm::lookAtRH(pos,pos +directions[umath::to_integral(CubeMapSide::Right)],Vector3(0,1,0)),umath::to_integral(CubeMapSide::Right));
	SetViewMatrix(glm::lookAtRH(pos,pos +directions[umath::to_integral(CubeMapSide::Top)],Vector3(0,0,-1)),umath::to_integral(CubeMapSide::Top));
	SetViewMatrix(glm::lookAtRH(pos,pos +directions[umath::to_integral(CubeMapSide::Bottom)],Vector3(0,0,1)),umath::to_integral(CubeMapSide::Bottom));
	SetViewMatrix(glm::lookAtRH(pos,pos +directions[umath::to_integral(CubeMapSide::Front)],Vector3(0,1,0)),umath::to_integral(CubeMapSide::Front));
	SetViewMatrix(glm::lookAtRH(pos,pos +directions[umath::to_integral(CubeMapSide::Back)],Vector3(0,1,0)),umath::to_integral(CubeMapSide::Back));
	m_bSkipMatrixUpdate = b;
	if(m_bSkipMatrixUpdate == false)
	{
		for(auto i=0;i<6;i++)
			UpdateTransformationMatrix(i);
	}
}

void CLightPointComponent::UpdateTransformationMatrix(unsigned int j) // TODO This is called every time one of the matrices is changed; Only needs to be called once, after they're ALL changed!
{
	if(m_bSkipMatrixUpdate == true)
		return;
	MVPBias<6>::UpdateTransformationMatrix(j);
	auto pLightComponent = GetEntity().GetComponent<CLightComponent>();
	if(pLightComponent.expired())
		return;
	auto &shadowBuffer = pLightComponent->GetShadowBuffer();
	if(shadowBuffer == nullptr)
		return;
	std::array<Mat4,3> matrices = {GetBiasTransformationMatrix(),GetViewMatrix(4),GetProjectionMatrix()};
	c_engine->ScheduleRecordUpdateBuffer(shadowBuffer,0ull,matrices);
}
