// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.tree_list;

import pragma.gui;

pragma::gui::types::WITreeListElement::WITreeListElement() : WITableRow(), m_bCollapsed(false), m_pTreeParent {}, m_pArrow {}, m_pList {}, m_xOffset(0), m_depth(0)
{
	RegisterCallback<void>("OnCollapse");
	RegisterCallback<void>("OnTreeUpdate");
	RegisterCallback<void>("OnExpand");
	RegisterCallback<void>("OnTreeUpdate");
}

void pragma::gui::types::WITreeListElement::Initialize() { WITableRow::Initialize(); }
void pragma::gui::types::WITreeListElement::OnRemove()
{
	WITableRow::OnRemove();
	for(auto &hItem : m_items) {
		if(hItem.IsValid())
			hItem->Remove();
	}
}
uint32_t pragma::gui::types::WITreeListElement::GetDepth() const { return m_depth; }
void pragma::gui::types::WITreeListElement::OnVisibilityChanged(bool bVisible)
{
	WITableRow::OnVisibilityChanged(bVisible);
	if(IsCollapsed() == true)
		return;
	for(auto &hItem : m_items) {
		if(hItem.IsValid() == true) {
			auto *pEl = static_cast<WITreeListElement *>(hItem.get());
			pEl->SetVisible(bVisible);
		}
	}
}

bool pragma::gui::types::WITreeListElement::IsCollapsed() const { return m_bCollapsed; }
void pragma::gui::types::WITreeListElement::SetXOffset(uint32_t x) { m_xOffset = x; }

void pragma::gui::types::WITreeListElement::Toggle(bool bAll)
{
	if(IsCollapsed() == true)
		Expand(bAll);
	else
		Collapse(bAll);
}
void pragma::gui::types::WITreeListElement::Collapse(bool bAll)
{
	if(m_bCollapsed == true)
		return;
	if(m_fPopulate) {
		for(auto &hItem : m_items) {
			if(hItem.IsValid())
				hItem->Remove();
		}
		m_items.clear();
	}
	if(m_pArrow.IsValid())
		static_cast<WIArrow *>(m_pArrow.get())->SetDirection(WIArrow::Direction::Right);
	m_bCollapsed = true;
	for(auto &hItem : m_items) {
		if(hItem.IsValid() == true) {
			auto *pElement = static_cast<WITreeListElement *>(hItem.get());
			if(bAll == true)
				pElement->Collapse(bAll);
			pElement->SetVisible(false);
		}
	}
	CallCallbacks("OnCollapse");
	CallCallbacks("OnTreeUpdate");
}
void pragma::gui::types::WITreeListElement::Expand(bool bAll)
{
	if(m_bCollapsed == false)
		return;
	if(m_fPopulate)
		m_fPopulate(*this);
	if(m_pArrow.IsValid())
		static_cast<WIArrow *>(m_pArrow.get())->SetDirection(WIArrow::Direction::Down);
	m_bCollapsed = false;
	for(auto &hItem : m_items) {
		if(hItem.IsValid() == true) {
			auto *pElement = static_cast<WITreeListElement *>(hItem.get());
			if(bAll == true)
				pElement->Expand(bAll);
			pElement->SetVisible(true);
		}
	}
	CallCallbacks("OnExpand");
	CallCallbacks("OnTreeUpdate");
}
void pragma::gui::types::WITreeListElement::Clear()
{
	for(auto &hItem : m_items) {
		if(hItem.IsValid())
			hItem->Remove();
	}
	m_items.clear();
}
void pragma::gui::types::WITreeListElement::SetTreeParent(WITreeListElement *pEl)
{
	if(pEl == nullptr) {
		if(m_pArrow.IsValid())
			m_pArrow->Remove();
		m_pTreeParent = decltype(m_pTreeParent) {};
		m_depth = 0;
		return;
	}
	m_pTreeParent = pEl->GetHandle();
	m_depth = pEl->GetDepth() + 1;
	if(m_pArrow.IsValid() == false) {
		auto *pArrow = WGUI::GetInstance().Create<WIArrow>(this);
		m_pArrow = pArrow->GetHandle();
		pArrow->SetName("arrow");
		pArrow->SetSize(12, 12);
		pArrow->SetVisible(false);
		pArrow->SetPos(m_xOffset, 3);
		pArrow->SetDirection(WIArrow::Direction::Down);
		auto hThis = GetHandle();
		pArrow->AddCallback("OnMousePressed", FunctionCallback<util::EventReply>::CreateWithOptionalReturn([hThis](util::EventReply *reply) mutable -> CallbackReturnType {
			*reply = util::EventReply::Handled;
			if(hThis.IsValid() == false)
				return CallbackReturnType::HasReturnValue;
			static_cast<WITreeListElement *>(hThis.get())->Toggle();
			return CallbackReturnType::HasReturnValue;
		}));
		InsertElement(0, pArrow);
	}
}
void pragma::gui::types::WITreeListElement::SetList(WITreeList *pList)
{
	if(pList == nullptr) {
		m_pList = decltype(m_pList) {};
		return;
	}
	m_pList = pList->GetHandle();
}
const std::vector<pragma::gui::WIHandle> &pragma::gui::types::WITreeListElement::GetItems() const { return m_items; }
pragma::gui::types::WITreeListElement *pragma::gui::types::WITreeListElement::GetLastItem() const
{
	for(auto it = m_items.rbegin(); it != m_items.rend(); ++it) {
		auto &hItem = *it;
		if(hItem.IsValid())
			return static_cast<const WITreeListElement *>(hItem.get())->GetLastItem();
	}
	return const_cast<WITreeListElement *>(this);
}
void pragma::gui::types::WITreeListElement::SetTextElement(WIText *pText)
{
	(pText != nullptr) ? m_hText = pText->GetHandle() : WIHandle {};
	if(pText) {
		pText->SetName("text");
		pText->AddStyleClass("tree_list_element_text");
		pText->GetColorProperty()->Link(*GetColorProperty());
	}
}
pragma::gui::types::WIText *pragma::gui::types::WITreeListElement::GetTextElement() const { return m_hText.IsValid() ? const_cast<WIText *>(static_cast<const WIText *>(m_hText.get())) : nullptr; }
pragma::gui::types::WITreeListElement *pragma::gui::types::WITreeListElement::AddItem(const std::string &text, const std::function<void(WITreeListElement &)> &fPopulate)
{
	if(m_pList.IsValid() == false)
		return nullptr;
	auto *pList = static_cast<WITreeList *>(m_pList.get());
	auto *pEl = static_cast<WITreeListElement *>(static_cast<WITreeList *>(m_pList.get())->AddRow());
	auto hText = pEl->SetValue(0, text);
	pEl->SetXOffset(m_xOffset + ((m_pTreeParent.IsValid() == true) ? 10 : 0));
	pEl->SetTextElement(static_cast<WIText *>(hText.get()));
	pEl->SetTreeParent(this);
	pEl->SetList(pList);
	pEl->m_fPopulate = fPopulate;
	auto hThis = GetHandle();
	pEl->AddCallback("OnTreeUpdate", FunctionCallback<>::Create([hThis]() mutable {
		if(hThis.IsValid() == false)
			return;
		hThis.get()->CallCallbacks("OnTreeUpdate");
	}));
	if(pEl->m_pArrow.IsValid())
		hText->SetX(pEl->m_pArrow->GetX() + pEl->m_pArrow->GetWidth() + 4);
	auto *pLast = GetLastItem();
	pList->MoveRow(pEl, pLast);
	m_items.push_back(pEl->GetHandle());

	if(fPopulate)
		pEl->Collapse();
	if(pEl->m_pArrow.IsValid() && pEl->IsCollapsed())
		pEl->m_pArrow->SetVisible(true);
	return pEl;
}

