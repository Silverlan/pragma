#include "renderer.hpp"
#include "browser_process.hpp"
#include "browser_render_process_handler.hpp"
#include "browser_load_handler.hpp"
#include "browserclient.hpp"
#include <prosper_context.hpp>
#include <wgui/wibase.h>
#include <wrappers/memory_block.h>

WebRenderHandler::WebRenderHandler(pragma::BrowserProcess *process,const std::shared_ptr<prosper::Texture> &texture,WIBase *guiElement)
	: m_process(process),m_texture(texture),m_guiBase(guiElement->GetHandle())
{
	auto &memBlock = *m_texture->GetImage()->GetAnvilImage().get_memory_block();
	m_texture->GetImage()->GetAnvilImage().get_memory_block()->map(0ull,memBlock.get_create_info_ptr()->get_size(),&m_dataPtr);
}
WebRenderHandler::~WebRenderHandler()
{
	m_texture->GetImage()->GetAnvilImage().get_memory_block()->unmap();
}
bool WebRenderHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser,CefRect &rect)
{
	auto &context = WGUI::GetInstance().GetContext();
	auto &window = context.GetWindow();
	auto windowPos = window.GetPos();
	auto windowSize = window.GetSize();
	rect = CefRect(windowPos.x,windowPos.y,windowSize.x,windowSize.y);
	return true;
	//return false;
}
bool WebRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
{
	auto extents = m_texture->GetImage()->GetExtents();
	rect = CefRect(0,0,extents.width,extents.height);
	if(m_guiBase.IsValid())
	{
		auto &context = WGUI::GetInstance().GetContext();
		auto &window = context.GetWindow();
		auto windowPos = window.GetPos();
		auto pos = m_guiBase->GetAbsolutePos();
		rect.x = windowPos.x +pos.x;
		rect.y = windowPos.y +pos.y;
	}
	return true;
	//return false;
}
bool WebRenderHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser,int viewX,int viewY,int &screenX,int &screenY)
{
	auto &context = WGUI::GetInstance().GetContext();
	auto &window = context.GetWindow();
	auto windowPos = window.GetPos();
	screenX = windowPos.x;
	screenY = windowPos.y;
	if(m_guiBase.IsValid())
	{
		auto pos = m_guiBase->GetAbsolutePos();
		screenX += pos.x;
		screenY += pos.y;
	}
	screenX += viewX;
	screenY += viewY;
	return true;
}
bool WebRenderHandler::GetScreenInfo(CefRefPtr<CefBrowser> browser,CefScreenInfo &screen_info)
{
	return false;
}
void WebRenderHandler::OnPopupShow(CefRefPtr<CefBrowser> browser,bool show)
{

}
void WebRenderHandler::OnPopupSize(CefRefPtr<CefBrowser> browser,const CefRect &rect)
{

}
void WebRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser,PaintElementType type,const RectList &dirtyRects,const void *buffer,int width,int height)
{
	if(m_texture == nullptr)
		return;
	auto host = browser->GetHost();
	auto *client = static_cast<WebBrowserClient*>(host->GetClient().get());
	auto bLoaded = (client != nullptr && client->HasPageLoadingStarted()) ? true : false;
	if(bLoaded == false)
		return;
	auto &memBlock = *m_texture->GetImage()->GetAnvilImage().get_memory_block();
	memcpy(m_dataPtr,buffer,width *height *4u);
	/*
	vk::SubresourceLayout layout {};
	size_t pos = 0;
	for(auto y=decltype(height){0};y<height;++y)
	{
		auto *row = ptr;
		for(auto x=decltype(width){0};x<width;++x)
		{
			auto *pxSrc = reinterpret_cast<const uint8_t*>(buffer);
			auto *px = row;
			px[2] = pxSrc[pos++];
			px[1] = pxSrc[pos++];
			px[0] = pxSrc[pos++];
			px[3] = pxSrc[pos++] *alphaFactor;
			row += 4;
		}
		ptr += layout.rowPitch;
	}
	map->Flush();
	map = nullptr;*/
	//m_texture->GetImage()->MapMemory(static_cast<const uint8_t*>(buffer),true);

	//std::cout<<"OnPaint..."<<std::endl;
	/*Ogre::HardwarePixelBufferSharedPtr texBuf = m_renderTexture->getBuffer();
	texBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
	memcpy(texBuf->getCurrentLock().data, buffer, width*height*4);
	texBuf->unlock();*/
}
void WebRenderHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,CefCursorHandle cursor,CursorType type,const CefCursorInfo &custom_cursor_info)
{

}
bool WebRenderHandler::StartDragging(CefRefPtr<CefBrowser> browser,CefRefPtr<CefDragData> drag_data,DragOperationsMask allowed_ops,int x,int y)
{
	return false;
}
void WebRenderHandler::UpdateDragCursor(CefRefPtr<CefBrowser> browser,DragOperation operation)
{

}
void WebRenderHandler::OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser,double x,double y)
{

}
void WebRenderHandler::OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,const CefRange &selected_range,const RectList &character_bounds)
{

}
