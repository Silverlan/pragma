#include <prosper_context.hpp>
#include <prosper_util.hpp>
#include <prosper_sampler.hpp>
#include "wiweb.hpp"
#include "renderer.hpp"
#include "browserclient.hpp"
#include "browser_process.hpp"
#include "browser_render_process_handler.hpp"
#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>
#include <include/views/cef_browser_view.h>
#include <memory>
#include <pragma_module.hpp>

DEFINE_DERIVED_CHILD_HANDLE(PRAGMA_EXPORT,WI,WIBase,WITexturedShape,WIWeb,WIWeb);
LINK_WGUI_TO_CLASS(WIWeb,WIWeb);

decltype(WIWeb::s_bBrowserProcessInitialized) WIWeb::s_bBrowserProcessInitialized = false;
decltype(WIWeb::s_webCount) WIWeb::s_webCount = 0;

static CefRefPtr<pragma::BrowserProcess> s_browserProcess = nullptr;

WIWeb::WIWeb()
	: WITexturedRect()
{
	++s_webCount;
}

WIWeb::~WIWeb()
{
	if(--s_webCount == 0)
	{
		// TODO: This should probably not be cleaned up until the process is closing
		s_browserProcess = nullptr;
		s_bBrowserProcessInitialized = false;
	}
}

void WIWeb::InitializeHandle() {WIBase::InitializeHandle<WIWebHandle>();}

void WIWeb::Initialize()
{
	WITexturedRect::Initialize();
	SetKeyboardInputEnabled(true);
	SetMouseInputEnabled(true);
	SetMouseMovementCheckEnabled(true);
	SetScrollInputEnabled(true);
}

void WIWeb::Think()
{
	WIBase::Think();
	if(m_browserClient == nullptr)
		return;
	CefDoMessageLoopWork();
}

void WIWeb::Update()
{
	WITexturedRect::Update();
	InitializeChromiumBrowser();
}

void WIWeb::SetTransparentBackground(bool b) {m_bTransparentBackground = b;}

pragma::BrowserProcess *WIWeb::GetBrowserProcess()
{
	if(s_browserProcess == nullptr)
		return nullptr;
	return static_cast<pragma::BrowserProcess*>(s_browserProcess.get());
}

bool WIWeb::InitializeBrowserProcess()
{
	if(s_bBrowserProcessInitialized == true)
		return (s_browserProcess != nullptr) ? true : false;
	CefRefPtr<pragma::BrowserProcess> process = new pragma::BrowserProcess();
	CefMainArgs args {};
	auto result = CefExecuteProcess(args,process,nullptr); // ???
	if(result > 0)
		return false;
	s_browserProcess = process;

	CefSettings settings {};
	//settings.locale
	settings.windowless_rendering_enabled = true;
	settings.single_process = true; // TODO
	//settings.multi_threaded_message_loop = true;
	settings.no_sandbox = true;
	if(CefInitialize(args,settings,process,nullptr) == false)
	{
		s_browserProcess = nullptr;
		return false;
	}
	return true;
}

bool WIWeb::InitializeChromiumBrowser()
{
	auto &context = WGUI::GetInstance().GetContext();
	auto &dev = context.GetDevice();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
	imgCreateInfo.width = m_browserViewSize.x;
	imgCreateInfo.height = m_browserViewSize.y;
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
	imgCreateInfo.tiling = vk::ImageTiling::eLinear;
	imgCreateInfo.usage = vk::ImageUsageFlagBits::eSampled;
	auto img = prosper::util::create_image(dev,imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	imgViewCreateInfo.swizzleRed = vk::ComponentSwizzle::eB;
	imgViewCreateInfo.swizzleBlue = vk::ComponentSwizzle::eR;
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	m_texture = prosper::util::create_texture(dev,{},img,&imgViewCreateInfo,&samplerCreateInfo);
	if(m_texture == nullptr)
		return false;
	SetTexture(*m_texture);

	if(InitializeBrowserProcess() == false)
		return false;

	auto renderHandler = CefRefPtr<WebRenderHandler>(new WebRenderHandler(static_cast<pragma::BrowserProcess*>(s_browserProcess.get()),m_texture,this));
	m_webRenderer = std::make_shared<CefRefPtr<WebRenderHandler>>(renderHandler);

	//CefRefPtr<CefBrowser> browser;
	CefWindowInfo window_info;
	CefBrowserSettings browserSettings;
	//browserSettings.windowless_frame_rate = 60; // 30 is default

	auto hWindow = GetActiveWindow();
	window_info.SetAsWindowless(nullptr,m_bTransparentBackground);
	//window_info.windowless_rendering_enabled = true;
	//window_info.SetAsPopup(NULL, "cefsimple");

	//CefRefPtr<SimpleHandler> handler(new SimpleHandler(false));

	auto browserClient = CefRefPtr<WebBrowserClient>(new WebBrowserClient(renderHandler.get()));
	m_browserClient = std::make_shared<CefRefPtr<WebBrowserClient>>(browserClient);
	auto browser = CefBrowserHost::CreateBrowserSync(window_info,browserClient,"www.google.com",browserSettings,nullptr);
	m_browser = std::make_shared<CefRefPtr<CefBrowser>>(browser);
	//browser->GetMainFrame()->LoadURL
	//m_browserClient->GetKeyboardHandler()->OnKeyEvent
	//CefBrowserHost *x;
	//x->SendKeyEvent()

	// TODO: Release browser?
	
	//CefRunMessageLoop();
	//CefShutdown();

	return true;
}

void WIWeb::LoadURL(const std::string &url)
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->GetMainFrame()->LoadURL(url);
}

