#ifndef __RENDERER_HPP__
#define __RENDERER_HPP__

#include <include/cef_render_handler.h>
#include <prosper_texture.hpp>
#include <wgui/wihandle.h>

namespace pragma {class BrowserProcess;};
class WebRenderHandler
	: public CefRenderHandler
{
public:
	WebRenderHandler(pragma::BrowserProcess *process,const std::shared_ptr<prosper::Texture> &texture,WIBase *guiElement);
	virtual ~WebRenderHandler() override;
	virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,CefRect &rect) override;
	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;
	virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,int viewX,int viewY,int &screenX,int &screenY) override;
	virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser,CefScreenInfo &screen_info) override;
	virtual void OnPopupShow(CefRefPtr<CefBrowser> browser,bool show) override;
	virtual void OnPopupSize(CefRefPtr<CefBrowser> browser,const CefRect &rect) override;
	virtual void OnPaint(CefRefPtr<CefBrowser> browser,PaintElementType type,const RectList &dirtyRects,const void *buffer,int width, int height) override;
	virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,CefCursorHandle cursor,CursorType type,const CefCursorInfo &custom_cursor_info) override;
	virtual bool StartDragging(CefRefPtr<CefBrowser> browser,CefRefPtr<CefDragData> drag_data,DragOperationsMask allowed_ops,int x,int y) override;
	virtual void UpdateDragCursor(CefRefPtr<CefBrowser> browser,DragOperation operation) override;
	virtual void OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser,double x,double y) override;
	virtual void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,const CefRange &selected_range,const RectList &character_bounds) override;
	IMPLEMENT_REFCOUNTING(WebRenderHandler);
private:
	void *m_dataPtr = nullptr;
	std::shared_ptr<prosper::Texture> m_texture = nullptr;
	pragma::BrowserProcess *m_process = nullptr;
	WIHandle m_guiBase = {};
};

#endif
