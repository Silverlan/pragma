#ifndef __WITRANSFORMABLE_H__
#define __WITRANSFORMABLE_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

class DLLCLIENT WITransformable
	: public WIBase
{
protected:
	enum class ResizeMode
	{
		none = -1,
		ew = 1,
		we = 2,
		ns = 3,
		sn = 4,
		nwse = 5,
		nesw = 6,
		senw = 7,
		swne = 8
	};
	WIHandle m_hMoveRect = {};
	WIHandle m_hResizeRect = {};
	bool m_bDraggable = false;
	bool m_bResizable = false;
	bool m_bDragging = false;
	ResizeMode m_resizeMode = ResizeMode::none;
	bool m_bResizing = false;
	Vector2i m_resizeLastPos = {};
	Vector2i m_dragCursorOffset = {};
	void StartDrag();
	void EndDrag();
	void SetResizeMode(ResizeMode mode);
	void StartResizing();
	void EndResizing();
	ResizeMode InvertResizeAxis(ResizeMode mode,bool bXAxis,bool bYAxis) const;
	Vector2i m_minSize = {};
	Vector2i m_maxSize = {-1,-1};
	virtual void OnVisibilityChanged(bool bVisible) override;
	Vector2i GetConfinedMousePos();
	void OnTitleBarMouseEvent(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods);
	void OnCloseButtonPressed();
	void UpdateResizeRect();
	void UpdateResizeRectPos();
public:
	WITransformable();
	virtual ~WITransformable() override;
	virtual void Initialize() override;
	using WIBase::SetSize;
	virtual void SetSize(int x,int y) override;
	using WIBase::SetPos;
	virtual void SetPos(int x,int y) override;
	virtual void SetParent(WIBase *base) override;
	virtual void SetZPos(int zpos) override;
	virtual void SetVisible(bool b) override;
	WIBase *GetDragArea() const;
	void SetDraggable(bool b);
	void SetResizable(bool b);
	bool IsDraggable();
	bool IsResizable();
	virtual void Think() override;
	virtual void MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) override;
	virtual void OnCursorMoved(int x,int y) override;
	void SetMinWidth(int w);
	void SetMinHeight(int h);
	void SetMinSize(int w,int h);
	void SetMinSize(Vector2i size);
	int GetMinWidth();
	int GetMinHeight();
	const Vector2i &GetMinSize() const;
	void SetMaxWidth(int w);
	void SetMaxHeight(int h);
	void SetMaxSize(int w,int h);
	void SetMaxSize(Vector2i size);
	int GetMaxWidth();
	int GetMaxHeight();
	const Vector2i &GetMaxSize() const;
	void Close();
	virtual void Update() override;
};

#endif