#include "stdafx_client.h"
#include "pragma/gui/witransformable.h"
#include <wgui/types/witext.h>
#include <wgui/types/wibutton.h>
#include <mathutil/umath.h>

extern DLLCENGINE CEngine *c_engine;

LINK_WGUI_TO_CLASS(WITransformable,WITransformable);

#define WIFRAME_DRAG_OFFSET_BORDER 5
#define WIFRAME_RESIZE_OFFSET_BORDER 5

WITransformable::WITransformable()
	: WIBase()
{
	RegisterCallback<void>("OnClose");
	RegisterCallback<void,std::reference_wrapper<Vector2i>,bool>("TranslateTransformPosition");
}
WITransformable::~WITransformable()
{
	if(m_resizeMode != ResizeMode::none)
		WGUI::GetInstance().SetCursor(GLFW::Cursor::Shape::Arrow);
}
void WITransformable::Update() {WIBase::Update(); Resize();}
void WITransformable::SetMinWidth(int w) {SetMinSize(Vector2i(w,m_minSize.y));}
void WITransformable::SetMinHeight(int h) {SetMinSize(Vector2i(m_minSize.x,h));}
void WITransformable::SetMinSize(int w,int h) {SetMinSize(Vector2i(w,h));}
void WITransformable::SetMinSize(Vector2i size)
{
	m_minSize = size;
	if(size.x < 0)
		size.x = 0;
	if(size.y < 0)
		size.y = 0;
	Vector2i sz = GetSize();
	if(sz.x < size.x || sz.y < size.y)
	{
		if(sz.x < size.x)
			sz.x = size.x;
		if(sz.y < size.y)
			sz.y = size.y;
		SetSize(sz.x,sz.y);
	}
}
int WITransformable::GetMinWidth() {return m_minSize.x;}
int WITransformable::GetMinHeight() {return m_minSize.y;}
const Vector2i &WITransformable::GetMinSize() const {return m_minSize;}
void WITransformable::SetMaxWidth(int w) {SetMaxSize(Vector2i(w,m_maxSize.x));}
void WITransformable::SetMaxHeight(int h) {SetMaxSize(Vector2i(m_maxSize.x,h));}
void WITransformable::SetMaxSize(int w,int h) {SetMaxSize(Vector2i(w,h));}
void WITransformable::SetMaxSize(Vector2i size)
{
	if(size.x < -1)
		size.x = -1;
	if(size.y < -1)
		size.y = -1;
	m_maxSize = size;
	Vector2i sz = GetSize();
	if(sz.x > size.x || sz.y > size.y)
	{
		if(sz.x > size.x)
			sz.x = size.x;
		if(sz.y > size.y)
			sz.y = size.y;
		SetSize(sz.x,sz.y);
	}
}
int WITransformable::GetMaxWidth() {return m_maxSize.x;}
int WITransformable::GetMaxHeight() {return m_maxSize.y;}
const Vector2i &WITransformable::GetMaxSize() const {return m_maxSize;}
void WITransformable::OnTitleBarMouseEvent(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods)
{
	MouseCallback(button,state,mods);
	if(umath::is_flag_set(m_stateFlags,StateFlags::Draggable) == false)
		return;
	if(button == GLFW::MouseButton::Left)
	{
		if(state == GLFW::KeyState::Press)
		{
			if(m_resizeMode == ResizeMode::none)
				StartDrag();
		}
		else if(state == GLFW::KeyState::Release)
			EndDrag();
	}
}
Vector2i WITransformable::GetConfinedMousePos()
{
	Vector2i pos;
	WGUI::GetInstance().GetMousePos(pos.x,pos.y);
	if(pos.x < WIFRAME_DRAG_OFFSET_BORDER)
		pos.x = WIFRAME_DRAG_OFFSET_BORDER;
	if(pos.y < WIFRAME_DRAG_OFFSET_BORDER)
		pos.y = WIFRAME_DRAG_OFFSET_BORDER;
	auto wViewport = c_engine->GetWindowWidth();
	auto hViewport = c_engine->GetWindowHeight();
	if(pos.x > static_cast<int32_t>(wViewport -WIFRAME_DRAG_OFFSET_BORDER))
		pos.x = wViewport -WIFRAME_DRAG_OFFSET_BORDER;
	if(pos.y > static_cast<int32_t>(hViewport -WIFRAME_DRAG_OFFSET_BORDER))
		pos.y = hViewport -WIFRAME_DRAG_OFFSET_BORDER;
	return pos;
}
void WITransformable::StartDrag()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::Dragging) == true)
		return;
	GetMousePos(&m_dragCursorOffset.x,&m_dragCursorOffset.y);
	umath::set_flag(m_stateFlags,StateFlags::Dragging,true);
}
void WITransformable::OnVisibilityChanged(bool bVisible)
{
	if(bVisible == false)
	{
		if(m_resizeMode != ResizeMode::none)
		{
			EndResizing();
			WGUI::GetInstance().SetCursor(GLFW::Cursor::Shape::Arrow);
		}
		EndDrag();
	}
}
void WITransformable::EndDrag()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::Dragging) == false)
		return;
	umath::set_flag(m_stateFlags,StateFlags::Dragging,false);
}
void WITransformable::StartResizing()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::Resizing) == true || IsResizable() == false)
		return;
	umath::set_flag(m_stateFlags,StateFlags::Resizing,true);
	m_resizeLastPos = GetConfinedMousePos();
}
void WITransformable::EndResizing()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::Resizing) == false)
		return;
	umath::set_flag(m_stateFlags,StateFlags::Resizing,false);
	SetResizeMode(ResizeMode::none);
}
void WITransformable::SetResizeMode(ResizeMode mode)
{
	if(m_resizeMode == mode || IsResizable() == false)
		return;
	auto cursor = GLFW::Cursor::Shape::Arrow;
	switch(mode)
	{
	case ResizeMode::none:
		break;
	case ResizeMode::ns:
	case ResizeMode::sn:
		cursor = GLFW::Cursor::Shape::VResize;
		break;
	case ResizeMode::ew:
	case ResizeMode::we:
		cursor = GLFW::Cursor::Shape::HResize;
		break;
	default:
		cursor = GLFW::Cursor::Shape::Crosshair;
		break;
	};
	m_resizeMode = mode;
	WGUI::GetInstance().SetCursor(cursor);
}
void WITransformable::OnCloseButtonPressed()
{
	Close();
}
void WITransformable::Close()
{
	//SetVisible(false);
	RemoveSafely();
	CallCallbacks<void>("OnClose");
}
WIBase *WITransformable::GetDragArea() const {return m_hMoveRect.get();}
void WITransformable::Initialize()
{
	WIBase::Initialize();

	SetDraggable(true);
	SetResizable(true);
	m_hMoveRect = CreateChild<WIBase>();
	auto *pMoveRect = m_hMoveRect.get();
	pMoveRect->AddStyleClass("move_rect");
	pMoveRect->SetMouseInputEnabled(umath::is_flag_set(m_stateFlags,StateFlags::Draggable));
	pMoveRect->AddCallback("OnMouseEvent",FunctionCallback<void,GLFW::MouseButton,GLFW::KeyState,GLFW::Modifier>::Create(
		std::bind(&WITransformable::OnTitleBarMouseEvent,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)
	));
}
void WITransformable::MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods)
{
	WIBase::MouseCallback(button,state,mods);
	if(button == GLFW::MouseButton::Left)
	{
		if(state == GLFW::KeyState::Press)
		{
			if(m_resizeMode != ResizeMode::none)
				StartResizing();
		}
		else if(state == GLFW::KeyState::Release)
			EndResizing();
	}
}
void WITransformable::OnCursorMoved(int x,int y)
{
	WIBase::OnCursorMoved(x,y);
	if(umath::is_flag_set(m_stateFlags,StateFlags::Resizing) == true || umath::is_flag_set(m_stateFlags,StateFlags::Dragging) == true)
		return;
	if(x < -WIFRAME_RESIZE_OFFSET_BORDER || y < -WIFRAME_RESIZE_OFFSET_BORDER)
	{
		SetResizeMode(ResizeMode::none);
		return;
	}
	const Vector2i &size = GetSize();
	if(x > size.x +WIFRAME_RESIZE_OFFSET_BORDER || y > size.y +WIFRAME_RESIZE_OFFSET_BORDER)
	{
		SetResizeMode(ResizeMode::none);
		return;
	}
	if(y <= WIFRAME_RESIZE_OFFSET_BORDER)
	{
		if(x <= WIFRAME_RESIZE_OFFSET_BORDER)
			SetResizeMode(ResizeMode::nwse);
		else if(x >= size.x -WIFRAME_RESIZE_OFFSET_BORDER)
			SetResizeMode(ResizeMode::nesw);
		else
			SetResizeMode(ResizeMode::ns);
	}
	else if(y >= size.y -WIFRAME_RESIZE_OFFSET_BORDER)
	{
		if(x <= WIFRAME_RESIZE_OFFSET_BORDER)
			SetResizeMode(ResizeMode::swne);
		else if(x >= size.x -WIFRAME_RESIZE_OFFSET_BORDER)
			SetResizeMode(ResizeMode::senw);
		else
			SetResizeMode(ResizeMode::sn);
	}
	else if(x <= WIFRAME_RESIZE_OFFSET_BORDER)
		SetResizeMode(ResizeMode::ew);
	else if(x >= size.x -WIFRAME_RESIZE_OFFSET_BORDER)
		SetResizeMode(ResizeMode::we);
	else
		SetResizeMode(ResizeMode::none);
}
WITransformable::ResizeMode WITransformable::InvertResizeAxis(ResizeMode mode,bool bXAxis,bool bYAxis) const
{
	if(bXAxis == false && bYAxis == false)
		return mode;
	switch(mode)
	{
		case ResizeMode::ns:
			return (bYAxis == true) ? ResizeMode::sn : mode;
		case ResizeMode::sn:
			return (bYAxis == true) ? ResizeMode::ns : mode;
		case ResizeMode::we:
			return (bXAxis == true) ? ResizeMode::ew : mode;
		case ResizeMode::ew:
			return (bXAxis == true) ? ResizeMode::we : mode;
		case ResizeMode::nwse:
			return (bXAxis == true && bYAxis == true) ? ResizeMode::senw :
				(bYAxis == true) ? ResizeMode::swne : ResizeMode::nesw;
		case ResizeMode::nesw:
			return (bXAxis == true && bYAxis == true) ? ResizeMode::swne :
				(bYAxis == true) ? ResizeMode::senw : ResizeMode::nwse;
		case ResizeMode::senw:
			return (bXAxis == true && bYAxis == true) ? ResizeMode::nwse:
				(bYAxis == true) ? ResizeMode::nesw : ResizeMode::swne;
		case ResizeMode::swne:
			return (bXAxis == true && bYAxis == true) ? ResizeMode::nesw:
				(bYAxis == true) ? ResizeMode::nwse : ResizeMode::senw;
		default:
			return ResizeMode::none;
	};
}
void WITransformable::Think()
{
	WIBase::Think();
	if(umath::is_flag_set(m_stateFlags,StateFlags::Resizing) == true)
	{
		Vector2i cursorPos = {};
		GetMousePos(&cursorPos.x,&cursorPos.y);

		auto size = GetSize();
		auto pos = GetPos();
		auto bChangeSize = false;
		auto bChangePos = false;
		std::array<bool,2> bSwap = {false,false};
		std::array<bool,2> bResize = {
			(m_resizeMode == ResizeMode::we || m_resizeMode == ResizeMode::nesw || m_resizeMode == ResizeMode::senw) ? true : false,
			(m_resizeMode == ResizeMode::swne || m_resizeMode == ResizeMode::sn || m_resizeMode == ResizeMode::senw) ? true : false
		};
		std::array<bool,2> bReposition = {
			(m_resizeMode == ResizeMode::ew || m_resizeMode == ResizeMode::swne || m_resizeMode == ResizeMode::nwse) ? true : false,
			(m_resizeMode == ResizeMode::nesw || m_resizeMode == ResizeMode::ns || m_resizeMode == ResizeMode::nwse) ? true : false
		};
		std::array<bool,2> bKeepPos = {true,true};
		std::array<bool,2> bKeepSize = {true,true};
		for(glm::length_t i=0;i<bResize.size();++i)
		{
			if(bResize[i] == true)
			{
				size[i] = cursorPos[i];
				bChangeSize = true;
				bKeepSize[i] = false;
			}
		}

		size += pos;
		CallCallbacks<void,std::reference_wrapper<Vector2i>,bool>("TranslateTransformPosition",std::ref(size),false);
		size -= pos;

		auto &minSize = GetMinSize();
		auto &maxSize = GetMaxSize();
		for(glm::length_t i=decltype(bKeepSize.size()){0};i<bKeepSize.size();++i)
		{
			if(bKeepSize[i] == true)
				size[i] = GetSize()[i];
			else
			{
				if(minSize[i] >= 0)
					size[i] = umath::max(size[i],minSize[i]);
				if(maxSize[i] >= 0)
					size[i] = umath::min(size[i],maxSize[i]);
				if(size[i] < 0)
				{
					bSwap[i] = true;
					size[i] = 0;
				}
			}
		}

		for(glm::length_t i=decltype(bReposition.size()){0};i<bReposition.size();++i)
		{
			if(bReposition[i] == true)
			{
				pos[i] += cursorPos[i];
				bChangePos = true;
				bKeepPos[i] = false;
			}
		}
		CallCallbacks<void,std::reference_wrapper<Vector2i>,bool>("TranslateTransformPosition",std::ref(pos),false);
		for(glm::length_t i=decltype(bKeepPos.size()){0};i<bKeepPos.size();++i)
		{
			if(bKeepPos[i] == true)
				pos[i] = GetPos()[i];
			else
			{
				auto newSize = size[i] -(pos[i] -GetPos()[i]);
				if(minSize[i] >= 0 && newSize < minSize[i])
					pos[i] -= minSize[i] -newSize;
				if(maxSize[i] >= 0 && newSize > maxSize[i])
					pos[i] += maxSize[i] -newSize;

				if(pos[i] -GetPos()[i] > size[i])
				{
					bSwap[i] = true;
					pos[i] = GetPos()[i] +size[i];
				}
			}
		}
		for(glm::length_t i=decltype(bReposition.size()){0};i<bReposition.size();++i)
		{
			if(bReposition[i] == true)
			{
				size[i] -= pos[i] -GetPos()[i];
				bChangeSize = true;
			}
		}

		if(bChangeSize == true || bChangePos == true)
			SetSize(size);
		if(bChangePos == true)
			SetPos(pos);
		if(bSwap[0] == true || bSwap[1] == true)
			m_resizeMode = InvertResizeAxis(m_resizeMode,bSwap[0],bSwap[1]);
	}
	else if(umath::is_flag_set(m_stateFlags,StateFlags::Dragging) == true && m_hMoveRect.IsValid())
	{
		Vector2i mousePos;
		WGUI::GetInstance().GetMousePos(mousePos.x,mousePos.y);
		auto npos = mousePos -m_dragCursorOffset;
		SetAbsolutePos(npos);

		auto curPos = GetPos();
		curPos.x = umath::clamp(curPos.x,m_minDrag.x,m_maxDrag.x);
		curPos.y = umath::clamp(curPos.y,m_minDrag.y,m_maxDrag.y);
		SetPos(curPos);
		curPos = GetPos();

		auto prevPos = curPos;
		CallCallbacks<void,std::reference_wrapper<Vector2i>,bool>("TranslateTransformPosition",std::ref(curPos),true);
		SetPos(curPos);
	}
}
void WITransformable::SetSize(int x,int y)
{
	auto oldSize = GetSize();
	WIBase::SetSize(x,y);
	if(umath::is_flag_set(m_stateFlags,StateFlags::Dragging) == true)
	{
		auto sz = GetSize();
		Vector2 sc {1.f,1.f};
		if(oldSize.x != 0)
			sc.x = sz.x /static_cast<float>(oldSize.x);
		if(oldSize.y != 0)
			sc.y = sz.y /static_cast<float>(oldSize.y);
		m_dragCursorOffset.x *= static_cast<int32_t>(sc.x);
		m_dragCursorOffset.y *= static_cast<int32_t>(sc.y);
	}
	Vector2i minSize = m_minSize;
	if(x < minSize.x || y < minSize.y)
	{
		if(x < minSize.x)
			minSize.x = umath::max(x,-1);
		if(y < minSize.y)
			minSize.y = umath::max(y,-1);
		SetMinSize(minSize);
	}
	Vector2i maxSize = m_maxSize;
	if((x > maxSize.x && maxSize.x != -1) || (y > maxSize.y && maxSize.y != -1))
	{
		if(x > maxSize.x && maxSize.x != -1)
			maxSize.x = x;
		if(y > maxSize.y && maxSize.y != -1)
			maxSize.y = y;
		SetMaxSize(maxSize);
	}
	UpdateResizeRect();
}
bool WITransformable::IsBeingDragged() const {return umath::is_flag_set(m_stateFlags,StateFlags::Dragging);}
bool WITransformable::IsBeingResized() const {return umath::is_flag_set(m_stateFlags,StateFlags::Resizing);}
void WITransformable::SetDragBounds(const Vector2i &min,const Vector2i &max)
{
	m_minDrag = min;
	m_maxDrag = max;
}
std::pair<Vector2i,Vector2i> WITransformable::GetDragBounds() const
{
	return {m_minDrag,m_maxDrag};
}
void WITransformable::SetPos(int x,int y)
{
	WIBase::SetPos(x,y);
	UpdateResizeRectPos();
}

