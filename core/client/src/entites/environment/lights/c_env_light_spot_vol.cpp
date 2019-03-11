#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_light_spot_vol.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/physics/raytraces.h>
#include <pragma/model/model.h>
#include <pragma/math/intersection.h>

using namespace pragma;

#pragma optimize("",off)
LINK_ENTITY_TO_CLASS(env_light_spot_vol,CEnvLightSpotVol);

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

void CLightSpotVolComponent::Initialize()
{
	BaseEnvLightSpotVolComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent<CModelComponent>();
	auto pRenderComponent = ent.AddComponent<CRenderComponent>();
	if(pRenderComponent.valid())
	{
		FlagCallbackForRemoval(pRenderComponent->BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
			c_game->SetFrameDepthBufferSamplingRequired();
		}),CallbackType::Entity);
	}
}

void CLightSpotVolComponent::ReceiveData(NetPacket &packet)
{
	m_coneAngle = packet->Read<float>();
}

util::EventReply CLightSpotVolComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEnvLightSpotVolComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseToggleComponent::EVENT_ON_TURN_ON)
	{
		auto pRenderComponent = static_cast<CBaseEntity&>(GetEntity()).GetRenderComponent();
		if(pRenderComponent.valid())
			pRenderComponent->SetRenderMode(RenderMode::World);
	}
	else if(eventId == BaseToggleComponent::EVENT_ON_TURN_OFF)
	{
		auto pRenderComponent = static_cast<CBaseEntity&>(GetEntity()).GetRenderComponent();
		if(pRenderComponent.valid())
			pRenderComponent->SetRenderMode(RenderMode::None);
	}
	return util::EventReply::Unhandled;
}

void CLightSpotVolComponent::InitializeVolumetricLight()
{
	auto mdlComponent = GetEntity().GetModelComponent();
	if(mdlComponent.expired())
		return;
	auto mdl = c_game->CreateModel();
	auto group = mdl->AddMeshGroup("reference");

	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto maxDist = pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 100.f;
	auto endRadius = maxDist *umath::tan(umath::deg_to_rad(m_coneAngle));

	auto *mat = static_cast<CMaterial*>(client->CreateMaterial("lightcone","light_cone"));
	auto &data = mat->GetDataBlock();
	data->AddValue("bool","translucent","1");
	data->AddValue("float","cone_height",std::to_string(maxDist));
	mat->SetTexture("diffusemap","error");

	const uint32_t coneDetail = 64;
	const uint32_t segmentCount = 20;
	const Vector3 dir {0.f,0.f,1.f};
	auto mesh = std::make_shared<CModelMesh>();
	for(auto i=decltype(segmentCount){0};i<segmentCount;++i)
	{
		auto startSc = i /static_cast<float>(segmentCount);
		auto endSc = (i +1) /static_cast<float>(segmentCount);

		auto startPos = dir *maxDist *startSc;
		auto endPos = dir *maxDist *endSc;
		auto segStartRadius = endRadius *startSc;
		auto segEndRadius = endRadius *endSc;
		auto subMesh = std::make_shared<CModelSubMesh>();
		std::vector<Vector3> verts;
		std::vector<Vector3> normals;
		Geometry::GenerateTruncatedConeMesh(startPos,static_cast<float>(segStartRadius),dir,uvec::distance(startPos,endPos),static_cast<float>(segEndRadius),verts,&subMesh->GetTriangles(),&normals,coneDetail,false);

		auto &meshVerts = subMesh->GetVertices();
		meshVerts.reserve(verts.size());
		for(auto idx=decltype(verts.size()){0};idx<verts.size();++idx)
			meshVerts.push_back(Vertex{verts[idx],normals[idx]});

		subMesh->SetTexture(0);
		mesh->AddSubMesh(subMesh);
	}
	group->AddMesh(mesh);
	mdl->AddMaterial(0,mat);
	mdl->Update(ModelUpdateFlags::All);

	mdlComponent->SetModel(mdl);
}

void CLightSpotVolComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	InitializeVolumetricLight();
}

luabind::object CLightSpotVolComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CLightSpotVolComponentHandleWrapper>(l);}

///////

void CEnvLightSpotVol::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CLightSpotVolComponent>();
}
#pragma optimize("",on)
