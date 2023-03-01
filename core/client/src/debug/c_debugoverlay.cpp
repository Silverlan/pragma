/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/rendering/shaders/debug/c_shader_debug.hpp"
#include "pragma/rendering/shaders/debug/c_shader_debug_text.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/math/icosphere.h"
#include <pragma/math/util_hermite.h>
#include <pragma/math/e_frustum.h>
#include <wgui/types/witext.h>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/math/intersection.h>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

namespace DebugRenderer {
	struct RuntimeObject {
		RuntimeObject(const std::shared_ptr<DebugRenderer::BaseObject> &o, float duration);
		std::shared_ptr<DebugRenderer::BaseObject> obj;
		double time;
	};
	RuntimeObject::RuntimeObject(const std::shared_ptr<DebugRenderer::BaseObject> &o, float duration) : obj(o), time(client->RealTime() + duration) {}
}

static constexpr uint32_t s_maxDebugObjectCount = 100'000;
static std::unordered_map<DebugRenderer::Type, std::vector<DebugRenderer::RuntimeObject>> s_debugObjects {{DebugRenderer::Type::Triangles, {}}, {DebugRenderer::Type::Lines, {}}, {DebugRenderer::Type::LinesStrip, {}}, {DebugRenderer::Type::Points, {}},
  {DebugRenderer::Type::PointsVertex, {}}, {DebugRenderer::Type::Other, {}}};

static bool is_obj_valid(DebugRenderer::RuntimeObject &o, double t) { return o.obj != nullptr && o.obj->IsValid() && (t <= o.time || o.obj.use_count() != 1); }

