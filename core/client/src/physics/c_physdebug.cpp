#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/physics/c_physdebug.h"
#include "pragma/debug/c_debugoverlay.h"
#include <pragma/console/conout.h>
#include "pragma/rendering/shaders/debug/c_shader_debug.h"
#include <pragma/physics/physobj.h>
#include "pragma/entities/point/constraints/point_constraint_base.h"
#include <pragma/physics/physenvironment.h>
#include <pragma/physics/physconstraint.h>
#include "pragma/physics/physcollisionobject.h"
#include "pragma/physics/physcontroller.h"
#include <pragma/physics/physshape.h>
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/rendering/shaders/debug/c_shader_debug.hpp"
#include <pragma/entities/components/base_transform_component.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

static util::WeakHandle<prosper::Shader> s_shaderDebug = {};
static auto s_debugObjectCount = 0ull;
WVBtIDebugObject::WVBtIDebugObject()
{
	if(s_debugObjectCount++ == 0ull)
		s_shaderDebug = c_engine->GetShader("debug_vertex");
}
WVBtIDebugObject::WVBtIDebugObject(PhysCollisionObject *o)
	: WVBtIDebugObject()
{
	hObject = o->GetBaseHandle();
	auto *co = o->GetCollisionObject();
	auto flags = co->getCollisionFlags();
	if((flags &btCollisionObject::CollisionFlags::CF_STATIC_OBJECT) == 0)
		dynamic = true;
}
WVBtIDebugObject::WVBtIDebugObject(PhysConstraint *c)
	: WVBtIDebugObject()
{
	dynamic = true;
	constraint = true;
	hObject = c->GetBaseHandle();
}

WVBtIDebugObject::~WVBtIDebugObject()
{
	if(--s_debugObjectCount == 0ull)
		s_shaderDebug = {};
}

void WVBtIDebugObject::Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CCameraComponent &cam)
{
	if(s_shaderDebug.expired())
		return;
	auto &shader = static_cast<pragma::ShaderDebugVertexColor&>(*s_shaderDebug);
	if(shader.BeginDraw(drawCmd,pragma::ShaderDebug::Pipeline::Line) == false)
		return;
	auto vp = cam.GetProjectionMatrix() *cam.GetViewMatrix();
	auto m = umat::identity(); // TODO
	(*drawCmd)->record_set_line_width(2.f);
	shader.Draw(**lineBuffer,**colorBuffer,m_vertexCount,vp *m);
	shader.EndDraw();
}

void WVBtIDebugObject::UpdateBuffer()
{
	if(lines.empty() || s_shaderDebug.expired())
		return;
	auto &dev = c_engine->GetDevice();
	m_vertexCount = static_cast<uint32_t>(lines.size());
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.size = m_vertexCount *sizeof(Vector3);
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::VERTEX_BUFFER_BIT;
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
	if(colorBuffer != nullptr)
		c_engine->KeepResourceAliveUntilPresentationComplete(lineBuffer);
	lineBuffer = prosper::util::create_buffer(dev,createInfo,lines.data());

	createInfo.size = colors.size() *sizeof(Vector4);
	if(colorBuffer != nullptr)
		c_engine->KeepResourceAliveUntilPresentationComplete(colorBuffer);
	colorBuffer = prosper::util::create_buffer(dev,createInfo,colors.data());

	// Obsolete?
	/*m_descSetGroup = nullptr;
	Vector4 color{0.f,0.f,0.f,1.f};
	bool bUseVertexColor = true;
	Vulkan::Std140LayoutBlockData data(5);
	data<<color<<bUseVertexColor;
	m_instanceBuffer = Vulkan::Buffer::Create(context,Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT,data.GetSize(),data.GetData());
	auto *shaderDebug = hShader.get<Shader::Debug>();
	shaderDebug->GenerateDescriptorSet(static_cast<uint32_t>(Shader::Debug::DescSet::Instance),m_descSet);
	m_descSet->Update(m_instanceBuffer);*/
}

/////////////////////////////////////////////////

