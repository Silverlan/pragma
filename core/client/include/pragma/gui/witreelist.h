#ifndef __WITREELIST_H__
#define __WITREELIST_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/witable.h"

class WITreeList;
class WIText;
class DLLCLIENT WITreeListElement
	: public WITableRow
{
protected:
	std::vector<WIHandle> m_items;
	bool m_bCollapsed;
	uint32_t m_xOffset;
	uint32_t m_depth;
	WIHandle m_pTreeParent;
	WIHandle m_pArrow;
	WIHandle m_pList;
	WIHandle m_hText = {};
	WITreeListElement *GetLastItem() const;
	void SetTextElement(WIText *pText);
public:
	WITreeListElement();
	virtual void Initialize() override;
	virtual void OnVisibilityChanged(bool bVisible) override;
	void SetTreeParent(WITreeListElement *pEl);
	void SetXOffset(uint32_t x);
	void SetList(WITreeList *pList);
	uint32_t GetDepth() const;
	bool IsCollapsed() const;
	void Toggle(bool bAll=false);
	void Collapse(bool bAll=false);
	void Expand(bool bAll=false);
	void Clear();
	WIText *GetTextElement() const;
	const std::vector<WIHandle> &GetItems() const;
	WITreeListElement *AddItem(const std::string &text);
};

class DLLCLIENT WITreeList
	: public WITable
{
protected:
	WIHandle m_pRoot;
public:
	WITreeList();
	virtual void Initialize() override;
	virtual void SetSize(int x,int y) override;
	virtual void MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) override;
	virtual WITableRow *AddRow() override;
	WITreeListElement *AddItem(const std::string &text);
	virtual void Update() override;
	WITreeListElement *GetRootItem() const;
	void ExpandAll();
	void CollapseAll();
};

#endif