////////////

pragma::gui::types::WITreeList::WITreeList() : WITable(), m_pRoot {} {}

void pragma::gui::types::WITreeList::Initialize()
{
	WITable::Initialize();
	SetRowHeight(18);
	auto *pRoot = WGUI::GetInstance().Create<WITreeListElement>(this);
	m_pRoot = pRoot->GetHandle();
	pRoot->SetList(this);
	pRoot->SetAutoAlignToParent(true);
	auto hThis = GetHandle();
	pRoot->AddCallback("OnTreeUpdate", FunctionCallback<>::Create([hThis, this]() {
		if(hThis.IsValid() == false)
			return;
		Update();
	}));
}
void pragma::gui::types::WITreeList::DoUpdate() { WITable::DoUpdate(); }
pragma::gui::types::WITreeListElement *pragma::gui::types::WITreeList::GetRootItem() const { return const_cast<WITreeListElement *>(static_cast<const WITreeListElement *>(m_pRoot.get())); }
pragma::gui::types::WITreeListElement *pragma::gui::types::WITreeList::AddItem(const std::string &text, const std::function<void(WITreeListElement &)> &fPopulate)
{
	if(m_pRoot.IsValid() == false)
		return nullptr;
	return static_cast<WITreeListElement *>(m_pRoot.get())->AddItem(text, fPopulate);
}
pragma::util::EventReply pragma::gui::types::WITreeList::MouseCallback(platform::MouseButton button, platform::KeyState state, platform::Modifier mods)
{
	if(WITable::MouseCallback(button, state, mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	RequestFocus();
	return util::EventReply::Handled;
}
void pragma::gui::types::WITreeList::SetSize(int x, int y) { WITable::SetSize(x, y); }
pragma::gui::types::WITableRow *pragma::gui::types::WITreeList::AddRow() { return WITable::AddRow<WITreeListElement>(); }
void pragma::gui::types::WITreeList::ExpandAll()
{
	if(m_pRoot.IsValid() == false)
		return;
	static_cast<WITreeListElement *>(m_pRoot.get())->Expand(true);
}
void pragma::gui::types::WITreeList::CollapseAll()
{
	if(m_pRoot.IsValid() == false)
		return;
	static_cast<WITreeListElement *>(m_pRoot.get())->Collapse(true);
}
