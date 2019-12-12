include("wimenuitem.lua")

util.register_class("gui.WIContextMenu",gui.Base)

local contextMenu
local cbMouseInput
local numMenues = 0
function gui.WIContextMenu:__init()
	gui.Base.__init(self)
end
function gui.WIContextMenu:OnRemove()
	numMenues = numMenues -1
	if(numMenues == 0 and util.is_valid(cbMouseInput) == true) then cbMouseInput:Remove() end
end
function gui.WIContextMenu:OnInitialize()
	numMenues = numMenues +1
	gui.Base.OnInitialize(self)

	self.m_tItems = {}
	self.m_subMenues = {}
	local pBg = gui.create("WIRect",self)
	pBg:SetAutoAlignToParent(true)
	self:SetKeyboardInputEnabled(true)
	pBg:SetColor(Color.Beige)
	self.m_pBg = pBg

	local pBgOutline = gui.create("WIOutlinedRect",self)
	pBgOutline:SetAutoAlignToParent(true)
	pBgOutline:SetColor(Color.Gray)
	self.m_pBgOutline = pBgOutline
	if(util.is_valid(cbMouseInput) == false) then
		cbMouseInput = input.add_callback("OnMouseInput",function(button,action,mods)
			if(action == input.STATE_PRESS) then
				if(contextMenu ~= nil and contextMenu:IsCursorInMenuBounds() == true) then return end
				gui.close_context_menu()
			end
		end)
	end
end
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
function gui.WIContextMenu:AddItem(name,fcOnClick,keybind)
	local pItem = gui.create("WIMenuItem",self)
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
	table.insert(self.m_tItems,pItem)
	return pItem
end
function gui.WIContextMenu:AddSubMenu(name)
	local pSubMenu
	local pItem = self:AddItem(name,function() return false end)
	if(pItem == nil) then return end
	pItem:AddCallback("OnCursorEntered",function()
		if(util.is_valid(pSubMenu)) then
			pSubMenu:SetVisible(true)
			local pos = pItem:GetAbsolutePos()
			pSubMenu:SetX(pos.x +pItem:GetWidth())
			pSubMenu:SetY(pos.y)
			--pSubMenu:RequestFocus()
		end
	end)
	pItem:AddCallback("OnCursorExited",function()
		if(util.is_valid(pSubMenu)) then
			if(pSubMenu:IsCursorInBounds()) then
				pItem:KillFocus()
				pSubMenu:RequestFocus()
			else pSubMenu:SetVisible(false) end
		end
	end)
	pSubMenu = gui.create("WIContextMenu")
	pSubMenu:AddCallback("OnCursorExited",function()
		pSubMenu:KillFocus()
		pSubMenu:SetVisible(false)
		self:RequestFocus()
	end)
	pSubMenu:SetVisible(false)
	pItem:RemoveElementOnRemoval(pSubMenu)
	table.insert(self.m_subMenues,pSubMenu)
	return pItem,pSubMenu
end
function gui.WIContextMenu:Update()
	local yOffset = 0
	local wItem = 256
	local hItem = 16
	for _,pItem in ipairs(self.m_tItems) do
		if(util.is_valid(pItem) == true) then
			pItem:SetWidth(wItem)
			pItem:SetHeight(hItem)
			pItem:SetY(yOffset)
			yOffset = yOffset +hItem
		end
	end
	self:SetSize(wItem,yOffset)
end
gui.close_context_menu = function()
	if(contextMenu == nil or util.is_valid(contextMenu) == false) then return end
	contextMenu:RemoveSafely()
	contextMenu = nil
end
gui.open_context_menu = function()
	gui.close_context_menu()
	contextMenu = gui.create("WIContextMenu")
	if(contextMenu ~= nil) then
		contextMenu:RequestFocus()
		contextMenu:SetPos(input.get_cursor_pos())
	end
	return contextMenu
end
gui.is_context_menu_open = function()
	return (contextMenu ~= nil and util.is_valid(contextMenu) == true) and true or false
end
gui.register("WIContextMenu",gui.WIContextMenu)
