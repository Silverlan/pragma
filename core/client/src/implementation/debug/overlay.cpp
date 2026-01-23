// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :debug.overlay;
import :client_state;
import :engine;
import :game;
import :rendering.shaders;
import pragma.string.unicode;

namespace pragma::debug::DebugRenderer {
	struct RuntimeObject {
		RuntimeObject(const std::shared_ptr<BaseObject> &o, float duration);
		std::shared_ptr<BaseObject> obj;
		double time;
	};
	RuntimeObject::RuntimeObject(const std::shared_ptr<BaseObject> &o, float duration) : obj(o), time(get_client_state()->RealTime() + duration) {}
}

static void init_debug_object(pragma::debug::DebugRenderer::BaseObject &o, const pragma::debug::DebugRenderInfo &renderInfo) { o.SetIgnoreDepth(renderInfo.ignoreDepthBuffer); }

static constexpr uint32_t s_maxDebugObjectCount = 100'000;
static std::unordered_map<pragma::debug::DebugRenderer::Type, std::vector<pragma::debug::DebugRenderer::RuntimeObject>> s_debugObjects {{pragma::debug::DebugRenderer::Type::Triangles, {}}, {pragma::debug::DebugRenderer::Type::Lines, {}},
  {pragma::debug::DebugRenderer::Type::LinesStrip, {}}, {pragma::debug::DebugRenderer::Type::Points, {}}, {pragma::debug::DebugRenderer::Type::PointsVertex, {}}, {pragma::debug::DebugRenderer::Type::Other, {}}};

static bool is_obj_valid(pragma::debug::DebugRenderer::RuntimeObject &o, double t) { return o.obj != nullptr && o.obj->IsValid() && (t <= o.time || o.obj.use_count() != 1); }

