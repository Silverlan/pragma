--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("wimenuitem.lua")
include("vbox.lua")

util.register_class("gui.WIContextMenu",gui.Base)

gui.impl = gui.impl or {}
gui.impl.contextMenu = gui.impl.contextMenu or {
	activeMenuCount = 0
}

function gui.WIContextMenu:__init()
	gui.Base.__init(self)
end
function gui.WIContextMenu:OnRemove()
	gui.impl.contextMenu.activeMenuCount = gui.impl.contextMenu.activeMenuCount -1
	if(gui.impl.contextMenu.activeMenuCount == 0 and util.is_valid(gui.impl.cbMouseInput) == true) then gui.impl.cbMouseInput:Remove() end
end
function gui.WIContextMenu:OnInitialize()
	gui.impl.contextMenu.activeMenuCount = gui.impl.contextMenu.activeMenuCount +1
	gui.Base.OnInitialize(self)

	self:SetSize(64,128)

	self.m_tItems = {}
	self.m_subMenues = {}
	local pBg = gui.create("WIRect",self)
	pBg:SetBackgroundElement(true)
	pBg:SetAutoAlignToParent(true)
	pBg:AddStyleClass("context_menu_background")
	self:SetKeyboardInputEnabled(true)
	pBg:SetColor(Color.Beige)
	self.m_pBg = pBg

	local pBgOutline = gui.create("WIOutlinedRect",self)
	pBgOutline:SetBackgroundElement(true)
	pBgOutline:SetAutoAlignToParent(true)
	pBgOutline:AddStyleClass("context_menu_outline")
	pBgOutline:SetColor(Color.Gray)
	self.m_pBgOutline = pBgOutline
	if(util.is_valid(gui.impl.cbMouseInput) == false) then
		gui.impl.cbMouseInput = input.add_callback("OnMouseInput",function(button,action,mods)
			if(action == input.STATE_PRESS) then
				local el = gui.get_element_under_cursor(gui.find_focused_window())
				while(util.is_valid(el) and el:GetClass() ~= "wicontextmenu") do el = el:GetParent() end
				if(util.is_valid(el)) then return end
				gui.close_context_menu()
			end
		end)
	end

	local scrollContainer = gui.create("WIScrollContainer",self,0,0,self:GetWidth(),self:GetHeight(),0,0,1,1)
	scrollContainer:AddCallback("SetSize",function(el)
		self.m_contents:SetWidth(el:GetWidth())
	end)
	scrollContainer:GetVerticalScrollBar():SetScrollAmount(1)
	self.m_scrollContainer = scrollContainer

	local contents = gui.create("WIVBox",scrollContainer,0,0,self:GetWidth(),self:GetHeight())
	contents:SetFixedWidth(true)
	contents:AddCallback("SetSize",function(el)
		for _,item in ipairs(self.m_tItems) do
			if(item:IsValid()) then item:SetWidth(el:GetWidth()) end
		end
	end)
	contents:AddCallback("OnUpdated",function(el)
		self.m_scrollContainer:ScheduleUpdate()
	end)
	self.m_contents = contents

	self:AddStyleClass("context_menu")
	if(gui.impl.contextMenu.skin ~= nil) then self:SetSkin(gui.impl.contextMenu.skin) end
end
function gui.WIContextMenu:IsPopulated() return self:GetItemCount() > 0 end
function gui.WIContextMenu:IsCursorInMenuBounds()
	if(self:IsCursorInBounds() == true) then return true end
	for _,subMenu in ipairs(self.m_subMenues) do
		if(subMenu:IsValid() == true and subMenu:IsCursorInMenuBounds() == true) then return true end
	end
	return false
end
function gui.WIContextMenu:GetSelectedItem()
	for _,pItem in ipairs(self.m_tItems) do
		if(pItem:IsValid() == true and pItem:IsSelected() == true) then
			return pItem
		end
	end
	for _,subMenu in ipairs(self.m_subMenues) do
		if(subMenu:IsValid() == true) then
			local pItem = subMenu:GetSelectedItem()
			if(pItem ~= nil) then return pItem end
		end
	end
end
function gui.WIContextMenu:KeyboardCallback(key,scanCode,action,mods)
	local pItem = self:GetSelectedItem()
	if(pItem == nil) then return end
	local cmd = pItem:GetKeybindCommand()
	if(cmd == nil) then return end
	local b,keyStr = input.key_to_text(key)
	engine.bind_key(keyStr,cmd)
	pItem:SetRightText(keyStr)
end
function gui.WIContextMenu:AddLine()
	-- TODO
end
function gui.WIContextMenu:OnUpdate()
	local updateItems = {}
	local function updateAutoSize(item)
		for _,menu in ipairs(item.m_subMenues) do
			updateAutoSize(menu)
		end
		for _,subItem in ipairs(item.m_tItems) do
			if(subItem:IsValid()) then
				subItem:Update()
			end
		end

		item.m_contents:SetAutoSizeToContents(true,false)
		item.m_contents:Update()

		table.insert(updateItems,item.m_scrollContainer)
	end
	updateAutoSize(self)

	local w = 108
	local h = self:GetHeight()
	for _,item in ipairs(self.m_tItems) do
		item:SizeToContents()
		if(item:IsValid()) then w = math.max(w,item:GetWidth()) end
	end
	w = w +20
	if(self.m_contents:GetHeight() <= 128 and self.m_contents:GetHeight() ~= self:GetHeight()) then
		h = self.m_contents:GetHeight()
	end
	self:SetSize(w,h)

	for _,item in ipairs(updateItems) do item:Update() end

	self:UpdateFlipState()