static void cleanup()
{
	// Clean up old debug objects once in a while
	static uint32_t g_lastCleanupN = 0;
	if(++g_lastCleanupN < 1'000)
		return;
	g_lastCleanupN = 0;
	auto &t = client->RealTime();
	for(auto &pair : s_debugObjects) {
		auto &objs = pair.second;
		size_t i = 0;
		for(auto it = objs.begin(); it != objs.end();) {
			if(is_obj_valid(*it, t) && i < s_maxDebugObjectCount) {
				++it;
				++i;
				continue;
			}
			it = objs.erase(it);
		}
	}
}

DebugRenderer::BaseObject::BaseObject() : m_bValid(true) {}
bool DebugRenderer::BaseObject::IsValid() const { return m_bValid; }
void DebugRenderer::BaseObject::Remove() { m_bValid = false; }
const umath::ScaledTransform &DebugRenderer::BaseObject::GetPose() const { return const_cast<BaseObject *>(this)->GetPose(); }
umath::ScaledTransform &DebugRenderer::BaseObject::GetPose() { return m_pose; }
void DebugRenderer::BaseObject::SetPose(const umath::ScaledTransform &pose) { m_pose = pose; }
const Vector3 &DebugRenderer::BaseObject::GetPos() const { return m_pose.GetOrigin(); }
void DebugRenderer::BaseObject::SetPos(const Vector3 &pos)
{
	m_pose.SetOrigin(pos);
	UpdateModelMatrix();
}
const Quat &DebugRenderer::BaseObject::GetRotation() const { return m_pose.GetRotation(); }
void DebugRenderer::BaseObject::SetRotation(const Quat &rot)
{
	m_pose.SetRotation(rot);
	UpdateModelMatrix();
}
EulerAngles DebugRenderer::BaseObject::GetAngles() const { return EulerAngles(m_pose.GetRotation()); }
void DebugRenderer::BaseObject::SetAngles(const EulerAngles &ang) { SetRotation(uquat::create(ang)); }
const Vector3 &DebugRenderer::BaseObject::GetScale() const { return m_pose.GetScale(); }
void DebugRenderer::BaseObject::SetScale(const Vector3 &scale)
{
	m_pose.SetScale(scale);
	UpdateModelMatrix();
}
const Mat4 &DebugRenderer::BaseObject::GetModelMatrix() const { return m_modelMatrix; }
bool DebugRenderer::BaseObject::IsVisible() const { return m_bVisible; }
void DebugRenderer::BaseObject::SetVisible(bool b) { m_bVisible = b; }
void DebugRenderer::BaseObject::UpdateModelMatrix() { m_modelMatrix = m_pose.::umath::Transform::ToMatrix() * glm::scale(glm::mat4 {1.f}, GetScale()); }

///////////////////////////

DebugRenderer::CollectionObject::CollectionObject(const std::vector<std::shared_ptr<BaseObject>> &objs) : BaseObject(), m_objects(objs) {}
void DebugRenderer::CollectionObject::AddObject(const std::shared_ptr<BaseObject> &o) { m_objects.push_back(o); }
void DebugRenderer::CollectionObject::SetPos(const Vector3 &pos)
{
	BaseObject::SetPos(pos);
	for(auto &o : m_objects)
		o->SetPos(pos);
}
void DebugRenderer::CollectionObject::SetRotation(const Quat &rot)
{
	BaseObject::SetRotation(rot);
	for(auto &o : m_objects)
		o->SetRotation(rot);
}
void DebugRenderer::CollectionObject::SetAngles(const EulerAngles &ang)
{
	BaseObject::SetAngles(ang);
	for(auto &o : m_objects)
		o->SetAngles(ang);
}
void DebugRenderer::CollectionObject::Remove()
{
	BaseObject::Remove();
	for(auto &o : m_objects)
		o->Remove();
}
void DebugRenderer::CollectionObject::SetScale(const Vector3 &scale)
{
	BaseObject::SetScale(scale);
	for(auto &o : m_objects)
		o->SetScale(scale);
}
DebugRenderer::ObjectType DebugRenderer::CollectionObject::GetType() const { return ObjectType::Collection; }

///////////////////////////

DebugRenderer::WorldObject::WorldObject() : BaseObject(), m_vertexBuffer(nullptr), m_colorBuffer(nullptr), m_color {0.f, 0.f, 0.f, 1.f} {}

DebugRenderer::WorldObject::WorldObject(const Vector4 &color) : BaseObject(), m_vertexBuffer(nullptr), m_colorBuffer(nullptr), m_color(color) {}
DebugRenderer::WorldObject::~WorldObject()
{
	if(m_vertexBuffer != nullptr)
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_vertexBuffer);
	if(m_colorBuffer != nullptr)
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_colorBuffer);
}
DebugRenderer::ObjectType DebugRenderer::WorldObject::GetType() const { return ObjectType::World; }
const Vector4 &DebugRenderer::WorldObject::GetColor() const { return m_color; }
const Vector4 &DebugRenderer::WorldObject::GetOutlineColor() const { return m_outlineColor; }
void DebugRenderer::WorldObject::SetColor(const Vector4 &col) { m_color = col; }
void DebugRenderer::WorldObject::SetOutlineColor(const Vector4 &outlineColor)
{
	m_bOutline = true;
	m_outlineColor = outlineColor;
}
bool DebugRenderer::WorldObject::HasOutline() const { return m_bOutline; }
const std::shared_ptr<prosper::IBuffer> &DebugRenderer::WorldObject::GetColorBuffer() const { return m_colorBuffer; }
const std::shared_ptr<prosper::IBuffer> &DebugRenderer::WorldObject::GetVertexBuffer() const { return m_vertexBuffer; }
uint32_t DebugRenderer::WorldObject::GetVertexCount() const { return (m_vertexCount != std::numeric_limits<uint32_t>::max()) ? m_vertexCount : m_vertices.size(); }
void DebugRenderer::WorldObject::AddVertex(const Vector3 &v) { m_vertices.push_back(v); }
std::vector<Vector3> &DebugRenderer::WorldObject::GetVertices() { return m_vertices; }
std::vector<Vector4> &DebugRenderer::WorldObject::GetColors() { return m_colors; }
bool DebugRenderer::WorldObject::InitializeBuffers(const std::shared_ptr<prosper::IBuffer> &vertexBuffer, uint32_t vertexCount)
{
	m_colorBuffer = nullptr;
	m_vertexBuffer = vertexBuffer;
	m_vertexCount = vertexCount;
	return m_vertexBuffer != nullptr;
}
bool DebugRenderer::WorldObject::InitializeBuffers()
{
	m_colorBuffer = nullptr;
	m_vertexBuffer = nullptr;
	m_vertexCount = 0;
	if(m_vertices.empty())
		return false;
	auto createInfo = prosper::util::BufferCreateInfo {};
	createInfo.size = m_vertices.size() * sizeof(m_vertices.front());
	createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	m_vertexBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo, m_vertices.data());
	m_vertexCount = m_vertices.size();

	if(m_colors.empty())
		return true;
	createInfo.size = m_colors.size() * sizeof(m_colors.front());
	m_colorBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo, m_colors.data());
	return true;
}

void DebugRenderer::WorldObject::UpdateVertexBuffer()
{
	if(m_vertexBuffer == nullptr)
		return;
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_vertexBuffer, 0ull, m_vertices.size() * sizeof(m_vertices.front()), m_vertices.data());
}

void DebugRenderer::WorldObject::UpdateColorBuffer()
{
	if(m_colorBuffer == nullptr)
		return;
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_colorBuffer, 0ull, m_colors.size() * sizeof(m_colors.front()), m_colors.data());
}

///////////////////////////