static void cleanup()
{
	// Clean up old debug objects once in a while
	static uint32_t g_lastCleanupN = 0;
	if(++g_lastCleanupN < 1'000)
		return;
	g_lastCleanupN = 0;
	auto &t = pragma::get_client_state()->RealTime();
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

pragma::debug::DebugRenderer::BaseObject::BaseObject() { math::set_flag(m_flags, Flags::Valid); }
bool pragma::debug::DebugRenderer::BaseObject::IsValid() const { return math::is_flag_set(m_flags, Flags::Valid); }
void pragma::debug::DebugRenderer::BaseObject::Remove() { math::set_flag(m_flags, Flags::Valid, false); }
const pragma::math::ScaledTransform &pragma::debug::DebugRenderer::BaseObject::GetPose() const { return const_cast<BaseObject *>(this)->GetPose(); }
pragma::math::ScaledTransform &pragma::debug::DebugRenderer::BaseObject::GetPose() { return m_pose; }
void pragma::debug::DebugRenderer::BaseObject::SetPose(const math::ScaledTransform &pose)
{
	m_pose = pose;
	UpdateModelMatrix();
}
const Vector3 &pragma::debug::DebugRenderer::BaseObject::GetPos() const { return m_pose.GetOrigin(); }
void pragma::debug::DebugRenderer::BaseObject::SetPos(const Vector3 &pos)
{
	m_pose.SetOrigin(pos);
	UpdateModelMatrix();
}
const Quat &pragma::debug::DebugRenderer::BaseObject::GetRotation() const { return m_pose.GetRotation(); }
void pragma::debug::DebugRenderer::BaseObject::SetRotation(const Quat &rot)
{
	m_pose.SetRotation(rot);
	UpdateModelMatrix();
}
EulerAngles pragma::debug::DebugRenderer::BaseObject::GetAngles() const { return EulerAngles(m_pose.GetRotation()); }
void pragma::debug::DebugRenderer::BaseObject::SetAngles(const EulerAngles &ang) { SetRotation(uquat::create(ang)); }
const Vector3 &pragma::debug::DebugRenderer::BaseObject::GetScale() const { return m_pose.GetScale(); }
void pragma::debug::DebugRenderer::BaseObject::SetScale(const Vector3 &scale)
{
	m_pose.SetScale(scale);
	UpdateModelMatrix();
}
const Mat4 &pragma::debug::DebugRenderer::BaseObject::GetModelMatrix() const { return m_modelMatrix; }
bool pragma::debug::DebugRenderer::BaseObject::IsVisible() const { return math::is_flag_set(m_flags, Flags::Visible); }
void pragma::debug::DebugRenderer::BaseObject::SetVisible(bool b) { math::set_flag(m_flags, Flags::Visible, b); }
bool pragma::debug::DebugRenderer::BaseObject::ShouldIgnoreDepth() const { return math::is_flag_set(m_flags, Flags::IgnoreDepth); }
void pragma::debug::DebugRenderer::BaseObject::SetIgnoreDepth(bool b) { math::set_flag(m_flags, Flags::IgnoreDepth, b); }
void pragma::debug::DebugRenderer::BaseObject::UpdateModelMatrix() { m_modelMatrix = m_pose.Transform::ToMatrix() * glm::gtc::scale(glm::mat4 {1.f}, GetScale()); }

///////////////////////////

pragma::debug::DebugRenderer::CollectionObject::CollectionObject(const std::vector<std::shared_ptr<BaseObject>> &objs) : BaseObject(), m_objects(objs) {}
void pragma::debug::DebugRenderer::CollectionObject::AddObject(const std::shared_ptr<BaseObject> &o) { m_objects.push_back(o); }
void pragma::debug::DebugRenderer::CollectionObject::SetPos(const Vector3 &pos)
{
	BaseObject::SetPos(pos);
	for(auto &o : m_objects)
		o->SetPos(pos);
}
void pragma::debug::DebugRenderer::CollectionObject::SetRotation(const Quat &rot)
{
	BaseObject::SetRotation(rot);
	for(auto &o : m_objects)
		o->SetRotation(rot);
}
void pragma::debug::DebugRenderer::CollectionObject::SetAngles(const EulerAngles &ang)
{
	BaseObject::SetAngles(ang);
	for(auto &o : m_objects)
		o->SetAngles(ang);
}
void pragma::debug::DebugRenderer::CollectionObject::Remove()
{
	BaseObject::Remove();
	for(auto &o : m_objects)
		o->Remove();
}
void pragma::debug::DebugRenderer::CollectionObject::SetScale(const Vector3 &scale)
{
	BaseObject::SetScale(scale);
	for(auto &o : m_objects)
		o->SetScale(scale);
}
pragma::debug::DebugRenderer::ObjectType pragma::debug::DebugRenderer::CollectionObject::GetType() const { return ObjectType::Collection; }

///////////////////////////

pragma::debug::DebugRenderer::WorldObject::WorldObject() : BaseObject(), m_vertexBuffer(nullptr), m_colorBuffer(nullptr), m_color {0.f, 0.f, 0.f, 1.f} {}

pragma::debug::DebugRenderer::WorldObject::WorldObject(const Vector4 &color) : BaseObject(), m_vertexBuffer(nullptr), m_colorBuffer(nullptr), m_color(color) {}
pragma::debug::DebugRenderer::WorldObject::~WorldObject()
{
	if(m_vertexBuffer != nullptr)
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_vertexBuffer);
	if(m_colorBuffer != nullptr)
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_colorBuffer);
}
pragma::debug::DebugRenderer::ObjectType pragma::debug::DebugRenderer::WorldObject::GetType() const { return ObjectType::World; }
const Vector4 &pragma::debug::DebugRenderer::WorldObject::GetColor() const { return m_color; }
const Vector4 &pragma::debug::DebugRenderer::WorldObject::GetOutlineColor() const { return m_outlineColor; }
void pragma::debug::DebugRenderer::WorldObject::SetColor(const Vector4 &col) { m_color = col; }
void pragma::debug::DebugRenderer::WorldObject::SetOutlineColor(const Vector4 &outlineColor)
{
	m_bOutline = true;
	m_outlineColor = outlineColor;
}
bool pragma::debug::DebugRenderer::WorldObject::HasOutline() const { return m_bOutline; }
const std::shared_ptr<prosper::IBuffer> &pragma::debug::DebugRenderer::WorldObject::GetColorBuffer() const { return m_colorBuffer; }
const std::shared_ptr<prosper::IBuffer> &pragma::debug::DebugRenderer::WorldObject::GetVertexBuffer() const { return m_vertexBuffer; }
uint32_t pragma::debug::DebugRenderer::WorldObject::GetVertexCount() const { return (m_vertexCount != std::numeric_limits<uint32_t>::max()) ? m_vertexCount : m_vertices.size(); }
void pragma::debug::DebugRenderer::WorldObject::SetVertexPosition(uint32_t idx, const Vector3 &pos)
{
	if(idx >= m_vertices.size())
		return;
	m_vertices[idx] = pos;
}
std::optional<Vector3> pragma::debug::DebugRenderer::WorldObject::GetVertexPosition(uint32_t idx) const
{
	if(idx >= m_vertices.size())
		return {};
	return m_vertices[idx];
}
void pragma::debug::DebugRenderer::WorldObject::AddVertex(const Vector3 &v) { m_vertices.push_back(v); }
std::vector<Vector3> &pragma::debug::DebugRenderer::WorldObject::GetVertices() { return m_vertices; }
std::vector<Vector4> &pragma::debug::DebugRenderer::WorldObject::GetColors() { return m_colors; }
bool pragma::debug::DebugRenderer::WorldObject::InitializeBuffers(const std::shared_ptr<prosper::IBuffer> &vertexBuffer, uint32_t vertexCount)
{
	m_colorBuffer = nullptr;
	m_vertexBuffer = vertexBuffer;
	m_vertexCount = vertexCount;
	return m_vertexBuffer != nullptr;
}
bool pragma::debug::DebugRenderer::WorldObject::InitializeBuffers()
{
	m_colorBuffer = nullptr;
	m_vertexBuffer = nullptr;
	m_vertexCount = 0;
	if(m_vertices.empty())
		return false;
	m_vertexBuffer = get_cengine()->GetRenderContext().AllocateTemporaryBuffer(util::size_of_container(m_vertices), sizeof(Vector4), m_vertices.data());
	m_vertexCount = m_vertices.size();

	if(m_colors.empty())
		return true;
	m_colorBuffer = get_cengine()->GetRenderContext().AllocateTemporaryBuffer(util::size_of_container(m_colors), sizeof(Vector4), m_colors.data());
	return true;
}