void WIWeb::SetBrowserViewSize(const Vector2i size) {m_browserViewSize = size;}
const Vector2i &WIWeb::GetBrowserViewSize() const {return m_browserViewSize;}

bool WIWeb::CanGoBack()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return false;
	return browser->CanGoBack();
}
bool WIWeb::CanGoForward()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return false;
	return browser->CanGoForward();
}
void WIWeb::GoBack()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->GoBack();
}
void WIWeb::GoForward()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->GoForward();
}
bool WIWeb::HasDocument()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return false;
	return browser->HasDocument();
}
bool WIWeb::IsLoading()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return false;
	return browser->IsLoading();
}
void WIWeb::Reload()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->Reload();
}
void WIWeb::ReloadIgnoreCache()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->ReloadIgnoreCache();
}
void WIWeb::StopLoad()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->StopLoad();
}

void WIWeb::Copy()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->GetMainFrame()->Copy();
}
void WIWeb::Cut()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->GetMainFrame()->Cut();
}
void WIWeb::Delete()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->GetMainFrame()->Delete();
}
void WIWeb::Paste()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->GetMainFrame()->Paste();
}
void WIWeb::Redo()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->GetMainFrame()->Redo();
}
void WIWeb::SelectAll()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->GetMainFrame()->SelectAll();
}
void WIWeb::Undo()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->GetMainFrame()->Undo();
}
void WIWeb::SetZoomLevel(double lv)
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	browser->GetHost()->SetZoomLevel(lv);
}
double WIWeb::GetZoomLevel()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return 0.0;
	return browser->GetHost()->GetZoomLevel();
}

WebRenderHandler *WIWeb::GetRenderer() {return static_cast<CefRefPtr<WebRenderHandler>*>(m_webRenderer.get())->get();}
WebBrowserClient *WIWeb::GetBrowserClient() {return static_cast<CefRefPtr<WebBrowserClient>*>(m_browserClient.get())->get();}
CefBrowser *WIWeb::GetBrowser()
{
	if(m_browser == nullptr)
		return nullptr;
	return static_cast<CefRefPtr<CefBrowser>*>(m_browser.get())->get();
}