DebugRenderer::TextObject::TextObject(WIText *elText) : BaseObject(), m_hText(elText->GetHandle()), m_hCbRender()
{
	elText->AddCallback("OnTextRendered", FunctionCallback<void, std::reference_wrapper<const std::shared_ptr<prosper::RenderTarget>>>::Create([this](std::reference_wrapper<const std::shared_ptr<prosper::RenderTarget>> rt) {
		if(pragma::ShaderDebugTexture::DESCRIPTOR_SET_TEXTURE.IsValid() == false)
			return;
		if(m_descSetGroupText != nullptr)
			c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_descSetGroupText);
		auto &tex = rt.get()->GetTexture();
		m_descSetGroupText = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderDebugTexture::DESCRIPTOR_SET_TEXTURE);
		m_descSetGroupText->GetDescriptorSet()->SetBindingTexture(tex, 0u);
	}));
}
DebugRenderer::TextObject::~TextObject()
{
	if(m_descSetGroupText != nullptr)
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_descSetGroupText);
	if(m_hText.IsValid())
		m_hText->Remove();
	if(m_hCbRender.IsValid())
		m_hCbRender.Remove();
}
WIText *DebugRenderer::TextObject::GetTextElement() const { return static_cast<WIText *>(m_hText.get()); }
void DebugRenderer::TextObject::Initialize(CallbackHandle &hCallback) { m_hCbRender = hCallback; }
prosper::IDescriptorSet *DebugRenderer::TextObject::GetTextDescriptorSet() const { return (m_descSetGroupText != nullptr) ? m_descSetGroupText->GetDescriptorSet() : nullptr; }
DebugRenderer::ObjectType DebugRenderer::TextObject::GetType() const { return ObjectType::Text; }

///////////////////////////

std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawPoints(const std::shared_ptr<prosper::IBuffer> &vertexBuffer, uint32_t vertexCount, const Color &color, float duration)
{
	if(vertexCount == 0)
		return nullptr;
	auto o = std::make_shared<DebugRenderer::WorldObject>(color.ToVector4());
	if(o->InitializeBuffers(vertexBuffer, vertexCount) == false)
		return nullptr;
	cleanup();
	auto &objs = s_debugObjects[DebugRenderer::Type::PointsVertex];
	objs.push_back(DebugRenderer::RuntimeObject {o, duration});
	return objs.back().obj;
}

