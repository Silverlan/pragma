#if 0
#include "stdafx_client.h"
#include "pragma/physics/c_phys_visual_debugger.hpp"
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

static util::WeakHandle<prosper::Shader> s_shaderDebug = {};
static auto s_debugObjectCount = 0ull;
CPhysVisualDebugger::CPhysVisualDebugger()
{
	if(s_debugObjectCount++ == 0ull)
		s_shaderDebug = c_engine->GetShader("debug_vertex");
}

void CPhysVisualDebuggerObject::Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CCameraComponent &cam)
{
	if(s_shaderDebug.expired())
		return;
	// Combine client with shared debugger?
	auto &shader = static_cast<pragma::ShaderDebugVertexColor&>(*s_shaderDebug);
	if(shader.BeginDraw(drawCmd,pragma::ShaderDebug::Pipeline::Line) == false)
		return;
	auto vp = cam.GetProjectionMatrix() *cam.GetViewMatrix();
	auto m = umat::identity(); // TODO
	(*drawCmd)->record_set_line_width(2.f);
	shader.Draw(**lineBuffer,**colorBuffer,m_vertexCount,vp *m);
	shader.EndDraw();
}

void CPhysVisualDebuggerObject::UpdateBuffer()
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

////////////////

void CPhysVisualDebugger::Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CCameraComponent &cam)
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
			o->hObject = {};
			it = m_objects.erase(it);
			++numInvalid;
		}
		else
		{
			auto *physObj = o->hObject.Get();
			auto bDraw = o->constraint;
			if(bDraw == false)
			{
				auto *colObj = static_cast<pragma::physics::ICollisionObject*>(physObj);
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

void CPhysVisualDebugger::Draw(VisualDebugObject *o)
{
	o->lines.clear();
	o->colors.clear();
	if(!o->hObject.IsValid())
		return;
	if(o->constraint == true)
	{
		auto *constraint = static_cast<pragma::physics::IConstraint*>(o->hObject.Get());
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
	auto *object = static_cast<pragma::physics::ICollisionObject*>(o->hObject.Get());
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
		auto *rigid = object->GetRigidBody();
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

void CPhysVisualDebugger::Draw(pragma::physics::IBase *b)
{
	std::unique_ptr<VisualDebugObject> o = nullptr;
	if(b->IsConstraint())
		o = std::make_unique<VisualDebugObject>(static_cast<pragma::physics::IConstraint*>(b));
	else
	{
		auto *co = dynamic_cast<pragma::physics::ICollisionObject*>(b);
		if(co == nullptr)
			return;
		o = std::make_unique<VisualDebugObject>(co);
	}
	Draw(o.get());
	m_objects.push_back(std::move(o));
}
#endif
