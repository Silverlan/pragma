/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WITREELIST_H__
#define __WITREELIST_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/witable.h"

class WITreeList;
class WIText;
class DLLCLIENT WITreeListElement : public WITableRow {
  public:
	WITreeListElement();
	virtual void Initialize() override;
	virtual void OnRemove() override;
	virtual void OnVisibilityChanged(bool bVisible) override;
	void SetTreeParent(WITreeListElement *pEl);
	void SetXOffset(uint32_t x);
	void SetList(WITreeList *pList);
	uint32_t GetDepth() const;
	bool IsCollapsed() const;
	void Toggle(bool bAll = false);
	void Collapse(bool bAll = false);
	void Expand(bool bAll = false);
	void Clear();
	WIText *GetTextElement() const;
	const std::vector<WIHandle> &GetItems() const;
	WITreeListElement *AddItem(const std::string &text, const std::function<void(WITreeListElement &)> &fPopulate = nullptr);
  protected:
	std::vector<WIHandle> m_items;
	bool m_bCollapsed;
	std::function<void(WITreeListElement &)> m_fPopulate = nullptr;
	uint32_t m_xOffset;
	uint32_t m_depth;
	WIHandle m_pTreeParent;
	WIHandle m_pArrow;
	WIHandle m_pList;
	WIHandle m_hText = {};
	WITreeListElement *GetLastItem() const;
	void SetTextElement(WIText *pText);
};

class DLLCLIENT WITreeList : public WITable {
  protected:
	virtual void DoUpdate() override;
	WIHandle m_pRoot;
  public:
	WITreeList();
	virtual void Initialize() override;
	virtual void SetSize(int x, int y) override;
	virtual util::EventReply MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods) override;
	virtual WITableRow *AddRow() override;
	WITreeListElement *AddItem(const std::string &text, const std::function<void(WITreeListElement &)> &fPopulate = nullptr);
	WITreeListElement *GetRootItem() const;
	void ExpandAll();
	void CollapseAll();
};

#endif
