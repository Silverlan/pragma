#include "stdafx_client.h"
#include "pragma/console/c_cvar_global_functions.h"
#include <wgui/types/witext.h>
#include <wgui/types/wirect.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;

class GUIDebugCursorManager
{
public:
	GUIDebugCursorManager();
	~GUIDebugCursorManager();
	bool Initialize();
private:
	static std::string GetElementInfo(WIBase &el);
	void Clear();
	void OnThink();
	bool ShouldPass(WIBase &el) const;
	void SetTargetGUIElement(WIBase *optEl,bool clear);

	void SelectNextParentInHierarchy();
	void SelectNextChildInHierarchy();

	CallbackHandle m_cbThink = {};
	CallbackHandle m_cbScroll = {};
	CallbackHandle m_cbOnClose = {};
	WIHandle m_hText = {};
	std::array<WIHandle,4> m_borderElements = {};
	std::array<WIHandle,4> m_borderElementsConstrained = {};
	std::vector<WIHandle> m_cursorElementList = {}; // Last element is bottom-most element in hierarchy, all elements above are parents
};

GUIDebugCursorManager::GUIDebugCursorManager()
{}

GUIDebugCursorManager::~GUIDebugCursorManager()
{
	Clear();
}

void GUIDebugCursorManager::Clear()
{
	if(m_cbThink.IsValid())
		m_cbThink.Remove();
	if(m_cbScroll.IsValid())
		m_cbScroll.Remove();
	if(m_cbOnClose.IsValid())
		m_cbOnClose.Remove();
	if(m_hText.IsValid())
		m_hText->Remove();
	for(auto &hEl : m_borderElements)
	{
		if(hEl.IsValid())
			hEl->Remove();
	}
	for(auto &hEl : m_borderElementsConstrained)
	{
		if(hEl.IsValid())
			hEl->Remove();
	}
}

bool GUIDebugCursorManager::Initialize()
{
	auto &gui = WGUI::GetInstance();
	auto *pText = gui.Create<WIText>();
	if(pText == nullptr)
		return false;
	m_hText = pText->GetHandle();
	pText->SetZPos(std::numeric_limits<int32_t>::max());
	pText->SetVisible(false);
	pText->EnableShadow(true);
	pText->SetShadowOffset(Vector2i(1,1));
	pText->SetColor(Color::Orange);

	for(auto &hEl : m_borderElements)
	{
		auto *el = WGUI::GetInstance().Create<WIRect>();
		el->SetColor(Color::Red);
		el->SetZPos(std::numeric_limits<int>::max());
		el->SetVisible(false);
		hEl = el->GetHandle();
	}
	for(auto &hEl : m_borderElementsConstrained)
	{
		auto *el = WGUI::GetInstance().Create<WIRect>();
		el->SetColor(Color::Aqua);
		el->SetAlpha(0.75f);
		el->SetZPos(std::numeric_limits<int>::max());
		el->SetVisible(false);
		hEl = el->GetHandle();
	}

	m_cbScroll = c_engine->AddCallback("OnScrollInput",FunctionCallback<bool,std::reference_wrapper<GLFW::Window>,Vector2>::CreateWithOptionalReturn([this](bool *reply,std::reference_wrapper<GLFW::Window> window,Vector2 scrollAmount) -> CallbackReturnType {
		if(scrollAmount.y == 0.f)
			return CallbackReturnType::NoReturnValue;
		*reply = true;
		if(scrollAmount.y < 0.f)
			SelectNextParentInHierarchy();
		else
			SelectNextChildInHierarchy();
		return CallbackReturnType::HasReturnValue;
	}));
	m_cbOnClose = client->AddCallback("OnClose",FunctionCallback<void>::Create([this]() {
		Clear();
	}));

	m_cbThink = client->AddCallback("Think",FunctionCallback<void>::Create([this]() {
		OnThink();
	}));
	return true;
}

bool GUIDebugCursorManager::ShouldPass(WIBase &el) const
{
	if(el.IsVisible() == false || &el == m_hText.get() || el.IsDescendantOf(m_hText.get()))
		return false;
	for(auto &hEl : m_borderElements)
	{
		if(&el == hEl.get())
			return false;
	}
	for(auto &hEl : m_borderElementsConstrained)
	{
		if(&el == hEl.get())
			return false;
	}
	return true;
}

std::string GUIDebugCursorManager::GetElementInfo(WIBase &el)
{
	std::stringstream ss;
	auto pos = el.GetAbsolutePos();
	auto size = el.GetSize();

	ss<<"Element: "<<std::string(typeid(el).name())<<
		"; Class: "<<el.GetClass()<<
		"; Name: "<<el.GetName()<<
		"; Index: "<<el.GetIndex()<<
		"; Size: ("<<size.x<<","<<size.y<<")"<<
		"; Pos: ("<<pos.x<<","<<pos.y<<")";
	auto debugInfo = el.GetDebugInfo();
	if(debugInfo.empty() == false)
		ss<<"; Debug Info: "<<debugInfo;
	return ss.str();
}