WVBtIDebugDraw::WVBtIDebugDraw()
	: btIDebugDraw(),m_debugMode(0),m_drawObject(NULL),
	m_cbCollisionObject(),m_cbConstraint(),m_cbController()
{}
WVBtIDebugDraw::~WVBtIDebugDraw()
{
	Clear();
}
void WVBtIDebugDraw::drawLine(const btVector3 &from,const btVector3 &to,const btVector3 &color)
{
	drawLine(from,to,color,color);
}
void WVBtIDebugDraw::drawLine(const btVector3 &from,const btVector3 &to,const btVector3 &fromColor,const btVector3 &toColor)
{
	if(m_drawObject == nullptr)
	{
		DebugRenderer::DrawLine(
			Vector3(from.x() /PhysEnv::WORLD_SCALE,from.y() /PhysEnv::WORLD_SCALE,from.z() /PhysEnv::WORLD_SCALE),
			Vector3(to.x() /PhysEnv::WORLD_SCALE,to.y() /PhysEnv::WORLD_SCALE,to.z() /PhysEnv::WORLD_SCALE),
			Color(static_cast<int16_t>(fromColor.x() *255.f),static_cast<int16_t>(fromColor.y() *255.f),static_cast<int16_t>(fromColor.z() *255.f)),0.00001f
		);
		return;
	}
	m_drawObject->lines.push_back(Vector3(from.x() /PhysEnv::WORLD_SCALE,from.y() /PhysEnv::WORLD_SCALE,from.z() /PhysEnv::WORLD_SCALE));
	m_drawObject->lines.push_back(Vector3(to.x() /PhysEnv::WORLD_SCALE,to.y() /PhysEnv::WORLD_SCALE,to.z() /PhysEnv::WORLD_SCALE));

	m_drawObject->colors.push_back(Vector4(fromColor.x(),fromColor.y(),fromColor.z(),1.f));
	m_drawObject->colors.push_back(Vector4(toColor.x(),toColor.y(),toColor.z(),1.f));
}
void WVBtIDebugDraw::drawContactPoint(const btVector3&,const btVector3&,btScalar,int,const btVector3&) {}
void WVBtIDebugDraw::reportErrorWarning(const char *warningString)
{
	Con::cwar<<"[BULLET] "<<warningString<<Con::endl;
};
void WVBtIDebugDraw::draw3dText(const btVector3&,const char*)
{

}
int WVBtIDebugDraw::getDebugMode() const {return m_debugMode;}

void WVBtIDebugDraw::Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CCameraComponent &cam)
{
	auto maxDist = 1'000.f *static_cast<float>(PhysEnv::WORLD_SCALE);
	Vector3 posPl {};
	auto *pl = c_game->GetLocalPlayer();
	if(pl != nullptr)
	{
		auto &ent = pl->GetEntity();
		auto charComponent = ent.GetCharacterComponent();
		auto trComponent = ent.GetTransformComponent();
		if(charComponent.valid() || trComponent.valid())
			posPl = (charComponent.valid() ? charComponent->GetEyePosition() : trComponent->GetPosition()) *static_cast<float>(PhysEnv::WORLD_SCALE);
	}
	uint32_t numRendered = 0;
	uint32_t numInvalid = 0;
	uint32_t numInvalidShape = 0;
	for(auto it=m_objects.begin();it!=m_objects.end();)
	{
		auto &o = *it;
		if(!o->hObject.IsValid())
		{
			o->hObject = PhysHandle();
			it = m_objects.erase(it);
			++numInvalid;
		}
		else
		{
			auto *physObj = o->hObject.get();
			auto bDraw = o->constraint;
			if(bDraw == false)
			{
				auto *colObj = static_cast<PhysCollisionObject*>(physObj);
				auto shape = colObj->GetCollisionShape();
				if(shape != nullptr)
				{
					auto &colOrigin = colObj->GetOrigin();
					auto btOrigin = btVector3(colOrigin.x,colOrigin.y,colOrigin.z) *PhysEnv::WORLD_SCALE;
					auto *btObj = colObj->GetCollisionObject();
					auto *btShape = shape->GetShape();
					btVector3 origin;
					btScalar radius;
					btShape->getBoundingSphere(origin,radius);
					origin += btObj->getWorldTransform().getOrigin() +btOrigin;
					bDraw = (uvec::length(Vector3(origin.x(),origin.y(),origin.z()) -posPl) <= (maxDist +radius)) ? true : false;
				}
				else
					++numInvalidShape;
			}
			if(bDraw == true)
			{
				if(o->dynamic == true)
					Draw(o.get());
				o->Render(drawCmd,cam);
				++numRendered;
			}
			++it;
		}
	}
}