void WIWeb::OnCursorEntered()
{
	WIBase::OnCursorEntered();

}
void WIWeb::OnCursorExited()
{
	WIBase::OnCursorExited();

}
Vector2i WIWeb::GetBrowserMousePos() const
{
	return {
		m_mousePos.x /static_cast<float>(GetWidth()) *m_browserViewSize.x,
		m_mousePos.y /static_cast<float>(GetHeight()) *m_browserViewSize.y
	};
}
void WIWeb::OnCursorMoved(int x,int y)
{
	m_mousePos = {x,y};
	WIBase::OnCursorMoved(x,y);
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	auto host = browser->GetHost();
	auto brMousePos = GetBrowserMousePos();
	CefMouseEvent ev {};
	ev.x = brMousePos.x;
	ev.y = brMousePos.y;
	host->SendMouseMoveEvent(ev,!PosInBounds(x,y));
}
void WIWeb::MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods)
{
	WIBase::MouseCallback(button,state,mods);
	auto *browser = GetBrowser();
	if(browser == nullptr || (state != GLFW::KeyState::Press && state != GLFW::KeyState::Release))
		return;
	auto host = browser->GetHost();
	cef_mouse_button_type_t btType {};
	switch(button)
	{
		case GLFW::MouseButton::Left:
			btType = cef_mouse_button_type_t::MBT_LEFT;
			break;
		case GLFW::MouseButton::Right:
			btType = cef_mouse_button_type_t::MBT_RIGHT;
			break;
		case GLFW::MouseButton::Middle:
			btType = cef_mouse_button_type_t::MBT_MIDDLE;
			break;
		default:
			return;
	}
	auto brMousePos = GetBrowserMousePos();
	CefMouseEvent ev {};
	ev.x = brMousePos.x;
	ev.y = brMousePos.y;
	host->SendMouseClickEvent(ev,btType,(state == GLFW::KeyState::Press) ? false : true,1);

	//auto frame = browser->GetMainFrame();
	//frame->ExecuteJavaScript("function test() {alert('ExecuteJavaScript works!');}",frame->GetURL(),0);

	//frame->GetV8Context()->GetGlobal()->GetValue("test")->ExecuteFunction();
}
static uint32_t get_cef_modifiers(GLFW::Modifier mods)
{
	auto cefMods = 0u;
	const std::unordered_map<GLFW::Modifier,uint32_t> modifierMap = {
		{GLFW::Modifier::Shift,cef_event_flags_t::EVENTFLAG_SHIFT_DOWN},
		{GLFW::Modifier::Alt,cef_event_flags_t::EVENTFLAG_ALT_DOWN},
		{GLFW::Modifier::Control,cef_event_flags_t::EVENTFLAG_CONTROL_DOWN},
		{GLFW::Modifier::Super,cef_event_flags_t::EVENTFLAG_COMMAND_DOWN}
	};
	for(auto &pair : modifierMap)
	{
		if((mods &pair.first) != GLFW::Modifier::None)
			cefMods |= pair.second;
	}
	return cefMods;
}
void WIWeb::KeyboardCallback(GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods)
{
	WIBase::KeyboardCallback(key,scanCode,state,mods);
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	auto host = browser->GetHost();
	int32_t systemKey = -1;
	switch(key)
	{
		case GLFW::Key::Escape:
			systemKey = VK_ESCAPE;
			break;
		case GLFW::Key::Enter:
			systemKey = VK_RETURN;
			break;
		case GLFW::Key::Tab:
			systemKey = VK_TAB;
			break;
		case GLFW::Key::Backspace:
			systemKey = VK_BACK;
			break;
		case GLFW::Key::Insert:
			systemKey = VK_INSERT;
			break;
		case GLFW::Key::Delete:
			systemKey = VK_DELETE;
			break;
		case GLFW::Key::Right:
			systemKey = VK_RIGHT;
			break;
		case GLFW::Key::Left:
			systemKey = VK_LEFT;
			break;
		case GLFW::Key::Down:
			systemKey = VK_DOWN;
			break;
		case GLFW::Key::Up:
			systemKey = VK_UP;
			break;
		case GLFW::Key::PageUp:
			systemKey = VK_PRIOR;
			break;
		case GLFW::Key::PageDown:
			systemKey = VK_NEXT;
			break;
		case GLFW::Key::Home:
			systemKey = VK_HOME;
			break;
		case GLFW::Key::End:
			systemKey = VK_END;
			break;
		case GLFW::Key::CapsLock:
			systemKey = VK_CAPITAL;
			break;
		case GLFW::Key::ScrollLock:
			systemKey = VK_SCROLL;
			break;
		case GLFW::Key::NumLock:
			systemKey = VK_NUMLOCK;
			break;
		case GLFW::Key::PrintScreen:
			systemKey = VK_PRINT;
			break;
		case GLFW::Key::Pause:
			systemKey = VK_PAUSE;
			break;
		case GLFW::Key::F1:
			systemKey = VK_F1;
			break;
		case GLFW::Key::F2:
			systemKey = VK_F2;
			break;
		case GLFW::Key::F3:
			systemKey = VK_F3;
			break;
		case GLFW::Key::F4:
			systemKey = VK_F4;
			break;
		case GLFW::Key::F5:
			systemKey = VK_F5;
			break;
		case GLFW::Key::F6:
			systemKey = VK_F6;
			break;
		case GLFW::Key::F7:
			systemKey = VK_F7;
			break;
		case GLFW::Key::F8:
			systemKey = VK_F8;
			break;
		case GLFW::Key::F9:
			systemKey = VK_F9;
			break;
		case GLFW::Key::F10:
			systemKey = VK_F10;
			break;
		case GLFW::Key::F11:
			systemKey = VK_F11;
			break;
		case GLFW::Key::F12:
			systemKey = VK_F12;
			break;
		case GLFW::Key::F13:
			systemKey = VK_F13;
			break;
		case GLFW::Key::F14:
			systemKey = VK_F14;
			break;
		case GLFW::Key::F15:
			systemKey = VK_F15;
			break;
		case GLFW::Key::F16:
			systemKey = VK_F16;
			break;
		case GLFW::Key::F17:
			systemKey = VK_F17;
			break;
		case GLFW::Key::F18:
			systemKey = VK_F18;
			break;
		case GLFW::Key::F19:
			systemKey = VK_F19;
			break;
		case GLFW::Key::F20:
			systemKey = VK_F20;
			break;
		case GLFW::Key::F21:
			systemKey = VK_F21;
			break;
		case GLFW::Key::F22:
			systemKey = VK_F22;
			break;
		case GLFW::Key::F23:
			systemKey = VK_F23;
			break;
		case GLFW::Key::F24:
			systemKey = VK_F24;
			break;
		//case GLFW::Key::F25:
		//	systemKey = VK_F25;
		//	break;
		case GLFW::Key::Kp0:
			systemKey = VK_NUMPAD0;
			break;
		case GLFW::Key::Kp1:
			systemKey = VK_NUMPAD1;
			break;
		case GLFW::Key::Kp2:
			systemKey = VK_NUMPAD2;
			break;
		case GLFW::Key::Kp3:
			systemKey = VK_NUMPAD3;
			break;
		case GLFW::Key::Kp4:
			systemKey = VK_NUMPAD4;
			break;
		case GLFW::Key::Kp5:
			systemKey = VK_NUMPAD5;
			break;
		case GLFW::Key::Kp6:
			systemKey = VK_NUMPAD6;
			break;
		case GLFW::Key::Kp7:
			systemKey = VK_NUMPAD7;
			break;
		case GLFW::Key::Kp8:
			systemKey = VK_NUMPAD8;
			break;
		case GLFW::Key::Kp9:
			systemKey = VK_NUMPAD9;
			break;
		case GLFW::Key::KpDecimal:
			systemKey = VK_DECIMAL;
			break;
		case GLFW::Key::KpDivide:
			systemKey = VK_DIVIDE;
			break;
		case GLFW::Key::KpMultiply:
			systemKey = VK_MULTIPLY;
			break;
		case GLFW::Key::KpSubtract:
			systemKey = VK_SUBTRACT;
			break;
		case GLFW::Key::KpAdd:
			systemKey = VK_ADD;
			break;
		case GLFW::Key::KpEnter:
			systemKey = VK_RETURN;
			break;
		//case GLFW::Key::KpEqual:
		//	systemKey = VK_KP_EQUAL;
		//	break;
		case GLFW::Key::LeftShift:
			systemKey = VK_LSHIFT;
			break;
		case GLFW::Key::LeftControl:
			systemKey = VK_LCONTROL;
			break;
		case GLFW::Key::LeftAlt:
			systemKey = VK_MENU;
			break;
		case GLFW::Key::LeftSuper:
			systemKey = VK_LWIN;
			break;
		case GLFW::Key::RightShift:
			systemKey = VK_LSHIFT;
			break;
		case GLFW::Key::RightControl:
			systemKey = VK_LCONTROL;
			break;
		case GLFW::Key::RightAlt:
			systemKey = VK_MENU;
			break;
		case GLFW::Key::RightSuper:
			systemKey = VK_RWIN;
			break;
		case GLFW::Key::Menu:
			systemKey = VK_MENU;
			break;
		default:
			return;
	}

	CefKeyEvent ev {};
	ev.type = (state == GLFW::KeyState::Press) ? cef_key_event_type_t::KEYEVENT_KEYDOWN : cef_key_event_type_t::KEYEVENT_KEYUP;
	ev.modifiers = get_cef_modifiers(mods);
	ev.character = systemKey;
	ev.native_key_code = systemKey;
	ev.windows_key_code = systemKey;
	ev.unmodified_character = systemKey;
	host->SendKeyEvent(ev);
}
void WIWeb::CharCallback(unsigned int c,GLFW::Modifier mods)
{
	WIBase::CharCallback(c,mods);
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	auto host = browser->GetHost();
	CefKeyEvent ev {};
	ev.type = cef_key_event_type_t::KEYEVENT_CHAR;
	ev.modifiers = get_cef_modifiers(mods);
	ev.character = c;
	ev.native_key_code = c;
	ev.windows_key_code = c;
	ev.unmodified_character = c;
	host->SendKeyEvent(ev);
}
void WIWeb::ScrollCallback(Vector2 offset)
{
	WIBase::ScrollCallback(offset);
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	auto host = browser->GetHost();
	auto brMousePos = GetBrowserMousePos();
	CefMouseEvent ev {};
	ev.x = brMousePos.x;
	ev.y = brMousePos.y;
	host->SendMouseWheelEvent(ev,offset.x *10.f,offset.y *10.f);
}
void WIWeb::OnFocusGained()
{
	WIBase::OnFocusGained();
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	auto host = browser->GetHost();
	host->SendFocusEvent(true);
}
void WIWeb::OnFocusKilled()
{
	WIBase::OnFocusKilled();
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	auto host = browser->GetHost();
	host->SendFocusEvent(false);
}
