// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.transformable;
import :gui.snap_area;

import :engine;
import pragma.gui;

#define WIFRAME_DRAG_OFFSET_BORDER 5
#define WIFRAME_RESIZE_OFFSET_BORDER 5

pragma::gui::types::WITransformable::WITransformable() : WIBase()
{
	RegisterCallback<void>("OnClose");
	RegisterCallback<void, std::reference_wrapper<Vector2i>, bool>("TranslateTransformPosition");
	RegisterCallback<void>("OnDragStart");
	RegisterCallback<void>("OnDragEnd");
}
pragma::gui::types::WITransformable::~WITransformable()
{
	if(m_resizeMode != ResizeMode::none)
		WGUI::GetInstance().SetCursor(platform::Cursor::Shape::Arrow, GetRootWindow());
}
void pragma::gui::types::WITransformable::DoUpdate()
{
	WIBase::DoUpdate();
	Resize();
}
void pragma::gui::types::WITransformable::SetResizeRatioLocked(bool bLocked)
{
	if(bLocked == true && (GetWidth() == 0 || GetHeight() == 0)) {
		SetResizeRatioLocked(false);
		return;
	}
	math::set_flag(m_stateFlags, StateFlags::ResizeRatioLocked, bLocked);
	m_resizeRatio = GetWidth() / static_cast<float>(GetHeight());
}
bool pragma::gui::types::WITransformable::IsResizeRatioLocked() const { return math::is_flag_set(m_stateFlags, StateFlags::ResizeRatioLocked); }
void pragma::gui::types::WITransformable::SetMinWidth(int w) { SetMinSize(Vector2i(w, m_minSize.y)); }
void pragma::gui::types::WITransformable::SetMinHeight(int h) { SetMinSize(Vector2i(m_minSize.x, h)); }
void pragma::gui::types::WITransformable::SetMinSize(int w, int h) { SetMinSize(Vector2i(w, h)); }
void pragma::gui::types::WITransformable::SetMinSize(Vector2i size)
{
	m_minSize = size;
	if(size.x < 0)
		size.x = 0;
	if(size.y < 0)
		size.y = 0;
	Vector2i sz = GetSize();
	if(sz.x < size.x || sz.y < size.y) {
		if(sz.x < size.x)
			sz.x = size.x;
		if(sz.y < size.y)
			sz.y = size.y;
		SetSize(sz.x, sz.y);
	}
}
int pragma::gui::types::WITransformable::GetMinWidth() { return m_minSize.x; }
int pragma::gui::types::WITransformable::GetMinHeight() { return m_minSize.y; }
const Vector2i &pragma::gui::types::WITransformable::GetMinSize() const { return m_minSize; }
void pragma::gui::types::WITransformable::SetMaxWidth(int w) { SetMaxSize(Vector2i(w, m_maxSize.x)); }
void pragma::gui::types::WITransformable::SetMaxHeight(int h) { SetMaxSize(Vector2i(m_maxSize.x, h)); }
void pragma::gui::types::WITransformable::SetMaxSize(int w, int h) { SetMaxSize(Vector2i(w, h)); }
void pragma::gui::types::WITransformable::SetMaxSize(Vector2i size)
{
	if(size.x < -1)
		size.x = -1;
	if(size.y < -1)
		size.y = -1;
	m_maxSize = size;
	Vector2i sz = GetSize();
	if(sz.x > size.x || sz.y > size.y) {
		if(sz.x > size.x)
			sz.x = size.x;
		if(sz.y > size.y)
			sz.y = size.y;
		SetSize(sz.x, sz.y);
	}
}
int pragma::gui::types::WITransformable::GetMaxWidth() { return m_maxSize.x; }
int pragma::gui::types::WITransformable::GetMaxHeight() { return m_maxSize.y; }
const Vector2i &pragma::gui::types::WITransformable::GetMaxSize() const { return m_maxSize; }
void pragma::gui::types::WITransformable::OnTitleBarMouseEvent(platform::MouseButton button, platform::KeyState state, platform::Modifier mods)
{
	MouseCallback(button, state, mods);
	if(math::is_flag_set(m_stateFlags, StateFlags::Draggable) == false)
		return;
	if(button == platform::MouseButton::Left) {
		if(state == platform::KeyState::Press) {
			if(m_resizeMode == ResizeMode::none)
				StartDrag();
		}
		else if(state == platform::KeyState::Release)
			EndDrag();
	}
}
Vector2i pragma::gui::types::WITransformable::GetConfinedMousePos()
{
	Vector2i pos;
	auto *window = GetRootWindow();
	WGUI::GetInstance().GetMousePos(pos.x, pos.y, window);
	if(pos.x < WIFRAME_DRAG_OFFSET_BORDER)
		pos.x = WIFRAME_DRAG_OFFSET_BORDER;
	if(pos.y < WIFRAME_DRAG_OFFSET_BORDER)
		pos.y = WIFRAME_DRAG_OFFSET_BORDER;
	auto wViewport = window ? (*window)->GetSize().x : GetWidth();
	auto hViewport = window ? (*window)->GetSize().y : GetHeight();
	if(pos.x > static_cast<int32_t>(wViewport - WIFRAME_DRAG_OFFSET_BORDER))
		pos.x = wViewport - WIFRAME_DRAG_OFFSET_BORDER;
	if(pos.y > static_cast<int32_t>(hViewport - WIFRAME_DRAG_OFFSET_BORDER))
		pos.y = hViewport - WIFRAME_DRAG_OFFSET_BORDER;
	return pos;
}
void pragma::gui::types::WITransformable::AddSnapTarget(WISnapArea &target) { m_snapTargets.push_back(target.GetHandle()); }
void pragma::gui::types::WITransformable::StartDrag()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::Dragging) == true)
		return;
	GetMousePos(&m_dragCursorOffset.x, &m_dragCursorOffset.y);
	math::set_flag(m_stateFlags, StateFlags::Dragging, true);
	CallCallbacks("OnDragStart");
}
void pragma::gui::types::WITransformable::OnVisibilityChanged(bool bVisible)
{
	if(bVisible == false) {
		if(m_resizeMode != ResizeMode::none) {
			EndResizing();
			WGUI::GetInstance().SetCursor(platform::Cursor::Shape::Arrow, GetRootWindow());
		}
		EndDrag();
	}
}
void pragma::gui::types::WITransformable::EndDrag()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::Dragging) == false)
		return;
	math::set_flag(m_stateFlags, StateFlags::Dragging, false);
	math::set_flag(m_stateFlags, StateFlags::WasDragged);
	CallCallbacks("OnDragEnd");
}
void pragma::gui::types::WITransformable::StartResizing()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::Resizing) == true || IsResizable() == false)
		return;
	math::set_flag(m_stateFlags, StateFlags::Resizing, true);
	m_resizeLastPos = GetConfinedMousePos();
	EnableThinking();
}
void pragma::gui::types::WITransformable::EndResizing()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::Resizing) == false)
		return;
	math::set_flag(m_stateFlags, StateFlags::Resizing, false);
	SetResizeMode(ResizeMode::none);
}
void pragma::gui::types::WITransformable::SetResizeMode(ResizeMode mode)
{
	if(m_resizeMode == mode || IsResizable() == false)
		return;
	auto cursor = platform::Cursor::Shape::Arrow;
	switch(mode) {
	case ResizeMode::none:
		break;
	case ResizeMode::ns:
	case ResizeMode::sn:
		cursor = platform::Cursor::Shape::VResize;
		break;
	case ResizeMode::ew:
	case ResizeMode::we:
		cursor = platform::Cursor::Shape::HResize;
		break;
	default:
		cursor = platform::Cursor::Shape::Crosshair;
		break;
	};
	m_resizeMode = mode;
	SetCursor(cursor);
}
void pragma::gui::types::WITransformable::SetRemoveOnClose(bool remove) { m_bRemoveOnClose = remove; }
void pragma::gui::types::WITransformable::OnCloseButtonPressed() { Close(); }
void pragma::gui::types::WITransformable::Close()
{
	//SetVisible(false);
	if(m_bRemoveOnClose)
		RemoveSafely();
	else
		SetVisible(false);
	CallCallbacks<void>("OnClose");
}
pragma::gui::types::WIBase *pragma::gui::types::WITransformable::GetDragArea() const { return const_cast<WIBase *>(m_hMoveRect.get()); }
void pragma::gui::types::WITransformable::Initialize()
{
	WIBase::Initialize();
	EnableThinking();
	SetDraggable(true);
	SetResizable(true);
	m_hMoveRect = CreateChild<WIBase>();
	auto *pMoveRect = m_hMoveRect.get();
	pMoveRect->AddStyleClass("move_rect");
	pMoveRect->SetMouseInputEnabled(math::is_flag_set(m_stateFlags, StateFlags::Draggable));
	pMoveRect->AddCallback("OnMouseEvent",
	  FunctionCallback<util::EventReply, platform::MouseButton, platform::KeyState, platform::Modifier>::CreateWithOptionalReturn([this](util::EventReply *reply, platform::MouseButton button, platform::KeyState state, platform::Modifier mods) -> CallbackReturnType {
		  OnTitleBarMouseEvent(button, state, mods);
		  *reply = util::EventReply::Handled;
		  return CallbackReturnType::HasReturnValue;
	  }));
}
pragma::util::EventReply pragma::gui::types::WITransformable::MouseCallback(platform::MouseButton button, platform::KeyState state, platform::Modifier mods)
{
	if(WIBase::MouseCallback(button, state, mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(button == platform::MouseButton::Left) {
		if(state == platform::KeyState::Press) {
			if(m_resizeMode != ResizeMode::none)
				StartResizing();
		}
		else if(state == platform::KeyState::Release)
			EndResizing();
	}
	return util::EventReply::Handled;
}
void pragma::gui::types::WITransformable::OnCursorMoved(int x, int y)
{
	WIBase::OnCursorMoved(x, y);
	if(math::is_flag_set(m_stateFlags, StateFlags::Resizing) == true || math::is_flag_set(m_stateFlags, StateFlags::Dragging) == true)
		return;
	const auto fValidateCursorOverlap = [this]() {
		auto &wgui = WGUI::GetInstance();
		auto *pElCursor = wgui.GetCursorGUIElement(wgui.GetBaseElement(), [](WIBase *el) -> bool { return true; }, GetRootWindow());
		if(pElCursor != nullptr && pElCursor != this && pElCursor->IsDescendantOf(this) == false) {
			SetResizeMode(ResizeMode::none);
			return false;
		}
		return true;
	};
	if(x < -WIFRAME_RESIZE_OFFSET_BORDER || y < -WIFRAME_RESIZE_OFFSET_BORDER) {
		SetResizeMode(ResizeMode::none);
		return;
	}
	const Vector2i &size = GetSize();
	if(x > size.x + WIFRAME_RESIZE_OFFSET_BORDER || y > size.y + WIFRAME_RESIZE_OFFSET_BORDER) {
		SetResizeMode(ResizeMode::none);
		return;
	}
	if(y <= WIFRAME_RESIZE_OFFSET_BORDER) {
		if(fValidateCursorOverlap() == false)
			return;
		if(x <= WIFRAME_RESIZE_OFFSET_BORDER)
			SetResizeMode(ResizeMode::nwse);
		else if(x >= size.x - WIFRAME_RESIZE_OFFSET_BORDER)
			SetResizeMode(ResizeMode::nesw);
		else
			SetResizeMode(ResizeMode::ns);
	}
	else if(y >= size.y - WIFRAME_RESIZE_OFFSET_BORDER) {
		if(fValidateCursorOverlap() == false)
			return;
		if(x <= WIFRAME_RESIZE_OFFSET_BORDER)
			SetResizeMode(ResizeMode::swne);
		else if(x >= size.x - WIFRAME_RESIZE_OFFSET_BORDER)
			SetResizeMode(ResizeMode::senw);
		else
			SetResizeMode(ResizeMode::sn);
	}
	else if(x <= WIFRAME_RESIZE_OFFSET_BORDER) {
		if(fValidateCursorOverlap() == false)
			return;
		SetResizeMode(ResizeMode::ew);
	}
	else if(x >= size.x - WIFRAME_RESIZE_OFFSET_BORDER) {
		if(fValidateCursorOverlap() == false)
			return;
		SetResizeMode(ResizeMode::we);
	}
	else
		SetResizeMode(ResizeMode::none);
}
pragma::gui::types::WITransformable::ResizeMode pragma::gui::types::WITransformable::InvertResizeAxis(ResizeMode mode, bool bXAxis, bool bYAxis) const
{
	if(bXAxis == false && bYAxis == false)
		return mode;
	switch(mode) {
	case ResizeMode::ns:
		return (bYAxis == true) ? ResizeMode::sn : mode;
	case ResizeMode::sn:
		return (bYAxis == true) ? ResizeMode::ns : mode;
	case ResizeMode::we:
		return (bXAxis == true) ? ResizeMode::ew : mode;
	case ResizeMode::ew:
		return (bXAxis == true) ? ResizeMode::we : mode;
	case ResizeMode::nwse:
		return (bXAxis == true && bYAxis == true) ? ResizeMode::senw : (bYAxis == true) ? ResizeMode::swne : ResizeMode::nesw;
	case ResizeMode::nesw:
		return (bXAxis == true && bYAxis == true) ? ResizeMode::swne : (bYAxis == true) ? ResizeMode::senw : ResizeMode::nwse;
	case ResizeMode::senw:
		return (bXAxis == true && bYAxis == true) ? ResizeMode::nwse : (bYAxis == true) ? ResizeMode::nesw : ResizeMode::swne;
	case ResizeMode::swne:
		return (bXAxis == true && bYAxis == true) ? ResizeMode::nesw : (bYAxis == true) ? ResizeMode::nwse : ResizeMode::senw;
	default:
		return ResizeMode::none;
	};
}
void pragma::gui::types::WITransformable::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	WIBase::Think(drawCmd);
	if(math::is_flag_set(m_stateFlags, StateFlags::Resizing) == true) {
		Vector2i cursorPos = {};
		GetMousePos(&cursorPos.x, &cursorPos.y);

		auto size = GetSize();
		auto oldSize = size;
		auto pos = GetPos();
		auto bChangeSize = false;
		auto bChangePos = false;
		std::array<bool, 2> bSwap = {false, false};
		std::array<bool, 2> bResize = {(m_resizeMode == ResizeMode::we || m_resizeMode == ResizeMode::nesw || m_resizeMode == ResizeMode::senw) ? true : false, (m_resizeMode == ResizeMode::swne || m_resizeMode == ResizeMode::sn || m_resizeMode == ResizeMode::senw) ? true : false};
		std::array<bool, 2> bReposition = {(m_resizeMode == ResizeMode::ew || m_resizeMode == ResizeMode::swne || m_resizeMode == ResizeMode::nwse) ? true : false, (m_resizeMode == ResizeMode::nesw || m_resizeMode == ResizeMode::ns || m_resizeMode == ResizeMode::nwse) ? true : false};
		std::array<bool, 2> bKeepPos = {true, true};
		std::array<bool, 2> bKeepSize = {true, true};
		for(size_t i = 0; i < bResize.size(); ++i) {
			if(bResize[i] == true) {
				size[i] = cursorPos[i];
				bChangeSize = true;
				bKeepSize[i] = false;
			}
		}

		size += pos;
		CallCallbacks<void, std::reference_wrapper<Vector2i>, bool>("TranslateTransformPosition", std::ref(size), false);
		size -= pos;

		auto &minSize = GetMinSize();
		auto &maxSize = GetMaxSize();
		for(size_t i = decltype(bKeepSize.size()) {0}; i < bKeepSize.size(); ++i) {
			if(bKeepSize[i] == true)
				size[i] = GetSize()[i];
			else {
				if(minSize[i] >= 0)
					size[i] = math::max(size[i], minSize[i]);
				if(maxSize[i] >= 0)
					size[i] = math::min(size[i], maxSize[i]);
				if(size[i] < 0) {
					bSwap[i] = true;
					size[i] = 0;
				}
			}
		}

		for(size_t i = decltype(bReposition.size()) {0}; i < bReposition.size(); ++i) {
			if(bReposition[i] == true) {
				pos[i] += cursorPos[i];
				bChangePos = true;
				bKeepPos[i] = false;
			}
		}
		CallCallbacks<void, std::reference_wrapper<Vector2i>, bool>("TranslateTransformPosition", std::ref(pos), false);
		for(size_t i = decltype(bKeepPos.size()) {0}; i < bKeepPos.size(); ++i) {
			if(bKeepPos[i] == true)
				pos[i] = GetPos()[i];
			else {
				auto newSize = size[i] - (pos[i] - GetPos()[i]);
				if(minSize[i] >= 0 && newSize < minSize[i])
					pos[i] -= minSize[i] - newSize;
				if(maxSize[i] >= 0 && newSize > maxSize[i])
					pos[i] += maxSize[i] - newSize;

				if(pos[i] - GetPos()[i] > size[i]) {
					bSwap[i] = true;
					pos[i] = GetPos()[i] + size[i];
				}
			}
		}
		for(size_t i = decltype(bReposition.size()) {0}; i < bReposition.size(); ++i) {
			if(bReposition[i] == true) {
				size[i] -= pos[i] - GetPos()[i];
				bChangeSize = true;
			}
		}

		if(bChangeSize == true || bChangePos == true) {
			if(bChangeSize == true && IsResizeRatioLocked()) {
				if(m_resizeMode == ResizeMode::ns || m_resizeMode == ResizeMode::sn)
					size.x = size.y * m_resizeRatio;
				else if(m_resizeMode == ResizeMode::we || m_resizeMode == ResizeMode::ew)
					size.y = size.x * (1.f / m_resizeRatio);
				else if(size.x > size.y)
					size.x = size.y * m_resizeRatio;
				else
					size.y = size.x * (1.f / m_resizeRatio);
			}
			SetSize(size);
		}
		if(bChangePos == true)
			SetPos(pos);
		if(bSwap[0] == true || bSwap[1] == true)
			m_resizeMode = InvertResizeAxis(m_resizeMode, bSwap[0], bSwap[1]);
	}
	else if(math::is_flag_set(m_stateFlags, StateFlags::Dragging) == true && m_hMoveRect.IsValid()) {
		Vector2i mousePos;
		WGUI::GetInstance().GetMousePos(mousePos.x, mousePos.y, GetRootWindow());
		auto npos = mousePos - m_dragCursorOffset;
		SetAbsolutePos(npos);

		auto curPos = GetPos();
		curPos.x = math::clamp(curPos.x, m_minDrag.x, m_maxDrag.x);
		curPos.y = math::clamp(curPos.y, m_minDrag.y, m_maxDrag.y);
		SetPos(curPos);
		curPos = GetPos();

		auto prevPos = curPos;
		CallCallbacks<void, std::reference_wrapper<Vector2i>, bool>("TranslateTransformPosition", std::ref(curPos), true);
		SetPos(curPos);
	}
	auto bDragging = math::is_flag_set(m_stateFlags, StateFlags::Dragging);
	if(bDragging == false && math::is_flag_set(m_stateFlags, StateFlags::WasDragged) == false)
		return;
	auto itSnapTarget = std::find_if(m_snapTargets.begin(), m_snapTargets.end(), [](const WIHandle &hSnapTarget) {
		if(hSnapTarget.IsValid() == false)
			return false;
		auto *pTriggerArea = const_cast<WISnapArea *>(static_cast<const WISnapArea *>(hSnapTarget.get()))->GetTriggerArea();
		return pTriggerArea && pTriggerArea->MouseInBounds();
	});
	util::ScopeGuard sgSnapGhost = [this, bDragging]() {
		if(bDragging)
			return;
		math::set_flag(m_stateFlags, StateFlags::WasDragged, false);
		if(m_snapGhost.IsValid() && m_snapGhost->IsVisible())
			SnapToTarget(*m_snapGhost.get());
		DestroySnapTargetGhost();
	};
	if(itSnapTarget == m_snapTargets.end()) {
		if(m_snapGhost.IsValid())
			m_snapGhost->SetVisible(false);
		return;
	}
	auto &snapTarget = static_cast<WISnapArea &>(*itSnapTarget->get());
	InitializeSnapTargetGhost(snapTarget);
	if(m_snapGhost.IsValid() == false)
		return;
	m_snapGhost->SetVisible(true);
	auto pos = snapTarget.GetAbsolutePos();
	auto sz = snapTarget.GetSize();
	m_snapGhost->SetPos(pos);
	m_snapGhost->SetSize(sz);
}

void pragma::gui::types::WITransformable::SnapToTarget(WIBase &el)
{
	SetAbsolutePos(el.GetPos());
	SetSize(el.GetSize());
}

void pragma::gui::types::WITransformable::InitializeSnapTargetGhost(WISnapArea &snapArea)
{
	if(m_snapGhost.IsValid())
		return;
	auto &wgui = WGUI::GetInstance();
	auto *pSnapGhost = wgui.Create<WIRect>();
	m_snapGhost = pSnapGhost->GetHandle();
	pSnapGhost->SetColor(Color {255, 255, 255, 100});

	auto *pOutline = wgui.Create<WIOutlinedRect>(pSnapGhost);
	pOutline->SetAutoAlignToParent(true);
	pOutline->SetOutlineWidth(2);
	pOutline->SetColor(colors::Black);

	RemoveOnRemoval(pSnapGhost);
	pSnapGhost->SetZPos(10'000);
}
void pragma::gui::types::WITransformable::DestroySnapTargetGhost()
{
	if(m_snapGhost.IsValid())
		m_snapGhost->Remove();
}
void pragma::gui::types::WITransformable::SetSize(int x, int y)
{
	auto oldSize = GetSize();
	WIBase::SetSize(x, y);
	if(math::is_flag_set(m_stateFlags, StateFlags::Dragging) == true) {
		auto sz = GetSize();
		Vector2 sc {1.f, 1.f};
		if(oldSize.x != 0)
			sc.x = sz.x / static_cast<float>(oldSize.x);
		if(oldSize.y != 0)
			sc.y = sz.y / static_cast<float>(oldSize.y);
		m_dragCursorOffset.x *= static_cast<int32_t>(sc.x);
		m_dragCursorOffset.y *= static_cast<int32_t>(sc.y);
	}
	Vector2i minSize = m_minSize;
	if(x < minSize.x || y < minSize.y) {
		if(x < minSize.x)
			minSize.x = math::max(x, -1);
		if(y < minSize.y)
			minSize.y = math::max(y, -1);
		SetMinSize(minSize);
	}
	Vector2i maxSize = m_maxSize;
	if((x > maxSize.x && maxSize.x != -1) || (y > maxSize.y && maxSize.y != -1)) {
		if(x > maxSize.x && maxSize.x != -1)
			maxSize.x = x;
		if(y > maxSize.y && maxSize.y != -1)
			maxSize.y = y;
		SetMaxSize(maxSize);
	}
	UpdateResizeRect();
}
bool pragma::gui::types::WITransformable::IsBeingDragged() const { return math::is_flag_set(m_stateFlags, StateFlags::Dragging); }
bool pragma::gui::types::WITransformable::IsBeingResized() const { return math::is_flag_set(m_stateFlags, StateFlags::Resizing); }
void pragma::gui::types::WITransformable::SetDragBounds(const Vector2i &min, const Vector2i &max)
{
	m_minDrag = min;
	m_maxDrag = max;
}
std::pair<Vector2i, Vector2i> pragma::gui::types::WITransformable::GetDragBounds() const { return {m_minDrag, m_maxDrag}; }
void pragma::gui::types::WITransformable::SetPos(int x, int y)
{
	WIBase::SetPos(x, y);
	UpdateResizeRectPos();
}

void pragma::gui::types::WITransformable::SetDraggable(bool b)
{
	math::set_flag(m_stateFlags, StateFlags::Draggable, b);
	if(b == false)
		EndDrag();
	if(m_hMoveRect.IsValid())
		m_hMoveRect->SetMouseInputEnabled(b);
	auto bMouseInput = (math::is_flag_set(m_stateFlags, StateFlags::Resizable) || math::is_flag_set(m_stateFlags, StateFlags::Draggable)) ? true : false;
	SetMouseInputEnabled(bMouseInput);
	SetMouseMovementCheckEnabled(bMouseInput);
}
void pragma::gui::types::WITransformable::UpdateResizeRectPos()
{
	if(m_hResizeRect.IsValid() == false)
		return;
	auto *pRect = m_hResizeRect.get();
	auto &pos = GetPos();
	pRect->SetPos(pos.x - WIFRAME_RESIZE_OFFSET_BORDER, pos.y - WIFRAME_RESIZE_OFFSET_BORDER);
}
void pragma::gui::types::WITransformable::UpdateResizeRect()
{
	if(m_hResizeRect.IsValid() == false)
		return;
	auto *pRect = m_hResizeRect.get();
	auto &sz = GetSize();
	pRect->SetSize(sz.x + WIFRAME_RESIZE_OFFSET_BORDER * 2, sz.y + WIFRAME_RESIZE_OFFSET_BORDER * 2);

	UpdateResizeRectPos();
}
void pragma::gui::types::WITransformable::SetParent(WIBase *base, std::optional<uint32_t> childIndex)
{
	if(m_hResizeRect.IsValid() == true)
		m_hResizeRect->SetParent(base); // Resize element parent has to be set before us!
	WIBase::SetParent(base, childIndex);
}
void pragma::gui::types::WITransformable::SetZPos(int zpos)
{
	if(m_hResizeRect.IsValid())
		m_hResizeRect->SetZPos(zpos);
	WIBase::SetZPos(zpos);
}
void pragma::gui::types::WITransformable::SetVisible(bool b)
{
	WIBase::SetVisible(b);
	if(m_hResizeRect.IsValid())
		m_hResizeRect->SetVisible(b);
}
void pragma::gui::types::WITransformable::SetResizable(bool b)
{
	math::set_flag(m_stateFlags, StateFlags::Resizable, b);
	if(b == false)
		EndResizing();
	auto bMouseInput = (math::is_flag_set(m_stateFlags, StateFlags::Resizable) || math::is_flag_set(m_stateFlags, StateFlags::Draggable)) ? true : false;
	SetMouseInputEnabled(bMouseInput);
	SetMouseMovementCheckEnabled(bMouseInput);

	if(b == false) {
		if(m_hResizeRect.IsValid())
			m_hResizeRect->Remove();
		return;
	}
	auto *pParent = GetParent();
	SetParent(nullptr);

	auto hThis = GetHandle();
	auto *resizeRect = WGUI::GetInstance().Create<WIBase>();
	resizeRect->SetMouseInputEnabled(true);
	resizeRect->SetVisible(IsVisible());
	resizeRect->SetZPos(GetZPos());
	resizeRect->AddCallback("OnMouseEvent",
	  FunctionCallback<util::EventReply, platform::MouseButton, platform::KeyState, platform::Modifier>::CreateWithOptionalReturn([hThis](util::EventReply *reply, platform::MouseButton button, platform::KeyState state, platform::Modifier mods) mutable -> CallbackReturnType {
		  if(hThis.IsValid() == false) {
			  *reply = util::EventReply::Handled;
			  return CallbackReturnType::HasReturnValue;
		  }
		  *reply = hThis.get()->MouseCallback(button, state, mods);
		  return CallbackReturnType::HasReturnValue;
	  }));
	RemoveOnRemoval(resizeRect);
	m_hResizeRect = resizeRect->GetHandle();
	SetParent(pParent);
	UpdateResizeRect();
}
bool pragma::gui::types::WITransformable::IsDraggable() { return math::is_flag_set(m_stateFlags, StateFlags::Draggable); }
bool pragma::gui::types::WITransformable::IsResizable() { return math::is_flag_set(m_stateFlags, StateFlags::Resizable); }