end
function gui.WIContextMenu:UpdateFlipState()
	local xBase = self:GetLeft()
	local yBase = self:GetTop()

	self.m_xFlipped = false
	self.m_yFlipped = false
	if(yBase +self:GetHeight() > self:GetParent():GetBottom()) then
		local y = yBase -self:GetHeight()
		if(#self.m_tItems > 0) then y = y +self.m_tItems[1]:GetHeight() end
		self:SetY(y)
		self.m_yFlipped = true
	else self:SetY(yBase) end
	if(xBase +self:GetWidth() > self:GetParent():GetRight()) then
		if(util.is_valid(self.m_parentMenu)) then self:SetX(self.m_parentMenu:GetX() -self:GetWidth())
		else self:SetX(xBase -self:GetWidth()) end
		self.m_xFlipped = true
	else self:SetX(xBase) end
end
function gui.WIContextMenu:GetItemCount() return #self.m_tItems end
function gui.WIContextMenu:Clear()
	for _,item in ipairs(self.m_tItems) do
		util.remove(item)
	end
end
function gui.WIContextMenu:AddItem(name,fcOnClick,keybind)
	local pItem = gui.create("WIMenuItem",self.m_contents)
	if(pItem == nil) then return end
	pItem:SetTitle(name)
	if(keybind ~= nil) then
		pItem:SetKeybindCommand(keybind)
		local mappedKeys = input.get_mapped_keys(keybind)
		if(#mappedKeys > 0) then
			local b,keyStr = input.key_to_text(mappedKeys[1])
			pItem:SetRightText(keyStr)
		end
	end
	pItem:SetAction(function(pItem)
		if(fcOnClick ~= nil) then
			if(fcOnClick(pItem) == false) then return end
		end
		gui.close_context_menu()
	end)
	pItem:SizeToContents()
	table.insert(self.m_tItems,pItem)
	return pItem
end
function gui.WIContextMenu:GetContents() return self.m_contents end
function gui.WIContextMenu:SetParentMenu(menu) self.m_parentMenu = menu end
function gui.WIContextMenu:GetParentMenu() return self.m_parentMenu end
function gui.WIContextMenu:AddSubMenu(name,onClick)
	local pSubMenu
	local pItem = self:AddItem(name,onClick or function() return false end)
	if(pItem == nil) then return end
	pItem:AddCallback("OnCursorEntered",function()
		if(util.is_valid(pSubMenu)) then
			pSubMenu:SetVisible(true)
			local pos = pItem:GetAbsolutePos()
			pSubMenu:SetX(pos.x +self:GetWidth())
			pSubMenu:SetY(pos.y)
			pSubMenu:UpdateFlipState()
			--pSubMenu:RequestFocus()
		end
	end)
	pItem:AddCallback("OnCursorExited",function()
		if(util.is_valid(pSubMenu)) then
			if(pSubMenu:IsCursorInBounds()) then
				pItem:KillFocus()
				pSubMenu:RequestFocus()
			else
				pSubMenu:SetVisible(false)
				self:RequestFocus()
			end
		end
	end)
	pSubMenu = gui.create("WIContextMenu",self:GetParent())
	pSubMenu:SetParentMenu(self)
	pSubMenu:AddCallback("OnCursorExited",function()
		pSubMenu:KillFocus()
		pSubMenu:SetVisible(false)
		self:RequestFocus()
	end)
	pSubMenu:SetVisible(false)
	pItem:RemoveElementOnRemoval(pSubMenu)
	table.insert(self.m_subMenues,pSubMenu)

	local pIcon = gui.create("WIArrow",pItem)
	local function updateIcon()
		pIcon:CenterToParentY()
		pIcon:SetX(pItem:GetWidth() -pIcon:GetWidth() -5)
	end
	updateIcon()
	pItem:AddCallback("SetSize",updateIcon)
	pIcon:SetDirection(gui.Arrow.DIRECTION_RIGHT)
	pIcon:AddStyleClass("context_menu_arrow")

	return pItem,pSubMenu
end
gui.close_context_menu = function()
	if(gui.impl.contextMenu.menu == nil or util.is_valid(gui.impl.contextMenu.menu) == false) then return end
	gui.impl.contextMenu.menu:RemoveSafely()
	gui.impl.contextMenu.menu = nil
end
gui.open_context_menu = function(window)
	gui.close_context_menu()
	if(util.is_valid(window) == false) then
		window = gui.find_focused_window()
		if(util.is_valid(window) == false) then window = gui.get_primary_window() end
	end
	if(util.is_valid(window) == false) then return end
	local elBase = gui.get_base_element(window)
	if(util.is_valid(elBase) == false) then return end
	gui.impl.contextMenu.menu = gui.create("WIContextMenu",elBase)
	if(gui.impl.contextMenu.menu ~= nil) then
		gui.impl.contextMenu.menu:RequestFocus()
		gui.impl.contextMenu.menu:SetPos(elBase:GetCursorPos())
	end
	return gui.impl.contextMenu.menu
end
gui.is_context_menu_open = function()
	return (gui.impl.contextMenu.menu ~= nil and util.is_valid(gui.impl.contextMenu.menu) == true) and true or false
end
gui.set_context_menu_skin = function(skin) gui.impl.contextMenu.skin = skin end
gui.register("WIContextMenu",gui.WIContextMenu)