std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawPoints(const std::vector<Vector3> &points, const Color &color, float duration)
{
	if(points.empty())
		return nullptr;
	auto o = std::make_shared<DebugRenderer::WorldObject>(color.ToVector4());
	auto &oVerts = o->GetVertices();
	oVerts = points;
	if(o->InitializeBuffers() == false)
		return nullptr;
	cleanup();
	auto &objs = s_debugObjects[DebugRenderer::Type::Points];
	objs.push_back(DebugRenderer::RuntimeObject {o, duration});
	return objs.back().obj;
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawPoint(const Vector3 &pos, const Color &color, float duration)
{
	auto r = DrawPoints(std::vector<Vector3> {Vector3 {0.f, 0.f, 0.f}}, color, duration);
	r->SetPos(pos);
	return r;
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawLines(const std::vector<Vector3> &lines, const Color &color, float duration)
{
	auto o = std::make_shared<DebugRenderer::WorldObject>(color.ToVector4());
	auto &oVerts = o->GetVertices();
	oVerts = lines;
	if(o->InitializeBuffers() == false)
		return nullptr;
	cleanup();
	auto &objs = s_debugObjects[DebugRenderer::Type::Lines];
	objs.push_back(DebugRenderer::RuntimeObject {o, duration});
	return objs.back().obj;
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawLine(const Vector3 &start, const Vector3 &end, const Color &color, float duration) { return DrawLines(std::vector<Vector3> {start, end}, color, duration); }
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawBox(const Vector3 &center, const Vector3 &min, const Vector3 &max, const Color &color, float duration) { return DrawBox(center, min, max, EulerAngles(0.f, 0.f, 0.f), color, duration); }
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawBox(const Vector3 &start, const Vector3 &end, const Color &color, float duration) { return DrawBox(start, end, EulerAngles(0.f, 0.f, 0.f), color, duration); }
static std::shared_ptr<DebugRenderer::BaseObject> draw_box(const Vector3 &center, const Vector3 &min, const Vector3 &max, const EulerAngles &ang, const Color &color, const Color *outlineColor, float duration)
{
	auto r = std::make_shared<DebugRenderer::CollectionObject>();
	if(outlineColor != nullptr) {
		std::vector<Vector3> meshVerts = {min, {min.x, min.y, max.z}, {min.x, max.y, max.z}, {max.x, max.y, min.z}, min, {min.x, max.y, min.z}, {max.x, min.y, max.z}, min, {max.x, min.y, min.z}, {max.x, max.y, min.z}, {max.x, min.y, min.z}, min, min, {min.x, max.y, max.z},
		  {min.x, max.y, min.z}, {max.x, min.y, max.z}, {min.x, min.y, max.z}, min, {min.x, max.y, max.z}, {min.x, min.y, max.z}, {max.x, min.y, max.z}, max, {max.x, min.y, min.z}, {max.x, max.y, min.z}, {max.x, min.y, min.z}, max, {max.x, min.y, max.z}, max, {max.x, max.y, min.z},
		  {min.x, max.y, min.z}, max, {min.x, max.y, min.z}, {min.x, max.y, max.z}, max, {min.x, max.y, max.z}, {max.x, min.y, max.z}};
		auto oMesh = DebugRenderer::DrawMesh(meshVerts, color, duration);
		if(oMesh != nullptr)
			r->AddObject(oMesh);
	}

	auto oOutline = std::make_shared<DebugRenderer::WorldObject>(((outlineColor != nullptr) ? *outlineColor : color).ToVector4());
	auto &verts = oOutline->GetVertices();
	auto start = min;
	auto end = max;
	verts = {start, Vector3(start.x, start.y, end.z), start, Vector3(end.x, start.y, start.z), start, Vector3(start.x, end.y, start.z), Vector3(start.x, start.y, end.z), Vector3(end.x, start.y, end.z), Vector3(start.x, start.y, end.z), Vector3(start.x, end.y, end.z),
	  Vector3(end.x, start.y, end.z), Vector3(end.x, start.y, start.z), Vector3(end.x, start.y, end.z), end, Vector3(end.x, start.y, start.z), Vector3(end.x, end.y, start.z), Vector3(start.x, end.y, start.z), Vector3(start.x, end.y, end.z), Vector3(start.x, end.y, start.z),
	  Vector3(end.x, end.y, start.z), end, Vector3(end.x, end.y, start.z), end, Vector3(start.x, end.y, end.z)};
	if(oOutline->InitializeBuffers() == false)
		return nullptr;
	cleanup();
	auto &objs = s_debugObjects[DebugRenderer::Type::Lines];
	objs.push_back(DebugRenderer::RuntimeObject {oOutline, duration});
	r->AddObject(oOutline);
	r->SetPos(center);
	r->SetAngles(ang);
	return r;
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawBox(const Vector3 &center, const Vector3 &min, const Vector3 &max, const EulerAngles &ang, const Color &color, const Color &outlineColor, float duration)
{
	return draw_box(center, min, max, ang, color, &outlineColor, duration);
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawBox(const Vector3 &center, const Vector3 &min, const Vector3 &max, const EulerAngles &ang, const Color &color, float duration) { return draw_box(center, min, max, ang, color, nullptr, duration); }
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawBox(const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &color, const Color &outlineColor, float duration)
{
	auto center = (end + start) * 0.5f;
	return DrawBox(center, start - center, end - center, ang, color, outlineColor, duration);
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawBox(const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &color, float duration)
{
	auto center = (end + start) * 0.5f;
	return DrawBox(center, start - center, end - center, ang, color, duration);
}
static WIText *create_text_element(const std::string &text)
{
	auto *el = static_cast<WIText *>(c_game->CreateGUIElement("WIText"));
	if(el == nullptr)
		return nullptr;
	el->SetText(text);
	el->SizeToContents();
	el->SetVisible(false);
	el->SetCacheEnabled(true);
	return el;
}
static std::shared_ptr<DebugRenderer::BaseObject> draw_text(WIText *el, const Vector3 &pos, const Vector2 &worldSize, float duration)
{
	auto &sz = el->GetSize();
	if(sz.x <= 0 || sz.y <= 0)
		return nullptr;
	Vector2 szUnits {(worldSize.x / 2.f) /* /static_cast<float>(el->GetWidth())*/, (worldSize.y / 2.f) /* /static_cast<float>(el->GetHeight())*/};

	auto o = std::make_shared<DebugRenderer::TextObject>(el);
	o->SetPos(pos);
	auto *ptrO = o.get();
	auto hEl = el->GetHandle();
	auto cb = c_game->AddCallback("Render", FunctionCallback<>::Create([pos, szUnits, hEl, ptrO]() {
		auto *cam = c_game->GetRenderCamera();
		if(!hEl.IsValid() || cam == nullptr)
			return;
		auto *el = static_cast<const WIText *>(hEl.get());
		auto rot = cam->GetEntity().GetRotation();
		ptrO->SetRotation(rot);

		auto m = umat::identity();
		const Vector2 scale {2.f, 2.f};
		const Vector3 matScale {-scale.x, -scale.y, -1.f};
		m = glm::scale(m, matScale);
		m = ptrO->GetModelMatrix() * m;
		m = glm::scale(m, Vector3 {szUnits.x, szUnits.y, 1.f});
		m = cam->GetProjectionMatrix() * cam->GetViewMatrix() * m;

		auto *ds = ptrO->GetTextDescriptorSet();
		if(ds != nullptr) {
			auto *pShader = static_cast<pragma::ShaderDebugTexture *>(c_game->GetGameShader(CGame::GameShader::DebugTexture).get());
			if(pShader != nullptr) {
				auto drawCmd = c_game->GetCurrentDrawCommandBuffer();
				prosper::ShaderBindState bindState {*drawCmd};
				if(pShader->RecordBeginDraw(bindState) == true) {
					pragma::ShaderDebug::PushConstants pushConstants {m, Vector4 {1.f, 1.f, 1.f, 1.f}};
					pShader->RecordDraw(bindState, *ds, pushConstants);
					pShader->RecordEndDraw(bindState);
				}
			}
		}

		//el->Draw(scale.x,scale.y,Vector2i((scale.x *0.5f) -el->GetWidth() *0.5f,(scale.y *0.5f) -el->GetHeight() *0.5f),Vector2i(0,0),Vector2i(0,0),Vector2i(0,0),umat::identity(),false,&m);
	}));
	o->Initialize(cb);
	cleanup();
	auto &objs = s_debugObjects[DebugRenderer::Type::Other];
	objs.push_back(DebugRenderer::RuntimeObject {o, duration});
	return objs.back().obj;
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawText(const std::string &text, const Vector3 &pos, const Vector2 &worldSize, const Color &color, float duration)
{
	auto r = DrawText(text, pos, worldSize, duration);
	if(r == nullptr)
		return nullptr;
	auto *pText = static_cast<TextObject *>(r.get())->GetTextElement();
	pText->SetColor(color);
	if(color.r != 0 || color.g != 0 || color.b != 0) {
		auto colShadow = Color::Black;
		colShadow.a = color.a;
		pText->EnableShadow(true);
		pText->SetShadowOffset(1, 1);
		pText->SetShadowColor(colShadow);
	}
	return r;
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawText(const std::string &text, const Vector3 &pos, float sizeScale, const Color &color, float duration)
{
	auto r = DrawText(text, pos, sizeScale, duration);
	if(r == nullptr)
		return nullptr;
	auto *pText = static_cast<TextObject *>(r.get())->GetTextElement();
	pText->SetColor(color);
	if(color.r != 0 || color.g != 0 || color.b != 0) {
		auto colShadow = Color::Black;
		colShadow.a = color.a;
		pText->EnableShadow(true);
		pText->SetShadowOffset(1, 1);
		pText->SetShadowColor(colShadow);
	}
	return r;
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawText(const std::string &text, const Vector3 &pos, const Vector2 &worldSize, float duration)
{
	auto *pText = create_text_element(text);
	if(pText == nullptr)
		return nullptr;
	return draw_text(pText, pos, worldSize, duration);
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawText(const std::string &text, const Vector3 &pos, float sizeScale, float duration)
{
	auto *pText = create_text_element(text);
	if(pText == nullptr)
		return nullptr;
	auto &sz = pText->GetSize();
	return draw_text(pText, pos, Vector2 {sz.x, sz.y} * sizeScale, duration);
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawMesh(const std::vector<Vector3> &verts, const Color &color, float duration)
{
	if(verts.empty() == true)
		return nullptr;
	auto o = std::make_shared<DebugRenderer::WorldObject>(color.ToVector4());
	auto &oVerts = o->GetVertices();
	oVerts = verts;
	if(o->InitializeBuffers() == false)
		return nullptr;
	cleanup();
	auto &triangleObjs = s_debugObjects[DebugRenderer::Type::Triangles];
	triangleObjs.push_back(DebugRenderer::RuntimeObject {o, duration});
	return triangleObjs.back().obj;
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawMesh(const std::vector<Vector3> &verts, const Color &color, const Color &outlineColor, float duration)
{
	if(verts.empty() == true)
		return nullptr;
	auto o = DrawMesh(verts, color, duration);
	static_cast<WorldObject *>(o.get())->SetOutlineColor(outlineColor.ToVector4());
	return o;
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawSphere(const Vector3 &origin, float radius, const Color &color, const Color &outlineColor, float duration, uint32_t recursionLevel)
{
	auto o = DrawSphere(origin, radius, color, duration, recursionLevel);
	static_cast<WorldObject *>(o.get())->SetOutlineColor(outlineColor.ToVector4());
	return o;
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawSphere(const Vector3 &origin, float radius, const Color &color, float duration, uint32_t recursionLevel)
{
	std::vector<Vector3> verts;
	IcoSphere::Create(Vector3 {0.f, 0.f, 0.f}, radius, verts, recursionLevel);
	auto r = DrawMesh(verts, color, duration);
	r->SetPos(origin);
	return r;
}
static std::shared_ptr<DebugRenderer::BaseObject> draw_truncated_cone(const Vector3 &origin, float startRadius, const Vector3 &dir, float dist, float endRadius, const Color &color, const Color *outlineColor, float duration, uint32_t segmentCount)
{
	auto rot = uquat::create_look_rotation(dir, uvec::get_perpendicular(dir));

	std::vector<Vector3> verts;
	std::vector<uint16_t> triangles;
	umath::geometry::generate_truncated_cone_mesh({}, startRadius, {0.f, 0.f, 1.f}, dist, endRadius, verts, &triangles, nullptr, segmentCount);

	std::vector<Vector3> meshVerts;
	meshVerts.reserve(triangles.size());
	for(auto idx : triangles)
		meshVerts.push_back(verts[idx]);

	auto r = DebugRenderer::DrawMesh(meshVerts, color, duration);
	r->SetPos(origin);
	r->SetRotation(rot);
	if(outlineColor != nullptr)
		static_cast<DebugRenderer::WorldObject *>(r.get())->SetOutlineColor(outlineColor->ToVector4());
	return r;
}
static std::shared_ptr<DebugRenderer::BaseObject> draw_cone(const Vector3 &origin, const Vector3 &dir, float dist, float angle, const Color &color, const Color *outlineColor, float duration, uint32_t segmentCount)
{
	auto radius = dist * umath::tan(umath::deg_to_rad(angle));
	return draw_truncated_cone(origin, 0.f, dir, dist, radius, color, outlineColor, duration, segmentCount);
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawCone(const Vector3 &origin, const Vector3 &dir, float dist, float angle, const Color &color, const Color &outlineColor, float duration, uint32_t segmentCount)
{
	return draw_cone(origin, dir, dist, angle, color, &outlineColor, duration, segmentCount);
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawCone(const Vector3 &origin, const Vector3 &dir, float dist, float angle, const Color &color, float duration, uint32_t segmentCount) { return draw_cone(origin, dir, dist, angle, color, nullptr, duration, segmentCount); }
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawTruncatedCone(const Vector3 &origin, float startRadius, const Vector3 &dir, float dist, float endRadius, const Color &color, float duration, uint32_t segmentCount)
{
	return draw_truncated_cone(origin, startRadius, dir, dist, endRadius, color, nullptr, duration, segmentCount);
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawTruncatedCone(const Vector3 &origin, float startRadius, const Vector3 &dir, float dist, float endRadius, const Color &color, const Color &outlineColor, float duration, uint32_t segmentCount)
{
	return draw_truncated_cone(origin, startRadius, dir, dist, endRadius, color, &outlineColor, duration, segmentCount);
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawCylinder(const Vector3 &origin, const Vector3 &dir, float dist, float radius, const Color &color, float duration, uint32_t segmentCount)
{
	return draw_truncated_cone(origin, radius, dir, dist, radius, color, nullptr, duration, segmentCount);
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawCylinder(const Vector3 &origin, const Vector3 &dir, float dist, float radius, const Color &color, const Color &outlineColor, float duration, uint32_t segmentCount)
{
	return draw_truncated_cone(origin, radius, dir, dist, radius, color, &outlineColor, duration, segmentCount);
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawPath(const std::vector<Vector3> &path, const Color &color, float duration)
{
	if(path.size() < 2)
		return nullptr;
	std::vector<Vector3> lines;
	lines.reserve((path.size() - 1) * 2);
	for(auto i = decltype(path.size()) {0}; i < path.size() - 1; ++i) {
		lines.push_back(path[i]);
		lines.push_back(path[i + 1]);
	}
	return DrawLines(lines, color, duration);
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawSpline(const std::vector<Vector3> &path, const Color &color, uint32_t segmentCount, float curvature, float duration)
{
	if(path.size() < 2)
		return nullptr;
	std::vector<Vector3> splinePath;
	auto numPath = path.size();
	splinePath.reserve((numPath - 1) * segmentCount * 2);
	for(auto i = decltype(numPath) {0}; i < numPath - 1; ++i) {
		auto &p1 = path[i];
		auto &p0 = (i > 0) ? path[i - 1] : p1;
		auto &p2 = path[i + 1];
		auto &p3 = (i < numPath - 2) ? path[i + 2] : p2;
		util::calc_hermite_spline(p0, p1, p2, p3, segmentCount, splinePath, curvature);
	}
	return DrawPath(splinePath, color, duration);
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawPlane(const Vector3 &n, float dist, const Color &color, float duration)
{
	auto perp = uvec::get_perpendicular(n);
	auto origin = n * dist;
	auto c = uvec::cross(n, perp);
	uvec::normalize(&c);
	const auto d = 50'000.f;
	auto v0 = origin - perp * d;
	auto v1 = origin - c * d;
	auto v2 = origin + perp * d;
	auto v3 = origin + c * d;
	std::vector<Vector3> mesh = {v0, v1, v2, v2, v3, v0,

	  v2, v1, v0, v0, v3, v2};
	auto r = std::make_shared<CollectionObject>();
	auto rMesh = DrawMesh(mesh, color, duration);
	if(rMesh != nullptr)
		r->AddObject(rMesh);
	Color colInv = {255 - color.r, 255 - color.g, 255 - color.b, color.a};
	auto rLines = DrawLines({v0, v2, v1, v3, origin, origin + n * 100.f}, colInv, duration);
	if(rLines != nullptr)
		r->AddObject(rLines);
	return r;
}
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawPlane(const umath::Plane &plane, const Color &color, float duration) { return DrawPlane(const_cast<umath::Plane &>(plane).GetNormal(), plane.GetDistance(), color, duration); }
std::shared_ptr<DebugRenderer::BaseObject> DebugRenderer::DrawFrustum(const std::vector<Vector3> &points, float duration)
{
	if(points.size() < 8)
		return nullptr;
	auto a = uint16_t {64};
	auto col = Color(255, 0, 0, a);
	auto r = std::make_shared<CollectionObject>();
	// Near
	r->AddObject(DrawMesh(
	  {
	    points.at(umath::to_integral(FrustumPoint::NearBottomLeft)),
	    points.at(umath::to_integral(FrustumPoint::NearBottomRight)),
	    points.at(umath::to_integral(FrustumPoint::NearTopRight)),

	    points.at(umath::to_integral(FrustumPoint::NearTopRight)),
	    points.at(umath::to_integral(FrustumPoint::NearTopLeft)),
	    points.at(umath::to_integral(FrustumPoint::NearBottomLeft)),
	  },
	  col, duration));
	// Far
	col = Color(0, 255, 0, a);
	r->AddObject(DrawMesh({points.at(umath::to_integral(FrustumPoint::FarBottomLeft)), points.at(umath::to_integral(FrustumPoint::FarTopLeft)), points.at(umath::to_integral(FrustumPoint::FarTopRight)),

	                        points.at(umath::to_integral(FrustumPoint::FarTopRight)), points.at(umath::to_integral(FrustumPoint::FarBottomRight)), points.at(umath::to_integral(FrustumPoint::FarBottomLeft))},
	  col, duration));
	// Left
	col = Color(0, 0, 255, a);
	r->AddObject(DrawMesh({points.at(umath::to_integral(FrustumPoint::FarBottomLeft)), points.at(umath::to_integral(FrustumPoint::NearBottomLeft)), points.at(umath::to_integral(FrustumPoint::NearTopLeft)),

	                        points.at(umath::to_integral(FrustumPoint::NearTopLeft)), points.at(umath::to_integral(FrustumPoint::FarTopLeft)), points.at(umath::to_integral(FrustumPoint::FarBottomLeft))},
	  col, duration));
	// Right
	col = Color(255, 255, 0, a);
	r->AddObject(DrawMesh({points.at(umath::to_integral(FrustumPoint::FarBottomRight)), points.at(umath::to_integral(FrustumPoint::FarTopRight)), points.at(umath::to_integral(FrustumPoint::NearTopRight)),

	                        points.at(umath::to_integral(FrustumPoint::NearTopRight)), points.at(umath::to_integral(FrustumPoint::NearBottomRight)), points.at(umath::to_integral(FrustumPoint::FarBottomRight))},
	  col, duration));
	// Top
	col = Color(255, 0, 255, a);
	r->AddObject(DrawMesh({points.at(umath::to_integral(FrustumPoint::NearTopLeft)), points.at(umath::to_integral(FrustumPoint::NearTopRight)), points.at(umath::to_integral(FrustumPoint::FarTopRight)),

	                        points.at(umath::to_integral(FrustumPoint::FarTopRight)), points.at(umath::to_integral(FrustumPoint::FarTopLeft)), points.at(umath::to_integral(FrustumPoint::NearTopLeft))},
	  col, duration));
	// Bottom
	col = Color(0, 255, 255, a);
	r->AddObject(DrawMesh({points.at(umath::to_integral(FrustumPoint::NearBottomLeft)), points.at(umath::to_integral(FrustumPoint::FarBottomLeft)), points.at(umath::to_integral(FrustumPoint::FarBottomRight)),

	                        points.at(umath::to_integral(FrustumPoint::FarBottomRight)), points.at(umath::to_integral(FrustumPoint::NearBottomRight)), points.at(umath::to_integral(FrustumPoint::NearBottomLeft))},
	  col, duration));
	return r;
}
std::array<std::shared_ptr<DebugRenderer::BaseObject>, 3> DebugRenderer::DrawAxis(const Vector3 &origin, const Vector3 &x, const Vector3 &y, const Vector3 &z, float duration)
{
	const float distance = 16.f;
	return {DrawLine(origin, origin + x * distance, Color(255, 0, 0, 255), duration), DrawLine(origin, origin + y * distance, Color(0, 255, 0, 255), duration), DrawLine(origin, origin + z * distance, Color(0, 0, 255, 255), duration)};
}
std::array<std::shared_ptr<DebugRenderer::BaseObject>, 3> DebugRenderer::DrawAxis(const Vector3 &origin, const EulerAngles &ang, float duration)
{
	Vector3 x {1.f, 0.f, 0.f};
	Vector3 y {0.f, 1.f, 0.f};
	Vector3 z {0.f, 0.f, 1.f};
	uvec::rotate(&x, ang);
	uvec::rotate(&y, ang);
	uvec::rotate(&z, ang);
	return DrawAxis(origin, x, y, z, duration);
}
std::array<std::shared_ptr<DebugRenderer::BaseObject>, 3> DebugRenderer::DrawAxis(const Vector3 &origin, float duration) { return DrawAxis(origin, EulerAngles(0.f, 0.f, 0.f), duration); }
void DebugRenderer::ClearObjects()
{
	for(auto &it : s_debugObjects)
		it.second.clear();
}
void DebugRenderer::Render(std::shared_ptr<prosper::ICommandBuffer> &drawCmd, pragma::CCameraComponent &cam)
{
	if(s_debugObjects.empty())
		return;
	auto &whDebugShader = c_game->GetGameShader(CGame::GameShader::Debug);
	auto &whDebugVertexShader = c_game->GetGameShader(CGame::GameShader::DebugVertex);
	if(whDebugShader.expired() || whDebugVertexShader.expired())
		return;
	const std::unordered_map<DebugRenderer::Type, pragma::ShaderDebug::Pipeline> shaderPipeline = {{DebugRenderer::Type::Triangles, pragma::ShaderDebug::Pipeline::Triangle}, {DebugRenderer::Type::Lines, pragma::ShaderDebug::Pipeline::Line},
	  {DebugRenderer::Type::LinesStrip, pragma::ShaderDebug::Pipeline::LineStrip}, {DebugRenderer::Type::Points, pragma::ShaderDebug::Pipeline::Point}, {DebugRenderer::Type::PointsVertex, pragma::ShaderDebug::Pipeline::Vertex}};
	auto &t = client->RealTime();

	auto vp = cam.GetProjectionMatrix() * cam.GetViewMatrix();
	auto *shader = static_cast<pragma::ShaderDebug *>(whDebugShader.get());
	auto *shaderVertex = static_cast<pragma::ShaderDebugVertexColor *>(whDebugVertexShader.get());
	std::queue<std::shared_ptr<DebugRenderer::BaseObject>> outlines;
	static std::mutex g_renderDbgMutex;
	g_renderDbgMutex.lock();
	for(auto &pair : s_debugObjects) {
		auto type = pair.first;
		auto &meshes = pair.second;
		if(type == DebugRenderer::Type::Other) {
			for(auto it = meshes.begin(); it != meshes.end();) {
				auto &mesh = *it;
				if(!mesh.obj->IsValid() || (t > mesh.time && mesh.obj.use_count() == 1))
					it = meshes.erase(it);
				else
					++it;
			}
			continue;
		}
		if(meshes.empty())
			continue;
		auto itPipeline = shaderPipeline.find(type);
		prosper::ShaderBindState bindState {*drawCmd};
		if(itPipeline == shaderPipeline.end() || shader->RecordBeginDraw(bindState, itPipeline->second) == false)
			continue;
		auto pipelineId = itPipeline->second;
		if(pipelineId == pragma::ShaderDebug::Pipeline::Line || pipelineId == pragma::ShaderDebug::Pipeline::Wireframe || pipelineId == pragma::ShaderDebug::Pipeline::LineStrip)
			drawCmd->RecordSetLineWidth(2.f);
		for(auto it = meshes.begin(); it != meshes.end();) {
			auto &mesh = *it;
			if(!is_obj_valid(mesh, t))
				it = meshes.erase(it);
			else {
				auto &o = mesh.obj;
				if(o->IsVisible() == true && o->GetType() == DebugRenderer::ObjectType::World) {
					auto *ptrO = static_cast<DebugRenderer::WorldObject *>(o.get());
					auto &colBuffer = ptrO->GetColorBuffer();
					auto mvp = vp * ptrO->GetModelMatrix();
					if(colBuffer == nullptr)
						shader->RecordDraw(bindState, *ptrO->GetVertexBuffer(), ptrO->GetVertexCount(), mvp, ptrO->GetColor());
					else {
						//shader->Draw(ptrO->GetVertexBuffer(),ptrO->GetVertexCount(),mvp);
						// prosper TODO: Vertex color shader
					}
					if(ptrO->HasOutline())
						outlines.push(o);
				}
				++it;
			}
		}
		shader->RecordEndDraw(bindState);
	}
	g_renderDbgMutex.unlock();
	prosper::ShaderBindState bindState {*drawCmd};
	if(outlines.empty() || shader->RecordBeginDraw(bindState, pragma::ShaderDebug::Pipeline::Wireframe) == false)
		return;
	while(!outlines.empty()) {
		auto &o = static_cast<DebugRenderer::WorldObject &>(*outlines.front());
		shader->RecordDraw(bindState, *o.GetVertexBuffer(), o.GetVertexCount(), vp * o.GetModelMatrix());
		outlines.pop();
	}
	shader->RecordEndDraw(bindState);
}