void pragma::debug::DebugRenderer::WorldObject::UpdateVertexBuffer()
{
	if(m_vertexBuffer == nullptr)
		return;
	get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_vertexBuffer, 0ull, m_vertices.size() * sizeof(m_vertices.front()), m_vertices.data());
}

void pragma::debug::DebugRenderer::WorldObject::UpdateColorBuffer()
{
	if(m_colorBuffer == nullptr)
		return;
	get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_colorBuffer, 0ull, m_colors.size() * sizeof(m_colors.front()), m_colors.data());
}

///////////////////////////

pragma::debug::DebugRenderer::TextObject::TextObject(gui::types::WIText *elText) : BaseObject(), m_hText(elText->GetHandle()), m_hCbRender()
{
	elText->AddCallback("OnTextRendered", FunctionCallback<void, std::reference_wrapper<const std::shared_ptr<prosper::RenderTarget>>>::Create([this](std::reference_wrapper<const std::shared_ptr<prosper::RenderTarget>> rt) {
		if(ShaderDebugTexture::DESCRIPTOR_SET_TEXTURE.IsValid() == false)
			return;
		if(m_descSetGroupText != nullptr)
			get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_descSetGroupText);
		auto &tex = rt.get()->GetTexture();
		m_descSetGroupText = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderDebugTexture::DESCRIPTOR_SET_TEXTURE);
		m_descSetGroupText->GetDescriptorSet()->SetBindingTexture(tex, 0u);
	}));
}
pragma::debug::DebugRenderer::TextObject::~TextObject()
{
	if(m_descSetGroupText != nullptr)
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_descSetGroupText);
	if(m_hText.IsValid())
		m_hText->Remove();
	if(m_hCbRender.IsValid())
		m_hCbRender.Remove();
}
pragma::gui::types::WIText *pragma::debug::DebugRenderer::TextObject::GetTextElement() const { return static_cast<gui::types::WIText *>(m_hText.get()); }
void pragma::debug::DebugRenderer::TextObject::Initialize(CallbackHandle &hCallback) { m_hCbRender = hCallback; }
prosper::IDescriptorSet *pragma::debug::DebugRenderer::TextObject::GetTextDescriptorSet() const { return (m_descSetGroupText != nullptr) ? m_descSetGroupText->GetDescriptorSet() : nullptr; }
pragma::debug::DebugRenderer::ObjectType pragma::debug::DebugRenderer::TextObject::GetType() const { return ObjectType::Text; }

///////////////////////////

std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawPoints(const std::shared_ptr<prosper::IBuffer> &vertexBuffer, uint32_t vertexCount, const DebugRenderInfo &renderInfo)
{
	if(vertexCount == 0)
		return nullptr;
	auto o = pragma::util::make_shared<WorldObject>(renderInfo.color.ToVector4());
	init_debug_object(*o, renderInfo);
	if(o->InitializeBuffers(vertexBuffer, vertexCount) == false)
		return nullptr;
	cleanup();
	auto &objs = s_debugObjects[Type::PointsVertex];
	objs.push_back(RuntimeObject {o, renderInfo.duration});
	return objs.back().obj;
}

