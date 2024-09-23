--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("wimenuitem.lua")
include("vbox.lua")

util.register_class("gui.WIContextMenu", gui.Base)

gui.impl = gui.impl or {}
gui.impl.contextMenu = gui.impl.contextMenu or {
	activeMenuCount = 0,
	menues = {},
}

function gui.WIContextMenu:OnRemove()
	gui.impl.contextMenu.activeMenuCount = gui.impl.contextMenu.activeMenuCount - 1
	if gui.impl.contextMenu.activeMenuCount == 0 and util.is_valid(gui.impl.cbMouseInput) == true then
		gui.impl.cbMouseInput:Remove()
	end
end
function gui.WIContextMenu:OnInitialize()
	gui.impl.contextMenu.activeMenuCount = gui.impl.contextMenu.activeMenuCount + 1
	gui.Base.OnInitialize(self)

	self:SetSize(64, 128)

	self.m_tItems = {}
	self.m_subMenues = {}
	self.m_itemToSubMenu = {}
	local pBg = gui.create("WIRect", self)
	pBg:SetBackgroundElement(true)
	pBg:SetAutoAlignToParent(true)
	pBg:AddStyleClass("context_menu_background")
	self:SetKeyboardInputEnabled(true)
	pBg:SetColor(Color.Beige)
	self.m_pBg = pBg

	local pBgOutline = gui.create("WIOutlinedRect", self)
	pBgOutline:SetBackgroundElement(true)
	pBgOutline:SetAutoAlignToParent(true)
	pBgOutline:AddStyleClass("context_menu_outline")
	pBgOutline:SetColor(Color.Gray)
	self.m_pBgOutline = pBgOutline
	if util.is_valid(gui.impl.cbMouseInput) == false then
		gui.impl.cbMouseInput = input.add_callback("OnMouseInput", function(button, action, mods)
			if action == input.STATE_PRESS then
				local window = gui.find_focused_window()
				local el = gui.get_element_under_cursor(window, function(el)
					return el:GetMouseInputEnabled()
				end)
				while util.is_valid(el) and el:GetClass() ~= "wicontextmenu" do
					el = el:GetParent()
				end
				if util.is_valid(el) then
					return
				end
				gui.close_all_context_menues()
			end
		end)
	end

	local scrollContainer = gui.create("WIScrollContainer", self, 0, 0, self:GetWidth(), self:GetHeight(), 0, 0, 1, 1)
	scrollContainer:AddCallback("SetSize", function(el)
		self.m_contents:SetWidth(el:GetWidth())
	end)
	scrollContainer:GetVerticalScrollBar():SetScrollAmount(1)
	self.m_scrollContainer = scrollContainer

	local contents = gui.create("WIVBox", scrollContainer, 0, 0, self:GetWidth(), self:GetHeight())
	contents:SetFixedWidth(true)
	contents:AddCallback("SetSize", function(el)
		for _, item in ipairs(self.m_tItems) do
			if item:IsValid() then
				item:SetWidth(el:GetWidth())
			end
		end
	end)
	contents:AddCallback("OnUpdated", function(el)
		self.m_scrollContainer:ScheduleUpdate()
	end)
	self.m_contents = contents

	self:AddStyleClass("context_menu")
	if gui.impl.contextMenu.skin ~= nil then
		self:SetSkin(gui.impl.contextMenu.skin)
	end
end
function gui.WIContextMenu:IsPopulated()
	return self:GetItemCount() > 0
end
function gui.WIContextMenu:IsCursorInMenuBounds()
	if self:IsCursorInBounds() == true then
		return true
	end
	for _, subMenu in ipairs(self.m_subMenues) do
		if subMenu:IsValid() == true and subMenu:IsCursorInMenuBounds() == true then
			return true
		end
	end
	return false
end
function gui.WIContextMenu:GetSelectedItem()
	for _, pItem in ipairs(self.m_tItems) do
		if pItem:IsValid() == true and pItem:IsSelected() == true then
			return pItem
		end
	end
	for _, subMenu in ipairs(self.m_subMenues) do
		if subMenu:IsValid() == true then
			local pItem = subMenu:GetSelectedItem()
			if pItem ~= nil then
				return pItem
			end
		end
	end
end
function gui.WIContextMenu:KeyboardCallback(key, scanCode, action, mods)
	local pItem = self:GetSelectedItem()
	if pItem == nil then
		return
	end
	local cmd = pItem:GetKeybindCommand()
	if cmd == nil then
		return
	end
	local b, keyStr = input.key_to_text(key)
	engine.bind_key(keyStr, cmd)
	pItem:SetRightText(keyStr)
end
function gui.WIContextMenu:AddLine()
	-- TODO