void GUIDebugCursorManager::SelectNextParentInHierarchy()
{
	if(m_cursorElementList.empty() || m_cursorElementList.front().IsValid() == false)
		return;
	auto &hEl = m_cursorElementList.front();
	auto *parent = hEl->GetParent();
	if(parent)
		SetTargetGUIElement(parent,false);
}
void GUIDebugCursorManager::SelectNextChildInHierarchy()
{
	auto isFirstElement = true;
	for(auto it=m_cursorElementList.begin();it!=m_cursorElementList.end();)
	{
		auto first = isFirstElement;
		isFirstElement = false;
		auto &hEl = *it;
		if(hEl.IsValid() == false || first)
		{
			it = m_cursorElementList.erase(it);
			continue;
		}
		auto *el = it->get();
		m_cursorElementList.erase(it);
		SetTargetGUIElement(el,false);
		break;
	}
}

void GUIDebugCursorManager::SetTargetGUIElement(WIBase *optEl,bool clear)
{
	auto dbgGUIVisible = (optEl != nullptr);
	if(m_hText.IsValid())
		m_hText->SetVisible(dbgGUIVisible);
	for(auto &hEl : m_borderElements)
	{
		if(hEl.IsValid())
			hEl->SetVisible(dbgGUIVisible);
	}
	for(auto &hEl : m_borderElementsConstrained)
	{
		if(hEl.IsValid())
			hEl->SetVisible(dbgGUIVisible);
	}

	if(optEl == nullptr)
	{
		m_cursorElementList.clear();
		return;
	}
	auto &el = *optEl;
	if(m_cursorElementList.empty() == false && m_cursorElementList.back().get() == &el)
		return;
	if(clear)
		m_cursorElementList.clear();
	m_cursorElementList.insert(m_cursorElementList.begin(),el.GetHandle());

	auto *pText = static_cast<WIText*>(m_hText.get());
	if(pText)
	{
		pText->SetText(GetElementInfo(el));
		Con::cout<<pText->GetText()<<Con::endl;
	}

	// Initialize border to highlight the element
	auto pos = el.GetAbsolutePos();
	auto size = el.GetSize();
	auto constrainedEndPos = pos +size;
	auto *parent = el.GetParent();
	while(parent)
	{
		auto endPos = parent->GetAbsolutePos() +parent->GetSize();
		constrainedEndPos.x = umath::min(constrainedEndPos.x,endPos.x);
		constrainedEndPos.y = umath::min(constrainedEndPos.y,endPos.y);

		parent = parent->GetParent();
	}
	auto constrainedSize = constrainedEndPos -pos;
	constrainedSize.x = umath::max(constrainedSize.x,0);
	constrainedSize.y = umath::max(constrainedSize.y,0);

	auto fInitBorder = [](std::array<WIHandle,4> &elements,const Vector2i &pos,const Vector2i &size) {
		auto *top = elements.at(0).get();
		auto *right = elements.at(1).get();
		auto *bottom = elements.at(2).get();
		auto *left = elements.at(3).get();
		if(top)
		{
			top->SetPos(pos.x -1,pos.y -1);
			top->SetSize(size.x +2,1);
		}

		if(right)
		{
			right->SetPos(pos.x +size.x,pos.y -1);
			right->SetSize(1,size.y +2);
		}

		if(bottom)
		{
			bottom->SetPos(pos.x -1,pos.y +size.y);
			bottom->SetSize(size.x +2,1);
		}

		if(left)
		{
			left->SetPos(pos.x -1,pos.y -1);
			left->SetSize(1,size.y +2);
		}
	};
	fInitBorder(m_borderElements,pos,size);
	fInitBorder(m_borderElementsConstrained,pos,constrainedSize);


	std::string t = "\t";
	auto *pParent = el.GetParent();
	while(pParent != nullptr)
	{
		Con::cout<<t<<GetElementInfo(*pParent)<<Con::endl;
		pParent = pParent->GetParent();
		t += "\t";
	}
	if(pText)
		pText->SizeToContents();
}

void GUIDebugCursorManager::OnThink()
{
	if(m_hText.IsValid() == false)
		return;
	auto &gui = WGUI::GetInstance();
	auto *pText = static_cast<WIText*>(m_hText.get());
	auto *pEl = gui.GetCursorGUIElement(nullptr,[this](WIBase *pEl) -> bool {return ShouldPass(*pEl);});
	SetTargetGUIElement(pEl,true);

	if(m_hText.IsValid())
	{
		int32_t x,y;
		gui.GetMousePos(x,y);
		m_hText->SetPos(x,y);
	}
}

void Console::commands::debug_gui_cursor(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	static std::unique_ptr<GUIDebugCursorManager> s_dbgManager = nullptr;
	if(s_dbgManager != nullptr)
	{
		s_dbgManager = nullptr;
		return;
	}
	s_dbgManager = std::make_unique<GUIDebugCursorManager>();
	if(s_dbgManager->Initialize() == false)
		s_dbgManager = nullptr;
}