std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawPoints(const std::vector<Vector3> &points, const DebugRenderInfo &renderInfo)
{
	if(points.empty())
		return nullptr;
	auto o = pragma::util::make_shared<WorldObject>(renderInfo.color.ToVector4());
	init_debug_object(*o, renderInfo);
	auto &oVerts = o->GetVertices();
	oVerts = points;
	if(o->InitializeBuffers() == false)
		return nullptr;
	cleanup();
	auto &objs = s_debugObjects[Type::Points];
	objs.push_back(RuntimeObject {o, renderInfo.duration});
	return objs.back().obj;
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawPoint(const DebugRenderInfo &renderInfo)
{
	auto r = DrawPoints(std::vector<Vector3> {Vector3 {0.f, 0.f, 0.f}}, renderInfo);
	r->SetPos(renderInfo.pose.GetOrigin());
	return r;
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawLines(const std::vector<Vector3> &lines, const DebugRenderInfo &renderInfo)
{
	auto o = pragma::util::make_shared<WorldObject>(renderInfo.color.ToVector4());
	init_debug_object(*o, renderInfo);
	auto &oVerts = o->GetVertices();
	oVerts = lines;
	if(o->InitializeBuffers() == false)
		return nullptr;
	cleanup();
	auto &objs = s_debugObjects[Type::Lines];
	objs.push_back(RuntimeObject {o, renderInfo.duration});
	return objs.back().obj;
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawLine(const Vector3 &start, const Vector3 &end, const DebugRenderInfo &renderInfo) { return DrawLines(std::vector<Vector3> {start, end}, renderInfo); }
static std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> draw_box(const Vector3 &center, const Vector3 &min, const Vector3 &max, const pragma::debug::DebugRenderInfo &renderInfo, const Color *outlineColor)
{
	auto r = pragma::util::make_shared<pragma::debug::DebugRenderer::CollectionObject>();
	if(outlineColor != nullptr) {
		pragma::debug::DebugRenderInfo outlineDrawInfo {};
		outlineDrawInfo.color = renderInfo.color;
		outlineDrawInfo.duration = renderInfo.duration;
		std::vector<Vector3> meshVerts = {min, {min.x, min.y, max.z}, {min.x, max.y, max.z}, {max.x, max.y, min.z}, min, {min.x, max.y, min.z}, {max.x, min.y, max.z}, min, {max.x, min.y, min.z}, {max.x, max.y, min.z}, {max.x, min.y, min.z}, min, min, {min.x, max.y, max.z},
		  {min.x, max.y, min.z}, {max.x, min.y, max.z}, {min.x, min.y, max.z}, min, {min.x, max.y, max.z}, {min.x, min.y, max.z}, {max.x, min.y, max.z}, max, {max.x, min.y, min.z}, {max.x, max.y, min.z}, {max.x, min.y, min.z}, max, {max.x, min.y, max.z}, max, {max.x, max.y, min.z},
		  {min.x, max.y, min.z}, max, {min.x, max.y, min.z}, {min.x, max.y, max.z}, max, {min.x, max.y, max.z}, {max.x, min.y, max.z}};
		auto oMesh = pragma::debug::DebugRenderer::DrawMesh(meshVerts, outlineDrawInfo);
		if(oMesh != nullptr)
			r->AddObject(oMesh);
	}

	auto oOutline = pragma::util::make_shared<pragma::debug::DebugRenderer::WorldObject>(((outlineColor != nullptr) ? *outlineColor : renderInfo.color).ToVector4());
	init_debug_object(*oOutline, renderInfo);
	auto &verts = oOutline->GetVertices();
	auto start = min;
	auto end = max;
	verts = {start, Vector3(start.x, start.y, end.z), start, Vector3(end.x, start.y, start.z), start, Vector3(start.x, end.y, start.z), Vector3(start.x, start.y, end.z), Vector3(end.x, start.y, end.z), Vector3(start.x, start.y, end.z), Vector3(start.x, end.y, end.z),
	  Vector3(end.x, start.y, end.z), Vector3(end.x, start.y, start.z), Vector3(end.x, start.y, end.z), end, Vector3(end.x, start.y, start.z), Vector3(end.x, end.y, start.z), Vector3(start.x, end.y, start.z), Vector3(start.x, end.y, end.z), Vector3(start.x, end.y, start.z),
	  Vector3(end.x, end.y, start.z), end, Vector3(end.x, end.y, start.z), end, Vector3(start.x, end.y, end.z)};
	if(oOutline->InitializeBuffers() == false)
		return nullptr;
	cleanup();
	auto &objs = s_debugObjects[pragma::debug::DebugRenderer::Type::Lines];
	objs.push_back(pragma::debug::DebugRenderer::RuntimeObject {oOutline, renderInfo.duration});
	r->AddObject(oOutline);
	r->SetPos(center);
	r->SetAngles(renderInfo.pose.GetRotation());
	return r;
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawBox(const Vector3 &start, const Vector3 &end, const DebugRenderInfo &renderInfo)
{
	auto center = (end + start) * 0.5f;
	auto *poutlineColor = renderInfo.outlineColor ? &*renderInfo.outlineColor : nullptr;
	return draw_box(renderInfo.pose.GetOrigin() + center, start - center, end - center, renderInfo, poutlineColor);
}
static pragma::gui::types::WIText *create_text_element(const std::string &text)
{
	auto *el = static_cast<pragma::gui::types::WIText *>(pragma::get_cgame()->CreateGUIElement("WIText"));
	if(el == nullptr)
		return nullptr;
	el->SetText(text);
	el->SizeToContents();
	el->SetVisible(false);
	el->SetCacheEnabled(true);
	return el;
}
static std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> draw_text(pragma::gui::types::WIText *el, const Vector3 &pos, const Vector2 &worldSize, float duration)
{
	auto &sz = el->GetSize();
	if(sz.x <= 0 || sz.y <= 0)
		return nullptr;
	Vector2 szUnits {(worldSize.x / 2.f) /* /static_cast<float>(el->GetWidth())*/, (worldSize.y / 2.f) /* /static_cast<float>(el->GetHeight())*/};

	auto o = pragma::util::make_shared<pragma::debug::DebugRenderer::TextObject>(el);
	o->SetPos(pos);
	auto *ptrO = o.get();
	auto hEl = el->GetHandle();
	auto cb = pragma::get_cgame()->AddCallback("Render", FunctionCallback<>::Create([pos, szUnits, hEl, ptrO]() {
		auto *cam = pragma::get_cgame()->GetRenderCamera<pragma::CCameraComponent>();
		if(!hEl.IsValid() || cam == nullptr)
			return;
		auto *el = static_cast<const pragma::gui::types::WIText *>(hEl.get());
		auto rot = cam->GetEntity().GetRotation();
		ptrO->SetRotation(rot);

		auto m = umat::identity();
		const Vector2 scale {2.f, 2.f};
		const Vector3 matScale {-scale.x, -scale.y, -1.f};
		m = glm::gtc::scale(m, matScale);
		m = ptrO->GetModelMatrix() * m;
		m = glm::gtc::scale(m, Vector3 {szUnits.x, szUnits.y, 1.f});
		m = cam->GetProjectionMatrix() * cam->GetViewMatrix() * m;

		auto *ds = ptrO->GetTextDescriptorSet();
		if(ds != nullptr) {
			auto *pShader = static_cast<pragma::ShaderDebugTexture *>(pragma::get_cgame()->GetGameShader(pragma::CGame::GameShader::DebugTexture).get());
			if(pShader != nullptr) {
				auto drawCmd = pragma::get_cgame()->GetCurrentDrawCommandBuffer();
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
	auto &objs = s_debugObjects[pragma::debug::DebugRenderer::Type::Other];
	objs.push_back(pragma::debug::DebugRenderer::RuntimeObject {o, duration});
	return objs.back().obj;
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawText(const DebugRenderInfo &renderInfo, const std::string &text, const Vector2 &worldSize)
{
	auto *pText = create_text_element(text);
	if(!pText)
		return nullptr;
	auto r = draw_text(pText, renderInfo.pose.GetOrigin(), worldSize, renderInfo.duration);
	if(r == nullptr) {
		pText->Remove();
		return nullptr;
	}
	auto color = renderInfo.color;
	pText->SetColor(color);
	if(color.r != 0 || color.g != 0 || color.b != 0) {
		auto colShadow = colors::Black;
		colShadow.a = color.a;
		pText->EnableShadow(true);
		pText->SetShadowOffset(1, 1);
		pText->SetShadowColor(colShadow);
	}
	return r;
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawText(const DebugRenderInfo &renderInfo, const std::string &text, float sizeScale)
{
	auto *pText = create_text_element(text);
	if(!pText)
		return nullptr;
	auto &sz = pText->GetSize();
	auto r = draw_text(pText, renderInfo.pose.GetOrigin(), Vector2 {sz.x, sz.y} * sizeScale, renderInfo.duration);
	if(r == nullptr) {
		pText->Remove();
		return nullptr;
	}
	auto color = renderInfo.color;
	pText->SetColor(color);
	if(color.r != 0 || color.g != 0 || color.b != 0) {
		auto colShadow = colors::Black;
		colShadow.a = color.a;
		pText->EnableShadow(true);
		pText->SetShadowOffset(1, 1);
		pText->SetShadowColor(colShadow);
	}
	return r;
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawMesh(const std::vector<Vector3> &verts, const DebugRenderInfo &renderInfo)
{
	if(verts.empty() == true)
		return nullptr;
	auto o = pragma::util::make_shared<WorldObject>(renderInfo.color.ToVector4());
	init_debug_object(*o, renderInfo);
	auto &oVerts = o->GetVertices();
	oVerts = verts;
	if(o->InitializeBuffers() == false)
		return nullptr;
	cleanup();
	auto &triangleObjs = s_debugObjects[Type::Triangles];
	triangleObjs.push_back(RuntimeObject {o, renderInfo.duration});
	if(renderInfo.outlineColor)
		static_cast<WorldObject *>(triangleObjs.back().obj.get())->SetOutlineColor(renderInfo.outlineColor->ToVector4());
	return triangleObjs.back().obj;
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawSphere(const DebugRenderInfo &renderInfo, float radius, uint32_t recursionLevel)
{
	std::vector<Vector3> verts;
	math::IcoSphere::Create(Vector3 {0.f, 0.f, 0.f}, radius, verts, recursionLevel);
	auto r = DrawMesh(verts, renderInfo);
	return r;
}
static std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> draw_truncated_cone(const pragma::debug::DebugRenderInfo &renderInfo, float startRadius, const Vector3 &dir, float dist, float endRadius, uint32_t segmentCount)
{
	auto rot = uquat::create_look_rotation(dir, uvec::get_perpendicular(dir));

	std::vector<Vector3> verts;
	std::vector<uint16_t> triangles;
	pragma::math::geometry::generate_truncated_cone_mesh({}, startRadius, {0.f, 0.f, 1.f}, dist, endRadius, verts, &triangles, nullptr, segmentCount);

	std::vector<Vector3> meshVerts;
	meshVerts.reserve(triangles.size());
	for(auto idx : triangles)
		meshVerts.push_back(verts[idx]);

	auto r = pragma::debug::DebugRenderer::DrawMesh(meshVerts, renderInfo);
	return r;
}
static std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> draw_cone(const pragma::debug::DebugRenderInfo &renderInfo, const Vector3 &dir, float dist, float angle, uint32_t segmentCount)
{
	auto radius = dist * pragma::math::tan(pragma::math::deg_to_rad(angle));
	return draw_truncated_cone(renderInfo, 0.f, dir, dist, radius, segmentCount);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawCone(const DebugRenderInfo &renderInfo, const Vector3 &dir, float dist, float angle, uint32_t segmentCount) { return draw_cone(renderInfo, dir, dist, angle, segmentCount); }
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawTruncatedCone(const DebugRenderInfo &renderInfo, float startRadius, const Vector3 &dir, float dist, float endRadius, uint32_t segmentCount)
{
	return draw_truncated_cone(renderInfo, startRadius, dir, dist, endRadius, segmentCount);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawCylinder(const DebugRenderInfo &renderInfo, const Vector3 &dir, float dist, float radius, uint32_t segmentCount) { return draw_truncated_cone(renderInfo, radius, dir, dist, radius, segmentCount); }
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawPath(const std::vector<Vector3> &path, const DebugRenderInfo &renderInfo)
{
	if(path.size() < 2)
		return nullptr;
	std::vector<Vector3> lines;
	lines.reserve((path.size() - 1) * 2);
	for(auto i = decltype(path.size()) {0}; i < path.size() - 1; ++i) {
		lines.push_back(path[i]);
		lines.push_back(path[i + 1]);
	}
	return DrawLines(lines, renderInfo);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawSpline(const std::vector<Vector3> &path, uint32_t segmentCount, float curvature, const DebugRenderInfo &renderInfo)
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
		math::calc_hermite_spline(p0, p1, p2, p3, segmentCount, splinePath, curvature);
	}
	return DrawPath(splinePath, renderInfo);
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawPlane(const Vector3 &n, float dist, const DebugRenderInfo &renderInfo)
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
	auto r = pragma::util::make_shared<CollectionObject>();
	auto rMesh = DrawMesh(mesh, renderInfo);
	if(rMesh != nullptr)
		r->AddObject(rMesh);
	auto color = renderInfo.color;
	Color colInv = {static_cast<int16_t>(255 - color.r), static_cast<int16_t>(255 - color.g), static_cast<int16_t>(255 - color.b), static_cast<int16_t>(color.a)};
	auto linesRenderInfo = renderInfo;
	linesRenderInfo.color = colInv;
	auto rLines = DrawLines({v0, v2, v1, v3, origin, origin + n * 100.f}, linesRenderInfo);
	if(rLines != nullptr)
		r->AddObject(rLines);
	return r;
}
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawPlane(const math::Plane &plane, const DebugRenderInfo &renderInfo) { return DrawPlane(const_cast<math::Plane &>(plane).GetNormal(), plane.GetDistance(), renderInfo); }
std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> pragma::debug::DebugRenderer::DrawFrustum(const std::vector<Vector3> &points, const DebugRenderInfo &renderInfo)
{
	if(points.size() < 8)
		return nullptr;
	auto a = uint16_t {64};
	auto col = Color(255, 0, 0, a);
	auto r = pragma::util::make_shared<CollectionObject>();
	// Near
	r->AddObject(DrawMesh(
	  {
	    points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomLeft)),
	    points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomRight)),
	    points.at(pragma::math::to_integral(math::FrustumPoint::NearTopRight)),

	    points.at(pragma::math::to_integral(math::FrustumPoint::NearTopRight)),
	    points.at(pragma::math::to_integral(math::FrustumPoint::NearTopLeft)),
	    points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomLeft)),
	  },
	  renderInfo));
	// Far
	col = Color(0, 255, 0, a);
	r->AddObject(DrawMesh({points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomLeft)), points.at(pragma::math::to_integral(math::FrustumPoint::FarTopLeft)), points.at(pragma::math::to_integral(math::FrustumPoint::FarTopRight)),

	                        points.at(pragma::math::to_integral(math::FrustumPoint::FarTopRight)), points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomRight)), points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomLeft))},
	  renderInfo));
	// Left
	col = Color(0, 0, 255, a);
	r->AddObject(DrawMesh({points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomLeft)), points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomLeft)), points.at(pragma::math::to_integral(math::FrustumPoint::NearTopLeft)),

	                        points.at(pragma::math::to_integral(math::FrustumPoint::NearTopLeft)), points.at(pragma::math::to_integral(math::FrustumPoint::FarTopLeft)), points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomLeft))},
	  renderInfo));
	// Right
	col = Color(255, 255, 0, a);
	r->AddObject(DrawMesh({points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomRight)), points.at(pragma::math::to_integral(math::FrustumPoint::FarTopRight)), points.at(pragma::math::to_integral(math::FrustumPoint::NearTopRight)),

	                        points.at(pragma::math::to_integral(math::FrustumPoint::NearTopRight)), points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomRight)), points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomRight))},
	  renderInfo));
	// Top
	col = Color(255, 0, 255, a);
	r->AddObject(DrawMesh({points.at(pragma::math::to_integral(math::FrustumPoint::NearTopLeft)), points.at(pragma::math::to_integral(math::FrustumPoint::NearTopRight)), points.at(pragma::math::to_integral(math::FrustumPoint::FarTopRight)),

	                        points.at(pragma::math::to_integral(math::FrustumPoint::FarTopRight)), points.at(pragma::math::to_integral(math::FrustumPoint::FarTopLeft)), points.at(pragma::math::to_integral(math::FrustumPoint::NearTopLeft))},
	  renderInfo));
	// Bottom
	col = Color(0, 255, 255, a);
	r->AddObject(DrawMesh({points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomLeft)), points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomLeft)), points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomRight)),

	                        points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomRight)), points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomRight)), points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomLeft))},
	  renderInfo));
	return r;
}
std::array<std::shared_ptr<pragma::debug::DebugRenderer::BaseObject>, 3> pragma::debug::DebugRenderer::DrawAxis(const DebugRenderInfo &renderInfo, const Vector3 &x, const Vector3 &y, const Vector3 &z)
{
	const float distance = 16.f;
	auto rx = x;
	auto ry = y;
	auto rz = z;
	uvec::rotate(&rx, renderInfo.pose.GetRotation());
	uvec::rotate(&ry, renderInfo.pose.GetRotation());
	uvec::rotate(&rz, renderInfo.pose.GetRotation());
	auto &origin = renderInfo.pose.GetOrigin();
	auto tmpRenderInfo = renderInfo;
	tmpRenderInfo.SetColor(colors::Red);
	auto o0 = DrawLine(origin, origin + rx * distance);
	tmpRenderInfo.SetColor(colors::Lime);
	auto o1 = DrawLine(origin, origin + ry * distance);
	tmpRenderInfo.SetColor(colors::Blue);
	auto o2 = DrawLine(origin, origin + rz * distance);
	return {o0, o1, o2};
}
std::array<std::shared_ptr<pragma::debug::DebugRenderer::BaseObject>, 3> pragma::debug::DebugRenderer::DrawAxis(const DebugRenderInfo &renderInfo)
{
	Vector3 x {1.f, 0.f, 0.f};
	Vector3 y {0.f, 1.f, 0.f};
	Vector3 z {0.f, 0.f, 1.f};
	return DrawAxis(renderInfo, x, y, z);
}
void pragma::debug::DebugRenderer::ClearObjects()
{
	for(auto &it : s_debugObjects)
		it.second.clear();
}
void pragma::debug::DebugRenderer::Render(std::shared_ptr<prosper::ICommandBuffer> &drawCmd, CCameraComponent &cam)
{
	if(s_debugObjects.empty())
		return;
	auto &whDebugShader = get_cgame()->GetGameShader(CGame::GameShader::Debug);
	auto &whDebugVertexShader = get_cgame()->GetGameShader(CGame::GameShader::DebugVertex);
	if(whDebugShader.expired() || whDebugVertexShader.expired())
		return;
	const std::unordered_map<Type, ShaderDebug::Pipeline> shaderPipeline = {
	  {Type::Triangles, ShaderDebug::Pipeline::Triangle},
	  {Type::Lines, ShaderDebug::Pipeline::Line},
	  {Type::LinesStrip, ShaderDebug::Pipeline::LineStrip},
	  {Type::Points, ShaderDebug::Pipeline::Point},
	  {Type::PointsVertex, ShaderDebug::Pipeline::Vertex},
	};
	auto &t = get_client_state()->RealTime();

	auto vp = cam.GetProjectionMatrix() * cam.GetViewMatrix();
	auto *shader = static_cast<ShaderDebug *>(whDebugShader.get());
	auto *shaderVertex = static_cast<ShaderDebugVertexColor *>(whDebugVertexShader.get());
	std::queue<std::shared_ptr<BaseObject>> outlines;
	static std::mutex g_renderDbgMutex;
	g_renderDbgMutex.lock();
	for(auto &[type, meshes] : s_debugObjects) {
		if(type == Type::Other) {
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
		if(itPipeline == shaderPipeline.end())
			continue;
		auto curPipelineId = std::numeric_limits<std::underlying_type_t<ShaderDebug::Pipeline>>::max();
		auto basePipelineId = itPipeline->second;
		prosper::ShaderBindState bindState {*drawCmd};
		for(auto it = meshes.begin(); it != meshes.end();) {
			auto &mesh = *it;
			if(!is_obj_valid(mesh, t)) {
				it = meshes.erase(it);
				continue;
			}
			auto pipelineId = basePipelineId;
			if(mesh.obj->ShouldIgnoreDepth())
				pipelineId = static_cast<ShaderDebug::Pipeline>(math::to_integral(pipelineId) + math::to_integral(ShaderDebug::Pipeline::Count));
			if(curPipelineId != math::to_integral(pipelineId)) {
				if(shader->RecordBeginDraw(bindState, pipelineId) == false) {
					++it;
					continue;
				}
				curPipelineId = math::to_integral(pipelineId);
				if(basePipelineId == ShaderDebug::Pipeline::Line || basePipelineId == ShaderDebug::Pipeline::Wireframe || basePipelineId == ShaderDebug::Pipeline::LineStrip)
					drawCmd->RecordSetLineWidth(2.f);
			}

			auto &o = mesh.obj;
			if(o->IsVisible() == true && o->GetType() == ObjectType::World) {
				auto *ptrO = static_cast<WorldObject *>(o.get());
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
		shader->RecordEndDraw(bindState);
	}
	g_renderDbgMutex.unlock();
	prosper::ShaderBindState bindState {*drawCmd};
	if(outlines.empty() || shader->RecordBeginDraw(bindState, ShaderDebug::Pipeline::Wireframe) == false)
		return;
	while(!outlines.empty()) {
		auto &o = static_cast<WorldObject &>(*outlines.front());
		shader->RecordDraw(bindState, *o.GetVertexBuffer(), o.GetVertexCount(), vp * o.GetModelMatrix());
		outlines.pop();
	}
	shader->RecordEndDraw(bindState);
}