end
function gui.WIContextMenu:OnUpdate()
	local updateItems = {}
	local function updateAutoSize(item)
		for _, menu in ipairs(item.m_subMenues) do
			updateAutoSize(menu)
		end
		for _, subItem in ipairs(item.m_tItems) do
			if subItem:IsValid() then
				subItem:Update()
			end
		end

		item.m_contents:SetAutoSizeToContents(true, false)
		item.m_contents:Update()

		table.insert(updateItems, item.m_scrollContainer)
	end
	updateAutoSize(self)

	local w = 108
	local h = self:GetHeight()
	for _, item in ipairs(self.m_tItems) do
		item:SizeToContents()
		if item:IsValid() then
			w = math.max(w, item:GetWidth())
		end
	end
	w = w + 20
	if self.m_contents:GetHeight() <= 128 and self.m_contents:GetHeight() ~= self:GetHeight() then
		h = self.m_contents:GetHeight()
	end
	self:SetSize(w, h)

	for _, item in ipairs(updateItems) do
		item:Update()
	end

	self:UpdateFlipState()
end
function gui.WIContextMenu:UpdateFlipState()
	local xBase = self:GetLeft()
	local yBase = self:GetTop()

	self.m_xFlipped = false
	self.m_yFlipped = false
	if yBase + self:GetHeight() > self:GetParent():GetBottom() then
		local y = yBase - self:GetHeight()
		if #self.m_tItems > 0 then
			y = y + self.m_tItems[1]:GetHeight()
		end
		self:SetY(y)
		self.m_yFlipped = true
	else
		self:SetY(yBase)
	end
	if xBase + self:GetWidth() > self:GetParent():GetRight() then
		if util.is_valid(self.m_parentMenu) then
			self:SetX(self.m_parentMenu:GetX() - self:GetWidth())
		else
			self:SetX(xBase - self:GetWidth())
		end
		self.m_xFlipped = true
	else
		self:SetX(xBase)
	end
end
function gui.WIContextMenu:GetItemCount()
	return #self.m_tItems
end
function gui.WIContextMenu:Clear()
	for _, item in ipairs(self.m_tItems) do
		util.remove(item)
	end
end
function gui.WIContextMenu:AddItem(name, fcOnClick, keybind)
	local pItem = gui.create("WIMenuItem", self.m_contents)
	if pItem == nil then
		return
	end
	pItem:SetTitle(name)
	if keybind ~= nil then
		pItem:SetKeybindCommand(keybind)
		local mappedKeys = input.get_mapped_keys(keybind)
		if #mappedKeys > 0 then
			local b, keyStr = input.key_to_text(mappedKeys[1])
			pItem:SetRightText(keyStr)
		end
	end
	pItem:SetAction(function(pItem)
		if fcOnClick ~= nil then
			if fcOnClick(pItem) == false then
				return
			end
		end
		gui.close_context_menu(pItem:GetRootElement())
	end)
	pItem:SizeToContents()
	table.insert(self.m_tItems, pItem)
	return pItem
end
function gui.WIContextMenu:GetContents()
	return self.m_contents
end
function gui.WIContextMenu:SetParentMenu(menu, parentItem)
	self.m_parentMenu = menu
	self.m_parentItem = parentItem
end
function gui.WIContextMenu:GetParentMenu()
	return self.m_parentMenu
end
function gui.WIContextMenu:GetParentItem()
	return self.m_parentItem
end
function gui.WIContextMenu:CloseActiveSubMenu()
	if util.is_valid(self.m_activeSubMenu) == false then
		return
	end
	local pSubMenu = self.m_activeSubMenu
	pSubMenu:CloseActiveSubMenu()
	pSubMenu:SetVisible(false)
	self:RequestFocus()
	self.m_activeSubMenu = nil
end
function gui.WIContextMenu:RemoveItem(item)
	for i, el in ipairs(self.m_tItems) do
		if el:IsValid() and el == item then
			el:Remove()
			table.remove(self.m_tItems, i)
			return
		end
	end
end
function gui.WIContextMenu:RemoveSubMenu(menu)
	for i, el in ipairs(self.m_subMenues) do
		if el:IsValid() and el == menu then
			el:Remove()
			table.remove(self.m_subMenues, i)
			break
		end
	end
	local item = menu:GetParentItem()
	if util.is_valid(item) then
		self:RemoveItem(item)
	end
end
function gui.WIContextMenu:FindItemByName(name)
	local children = self.m_contents:FindChildrenByName(name)
	return children[1]
end
function gui.WIContextMenu:FindSubMenuByName(name)
	local item = self:FindItemByName(name)
	if util.is_valid(item) == false then
		return
	end
	local subMenu = self.m_itemToSubMenu[item]
	if util.is_valid(subMenu) == false then
		return
	end
	return item, subMenu