void WITransformable::SetDraggable(bool b)
{
	umath::set_flag(m_stateFlags,StateFlags::Draggable,b);
	if(b == false)
		EndDrag();
	if(m_hMoveRect.IsValid())
		m_hMoveRect->SetMouseInputEnabled(b);
	auto bMouseInput = (umath::is_flag_set(m_stateFlags,StateFlags::Resizable) || umath::is_flag_set(m_stateFlags,StateFlags::Draggable)) ? true : false;
	SetMouseInputEnabled(bMouseInput);
	SetMouseMovementCheckEnabled(bMouseInput);
}
void WITransformable::UpdateResizeRectPos()
{
	if(m_hResizeRect.IsValid() == false)
		return;
	auto *pRect = m_hResizeRect.get();
	auto &pos = GetPos();
	pRect->SetPos(pos.x -WIFRAME_RESIZE_OFFSET_BORDER,pos.y -WIFRAME_RESIZE_OFFSET_BORDER);
}
void WITransformable::UpdateResizeRect()
{
	if(m_hResizeRect.IsValid() == false)
		return;
	auto *pRect = m_hResizeRect.get();
	auto &sz = GetSize();
	pRect->SetSize(sz.x +WIFRAME_RESIZE_OFFSET_BORDER *2,sz.y +WIFRAME_RESIZE_OFFSET_BORDER *2);

	UpdateResizeRectPos();
}
void WITransformable::SetParent(WIBase *base)
{
	if(m_hResizeRect.IsValid() == true)
		m_hResizeRect->SetParent(base); // Resize element parent has to be set before us!
	WIBase::SetParent(base);
}
void WITransformable::SetZPos(int zpos)
{
	if(m_hResizeRect.IsValid())
		m_hResizeRect->SetZPos(zpos);
	WIBase::SetZPos(zpos);
}
void WITransformable::SetVisible(bool b)
{
	WIBase::SetVisible(b);
	if(m_hResizeRect.IsValid())
		m_hResizeRect->SetVisible(b);
}
void WITransformable::SetResizable(bool b)
{
	umath::set_flag(m_stateFlags,StateFlags::Resizable,b);
	if(b == false)
		EndResizing();
	auto bMouseInput = (umath::is_flag_set(m_stateFlags,StateFlags::Resizable) || umath::is_flag_set(m_stateFlags,StateFlags::Draggable)) ? true : false;
	SetMouseInputEnabled(bMouseInput);
	SetMouseMovementCheckEnabled(bMouseInput);

	if(b == false)
	{
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
	resizeRect->AddCallback("OnMouseEvent",FunctionCallback<void,GLFW::MouseButton,GLFW::KeyState,GLFW::Modifier>::Create([hThis](GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) {
		if(hThis.IsValid() == false)
			return;
		hThis.get()->MouseCallback(button,state,mods);
	}));
	RemoveOnRemoval(resizeRect);
	m_hResizeRect = resizeRect->GetHandle();
	SetParent(pParent);
	UpdateResizeRect();
}
bool WITransformable::IsDraggable() {return umath::is_flag_set(m_stateFlags,StateFlags::Draggable);}
bool WITransformable::IsResizable() {return umath::is_flag_set(m_stateFlags,StateFlags::Resizable);}