void WVBtIDebugDraw::Draw(WVBtIDebugObject *o)
{
	o->lines.clear();
	o->colors.clear();
	if(!o->hObject.IsValid())
		return;
	if(o->constraint == true)
	{
		auto *constraint = static_cast<PhysConstraint*>(o->hObject.get());
		auto *c = constraint->GetConstraint();
		if(c == nullptr)
			return;
		m_drawObject = o;

			auto *physEnv = c_game->GetPhysicsEnvironment();
			auto *world = physEnv->GetWorld();
			world->debugDrawConstraint(c);

		m_drawObject->UpdateBuffer();
		m_drawObject = nullptr;
		return;
	}
	auto *object = static_cast<PhysCollisionObject*>(o->hObject.get());
	auto shape = object->GetCollisionShape();
	if(shape == nullptr)
		return;
	auto t = object->GetWorldTransform();
	m_drawObject = o;
	
		auto *physEnv = c_game->GetPhysicsEnvironment();
		auto *world = physEnv->GetWorld();
		btVector3 col(1.f,1.f,1.f);
		if(object->IsRigid())
		{
			auto *rigid = static_cast<PhysRigidBody*>(object);
			if(rigid->GetMass() == 0.f)
				col = btVector3(0.f,0.6f,1.f);
			else
				col = btVector3(0.6f,1.f,0.f);
		}
		else if(object->IsGhost())
			col = btVector3(1.f,0.5f,0.f);
		auto origin = t.GetOrigin();
		//origin -= object->GetOrigin() *uquat::get_inverse(t.GetRotation());
		t.SetOrigin(origin);
		world->debugDrawObject(t.GetTransform(),shape->GetShape(),col);

	m_drawObject->UpdateBuffer();
	m_drawObject = nullptr;
}

void WVBtIDebugDraw::Draw(PhysBase *b)
{
	auto *constraint = dynamic_cast<PhysConstraint*>(b);
	std::unique_ptr<WVBtIDebugObject> o = nullptr;
	if(constraint != nullptr)
		o = std::make_unique<WVBtIDebugObject>(constraint);
	else
	{
		auto *co = dynamic_cast<PhysCollisionObject*>(b);
		if(co == nullptr)
			return;
		o = std::make_unique<WVBtIDebugObject>(co);
	}
	Draw(o.get());
	m_objects.push_back(std::move(o));
}

void WVBtIDebugDraw::Draw()
{
	auto *physEnv = c_game->GetPhysicsEnvironment();

	m_cbCollisionObject = FunctionCallback<void,PhysCollisionObject*>::Create([this](PhysCollisionObject *obj) {
		Draw(obj);
	});
	physEnv->AddEventCallback(PhysEnv::EVENT_COLLISION_OBJECT_CREATE,m_cbCollisionObject);
	m_cbConstraint = FunctionCallback<void,PhysConstraint*>::Create([this](PhysConstraint *constraint) {
		Draw(constraint);
	});
	physEnv->AddEventCallback(PhysEnv::EVENT_CONSTRAINT_CREATE,m_cbConstraint);
	m_cbController = FunctionCallback<void,PhysController*>::Create([this](PhysController *controller) {
		auto *ghost = controller->GetGhostObject();
		Draw(ghost);
	});

	auto &objs = physEnv->GetCollisionObjects();
	for(auto *o : objs)
		Draw(o);
	auto &controllers = physEnv->GetControllers();
	for(auto *controller : controllers)
	{
		auto *ghost = controller->GetGhostObject();
		Draw(ghost);
	}
}
void WVBtIDebugDraw::setDebugMode(int debugMode)
{
	Clear();
	m_debugMode = debugMode;
	if(m_debugMode == btIDebugDraw::DBG_NoDebug)
		return;
	Draw();
}

void WVBtIDebugDraw::Clear()
{
	m_objects.clear();
	if(m_cbCollisionObject.IsValid())
		m_cbCollisionObject.Remove();
	if(m_cbConstraint.IsValid())
		m_cbConstraint.Remove();
	if(m_cbController.IsValid())
		m_cbController.Remove();
}