end
function gui.WIContextMenu:AddSubMenu(name, onClick, fPopulate)
	local pSubMenu
	local pItem = self:AddItem(name, onClick or function()
		return false
	end)
	if pItem == nil then
		return
	end
	local isPopulated = (fPopulate == nil)
	pItem:AddCallback("OnCursorEntered", function()
		if util.is_valid(pSubMenu) then
			self:CloseActiveSubMenu()
			if isPopulated == false then
				fPopulate(pItem, pSubMenu)
				isPopulated = true
			end
			local id
			local itemName = pItem:GetName()
			if #itemName > 0 then
				id = "context_menu_" .. itemName
			end
			if id == nil then
				local depth = 1
				local parentMenu = self:GetParentMenu()
				while util.is_valid(parentMenu) do
					depth = depth + 1
					parentMenu = parentMenu:GetParentMenu()
				end
				id = "context_menu_" .. tostring(depth)
			end
			pSubMenu:SetName(id)
			pSubMenu:SetVisible(true)
			self.m_activeSubMenu = pSubMenu
			local pos = pItem:GetAbsolutePos()
			pSubMenu:SetX(pos.x + self:GetWidth())
			pSubMenu:SetY(pos.y)
			pSubMenu:UpdateFlipState()
			--pSubMenu:RequestFocus()
		end
	end)
	pItem:AddCallback("OnCursorExited", function()
		if util.is_valid(pSubMenu) then
			if pSubMenu:IsCursorInBounds() then
				pItem:KillFocus()
				pSubMenu:RequestFocus()
			elseif pSubMenu == self.m_activeSubMenu then
				self:CloseActiveSubMenu()
			end
		end
	end)
	pSubMenu = gui.create("WIContextMenu", self:GetParent())
	pSubMenu:SetParentMenu(self, pItem)
	pSubMenu:AddCallback("OnCursorExited", function()
		pSubMenu:KillFocus()
		pSubMenu:SetVisible(false)
		self:RequestFocus()
	end)
	pSubMenu:SetVisible(false)
	pItem:RemoveElementOnRemoval(pSubMenu)
	table.insert(self.m_subMenues, pSubMenu)
	self.m_itemToSubMenu[pItem] = pSubMenu

	local pIcon = gui.create("WIArrow", pItem)
	local function updateIcon()
		pIcon:CenterToParentY()
		pIcon:SetX(pItem:GetWidth() - pIcon:GetWidth() - 5)
	end
	updateIcon()
	pItem:AddCallback("SetSize", updateIcon)
	pIcon:SetDirection(gui.Arrow.DIRECTION_RIGHT)
	pIcon:AddStyleClass("context_menu_arrow")

	return pItem, pSubMenu
end
local function get_base_element(window)
	local typeName = util.get_type_name(window)
	if typeName == "Root" then
		return window
	end
	if typeName ~= "Window" then
		local elBase = window:GetRootElement()
		if util.is_valid(elBase) == false or util.get_type_name(elBase) ~= "Root" then
			return
		end
		return elBase
	end
	if util.is_valid(window) == false then
		window = gui.find_focused_window()
		if util.is_valid(window) == false then
			window = gui.get_primary_window()
		end
	end
	if util.is_valid(window) == false then
		return
	end
	local elBase = gui.get_base_element(window)
	if util.is_valid(elBase) == false or util.get_type_name(elBase) ~= "Root" then
		return
	end
	return elBase
end
gui.close_all_context_menues = function()
	for _, elMenu in pairs(gui.impl.contextMenu.menues) do
		if elMenu:IsValid() then
			elMenu:RemoveSafely()
		end
	end
	gui.impl.contextMenu.menues = {}
end
gui.close_context_menu = function(window)
	window = window or gui.get_base_element()
	local elBase = get_base_element(window)
	if util.is_valid(elBase) == false then
		return
	end

	if util.is_valid(gui.impl.contextMenu.menues[elBase]) == false then
		return
	end
	gui.impl.contextMenu.menues[elBase]:RemoveSafely()
	gui.impl.contextMenu.menues[elBase] = nil
end
gui.open_context_menu = function(window)
	window = window or gui.get_base_element()
	gui.close_context_menu(window)
	local elBase = get_base_element(window)
	if util.is_valid(elBase) == false then
		return
	end
	local menu = gui.create("WIContextMenu", elBase)
	if menu ~= nil then
		menu:SetName("context_menu")
		menu:RequestFocus()
		menu:SetPos(elBase:GetCursorPos())
		gui.impl.contextMenu.menues[elBase] = menu
	end
	return menu
end
gui.is_context_menu_open = function(elBase)
	elBase = elBase or gui.get_base_element()
	if util.is_valid(elBase) == false then
		return false
	end
	return util.is_valid(gui.impl.contextMenu.menues[elBase])
end
gui.set_context_menu_skin = function(skin)
	gui.impl.contextMenu.skin = skin
end
gui.register("WIContextMenu", gui.WIContextMenu)
