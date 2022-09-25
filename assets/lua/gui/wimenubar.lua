--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("hbox.lua")
include("wicontextmenu.lua")

util.register_class("gui.WIMenuBar",gui.HBox)

function gui.WIMenuBar:__init()
	gui.HBox.__init(self)
end
function gui.WIMenuBar:OnRemove()

end
function gui.WIMenuBar:OnInitialize()
	gui.HBox.OnInitialize(self)

	self:SetSize(128,24)

	local pMain = gui.create("WIRect")
	self:SetBackgroundElement(pMain)
	pMain:SetParent(self)
	pMain:SetColor(Color.Beige)
	pMain:AddStyleClass("menu_bar_background")
	self.m_pMain = pMain

	self.m_tItems = {}
	self.m_idToItem = {}
	--[[local pClose = gui.create("WIButton",self)
	pClose:SetText("X")
	pClose:AddCallback("OnPressed",function(pClose)
		if(util.is_valid(self) == false) then return end
		self:CallCallbacks("OnClose")
	end)
	self.m_pClose = pClose]]

	pMain:SetSize(self:GetSize())
	pMain:SetAnchor(0,0,1,1)

	--[[pClose:SetSize(self:GetHeight(),self:GetHeight())
	pClose:SetRight(self:GetRight())
	pClose:SetAnchor(1,0,1,0)]]
	self:AddStyleClass("menu_bar")
	self:ScheduleUpdate()
end
function gui.WIMenuBar:SetBackgroundColor(col) self.m_pMain:SetColor(col) end
function gui.WIMenuBar:IsContextMenuOpen()
	for _,item in ipairs(self.m_tItems) do
		if(item:IsValid() and item:IsContextMenuOpen()) then return true end
	end
	return false
end
function gui.WIMenuBar:UpdateItem(item)
	item:SetHeight(self:GetHeight())

	local elText = item:GetTextElement()
	if(util.is_valid(elText)) then
		elText:SizeToContents()
		elText:CenterToParent()

		item:SetWidth(elText:GetWidth() +10)
	end

	item:Update()
end
function gui.WIMenuBar:OnSizeChanged(w,h)
	if(self.m_tItems == nil) then return end
	self:OnUpdate()
end
function gui.WIMenuBar:OnUpdate()
	if(self.m_tItems == nil) then return end
	local x = 0
	for _,item in ipairs(self.m_tItems) do
		if(item:IsValid()) then
			item:SetX(x)
			item:SetHeight(self:GetHeight())

			local elText = item:GetTextElement()
			elText:SizeToContents()

			item:SetWidth(elText:GetWidth() +20)
			item:Update()

			elText:CenterToParent()
			x = x +item:GetWidth()
		end
	end
end
function gui.WIMenuBar:FindItemByIdentifier(identifier) return self.m_idToItem[identifier] end
function gui.WIMenuBar:AddItem(name,fcContextCallback,identifier)
	local pItem = gui.create("WIMenuItem",self)
	if(util.is_valid(pItem) == false) then return end
	pItem:SetTitle(name)
	pItem:SetContextMenuHandler(fcContextCallback)
	pItem:AddCallback("OnSelectionChanged",function(el,selected)
		if(self:IsValid() == false or self:IsContextMenuOpen() == false) then return end
		if(selected) then pItem:OpenContextMenu() end
	end)
	table.insert(self.m_tItems,pItem)
	self:UpdateItem(pItem)
	if(identifier ~= nil) then self.m_idToItem[identifier] = pItem end
	return pItem
end
gui.register("WIMenuBar",gui.WIMenuBar)
