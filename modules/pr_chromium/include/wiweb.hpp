#ifndef __WIWEB_HPP__
#define __WIWEB_HPP__

#include <wgui/types/wirect.h>
#include <c_ldef_wguihandles.h>
#include <prosper_texture.hpp>

class WebRenderHandler;
class WebBrowserClient;
class CefBrowser;
class CefBrowserHost;
namespace pragma {class BrowserProcess;};
class __declspec(dllexport) WIWeb
	: public WITexturedRect
{
public:
	WIWeb();
	virtual ~WIWeb() override;
	virtual void InitializeHandle() override;
	virtual void Initialize() override;
	virtual void Think() override;

	virtual void OnCursorEntered() override;
	virtual void OnCursorExited() override;
	virtual void OnCursorMoved(int x,int y) override;
	virtual void MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) override;
	virtual void KeyboardCallback(GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods) override;
	virtual void CharCallback(unsigned int c,GLFW::Modifier mods=GLFW::Modifier::None) override;
	virtual void ScrollCallback(Vector2 offset) override;
	virtual void OnFocusGained() override;
	virtual void OnFocusKilled() override;
	virtual void Update() override;

	void LoadURL(const std::string &url);
	void SetBrowserViewSize(const Vector2i size);
	const Vector2i &GetBrowserViewSize() const;
	bool CanGoBack();
	bool CanGoForward();
	void GoBack();
	void GoForward();
	bool HasDocument();
	bool IsLoading();
	void Reload();
	void ReloadIgnoreCache();
	void StopLoad();

	void Copy();
	void Cut();
	void Delete();
	void Paste();
	void Redo();
	void SelectAll();
	void Undo();

	void SetZoomLevel(double lv);
	double GetZoomLevel();
	void SetTransparentBackground(bool b);

	pragma::BrowserProcess *GetBrowserProcess();
	WebRenderHandler *GetRenderer();
	WebBrowserClient *GetBrowserClient();
	CefBrowser *GetBrowser();
private:
	bool InitializeChromiumBrowser();
	bool InitializeBrowserProcess();

	std::shared_ptr<prosper::Texture> m_texture = nullptr;
	bool m_bTransparentBackground = false;
	Vector2i m_browserViewSize = {};
	Vector2i m_mousePos = {};
	Vector2i GetBrowserMousePos() const;

	std::shared_ptr<void> m_webRenderer = nullptr;
	std::shared_ptr<void> m_browserClient = nullptr;
	std::shared_ptr<void> m_browser = nullptr;
	
	static bool s_bBrowserProcessInitialized;
	static uint32_t s_webCount;
};

DECLARE_DERIVED_CHILD_HANDLE(__declspec(dllexport),WI,WIBase,WITexturedShape,WIWeb,WIWeb);

#endif
