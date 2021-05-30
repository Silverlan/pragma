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
	self.m_pMain = pMain

	self.m_tItems = {}
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
end
function gui.WIMenuBar:SetBackgroundColor(col) self.m_pMain:SetColor(col) end
function gui.WIMenuBar:IsContextMenuOpen()
	for _,item in ipairs(self.m_tItems) do
		if(item:IsValid() and item:IsContextMenuOpen()) then return true end
	end
	return false
end
function gui.WIMenuBar:AddItem(name,fcContextCallback)
	local pItem = gui.create("WIMenuItem",self)
	if(util.is_valid(pItem) == false) then return end
	pItem:SetTitle(name)
	pItem:SetContextMenuHandler(fcContextCallback)
	pItem:AddCallback("OnSelectionChanged",function(el,selected)
		if(self:IsValid() == false or self:IsContextMenuOpen() == false) then return end
		if(selected) then pItem:OpenContextMenu() end
	end)
	table.insert(self.m_tItems,pItem)
	return pItem
end
gui.register("WIMenuBar",gui.WIMenuBar)
