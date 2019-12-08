#ifndef __WISCROLLCONTAINER_H__
#define __WISCROLLCONTAINER_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

class WIScrollBar;
class DLLCLIENT WIScrollContainer
	: public WIBase
{
public:
	WIScrollContainer();
	virtual ~WIScrollContainer() override;
	virtual void Initialize() override;
	virtual void SetSize(int x,int y) override;
	virtual util::EventReply ScrollCallback(Vector2 offset) override;
	virtual void SizeToContents(bool x=true,bool y=true) override;
	WIScrollBar *GetHorizontalScrollBar();
	WIScrollBar *GetVerticalScrollBar();
	int GetScrollBarWidthV();
	int GetScrollBarHeightH();
	int GetContentWidth();
	int GetContentHeight();
	Vector2i GetContentSize();

	void ScrollToBottom();

	void SetAutoStickToBottom(bool autoStick);
	bool ShouldAutoStickToBottom() const;
protected:
	virtual void DoUpdate() override;
	bool m_bAutoStickToBottom = false;
	WIHandle m_hScrollBarH = {};
	WIHandle m_hScrollBarV = {};
	WIHandle m_hWrapper = {};
	CallbackHandle m_hChildAdded = {};
	CallbackHandle m_hChildRemoved = {};
	Vector2i m_lastOffset = {};
	std::unordered_map<WIBase*,std::vector<CallbackHandle>> m_childCallbackHandles;
	virtual void OnChildAdded(WIBase *child) override;
	virtual void OnChildRemoved(WIBase *child) override;
	void OnHScrollOffsetChanged(unsigned int offset);
	void OnVScrollOffsetChanged(unsigned int offset);
	void OnWrapperChildAdded(WIBase *child);
	void OnWrapperChildRemoved(WIBase *child);
	static void OnChildReleased(WIBase *child);
	static void OnChildSetSize(WIBase *child);
};

#endif