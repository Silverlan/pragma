--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("hbox.lua")
include("wicontextmenu.lua")

util.register_class("gui.WIMenuBar", gui.HBox)

local get_event
function gui.WIMenuBar:OnRemove() end
function gui.WIMenuBar:OnInitialize()
	gui.HBox.OnInitialize(self)

	self:SetSize(128, 24)

	self:SetName("menu_bar")
	local pMain = gui.create("WIRect")
	self:SetBackgroundElement(pMain)
	pMain:SetParent(self)
	pMain:SetColor(Color.Beige)
	pMain:AddStyleClass("menu_bar_background")
	pMain:SetMouseInputEnabled(true)
	pMain:AddCallback("OnMouseEvent", function(el, mouseButton, keyState, mods)
		if mouseButton == input.MOUSE_BUTTON_LEFT then
			if keyState == input.STATE_PRESS then
				self.m_dragInitialWindowPos = gui.get_primary_window():GetPos()
				self.m_dragInitialCursorPos = self.m_dragInitialWindowPos + input.get_cursor_pos()
				self:UpdateThinkState()
			else
				self.m_dragInitialCursorPos = nil
				self.m_dragInitialWindowPos = nil
				self:UpdateThinkState()
			end
			return util.EVENT_REPLY_HANDLED
		end
		return util.EVENT_REPLY_HANDLED
	end)

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
	pMain:SetAnchor(0, 0, 1, 1)

	--[[pClose:SetSize(self:GetHeight(),self:GetHeight())
	pClose:SetRight(self:GetRight())
	pClose:SetAnchor(1,0,1,0)]]
	self:AddStyleClass("menu_bar")
	self:ScheduleUpdate()

	self.m_eventData = get_event()
	self:UpdateThinkState()
end
local function get_interpolated_color(colors, factor)
	local colIndex0 = math.floor(factor * #colors)
	local colIndex1 = (colIndex0 + 1) % #colors

	local col0 = colors[colIndex0 + 1]
	local col1 = colors[colIndex1 + 1]

	return col0:Lerp(col1, factor * #colors - colIndex0)
end
function gui.WIMenuBar:UpdateWindowColors()
	if self.m_eventData == nil then
		return
	end
	local dt = time.real_time()

	if self.m_eventData.titleBarColors ~= nil then
		local colors = self.m_eventData.titleBarColors
		local duration = (#colors - 1) * 30 -- 30 seconds per color
		local factor = (dt % duration) / duration
		gui.get_primary_window():SetTitleBarColor(get_interpolated_color(colors, factor))
	end
	if self.m_eventData.outlineColors ~= nil then
		local colors = self.m_eventData.outlineColors
		local duration = (#colors - 1) * 30 -- 30 seconds per color
		local factor = (dt % duration) / duration
		gui.get_primary_window():SetBorderColor(get_interpolated_color(colors, factor))
	end
end
function gui.WIMenuBar:OnThink()
	if self.m_dragInitialCursorPos ~= nil then
		local newCursorPos = gui.get_primary_window():GetPos() + input.get_cursor_pos()
		local dtPos = newCursorPos - self.m_dragInitialCursorPos
		local window = gui.get_primary_window()
		window:SetPos(self.m_dragInitialWindowPos + dtPos)
	end

	self:UpdateWindowColors()
end
function gui.WIMenuBar:UpdateThinkState()
	if self.m_eventData ~= nil or self.m_dragInitialWindowPos ~= nil then
		self:SetThinkingEnabled(true)
		return
	end
	self:SetThinkingEnabled(false)
end
function gui.WIMenuBar:SetBackgroundColor(col)
	self.m_pMain:SetColor(col)
end
function gui.WIMenuBar:IsContextMenuOpen()
	for _, item in ipairs(self.m_tItems) do
		if item:IsValid() and item:IsContextMenuOpen() then
			return true
		end
	end
	return false
end
function gui.WIMenuBar:UpdateItem(item)
	item:SetHeight(self:GetHeight())

	local elText = item:GetTextElement()
	if util.is_valid(elText) then
		elText:SizeToContents()
		elText:CenterToParent()

		item:SetWidth(elText:GetWidth() + 10)
	end

	item:Update()
end
function gui.WIMenuBar:OnSizeChanged(w, h)
	if self.m_tItems == nil then
		return
	end
	self:OnUpdate()
end
function gui.WIMenuBar:OnUpdate()
	if self.m_tItems == nil then
		return
	end
	local x = 0
	for _, item in ipairs(self.m_tItems) do
		if item:IsValid() then
			item:SetX(x)
			item:SetHeight(self:GetHeight())

			local elText = item:GetTextElement()
			elText:SizeToContents()

			item:SetWidth(elText:GetWidth() + 20)
			item:Update()

			elText:CenterToParent()
			x = x + item:GetWidth()
		end
	end
end
function gui.WIMenuBar:FindItemByIdentifier(identifier)
	return self.m_idToItem[identifier]
end
function gui.WIMenuBar:AddItem(name, fcContextCallback, identifier)
	local pItem = gui.create("WIMenuItem", self)
	if util.is_valid(pItem) == false then
		return
	end
	pItem:SetTitle(name)
	pItem:SetContextMenuHandler(fcContextCallback)
	pItem:AddCallback("OnSelectionChanged", function(el, selected)
		if self:IsValid() == false or self:IsContextMenuOpen() == false then
			return
		end
		if selected then
			pItem:OpenContextMenu()
		end
	end)
	table.insert(self.m_tItems, pItem)
	self:UpdateItem(pItem)
	if identifier ~= nil then
		self.m_idToItem[identifier] = pItem
	end
	return pItem
end
gui.register("WIMenuBar", gui.WIMenuBar)

get_event = function()
	local events = {
		["trans"] = {
			day = 31,
			month = 3,
			titleBarColors = {
				Color.CreateFromHexColor("5BCEFA"),
				Color.CreateFromHexColor("F5A9B8"),
				Color.CreateFromHexColor("FFFFFF"),
			},
			outlineColors = {
				Color.CreateFromHexColor("5BCEFA"),
				Color.CreateFromHexColor("F5A9B8"),
				Color.CreateFromHexColor("FFFFFF"),
			},
		},
		["lgbt"] = {
			day = 28,
			month = 6,
			titleBarColors = {
				Color.CreateFromHexColor("E40303"),
				Color.CreateFromHexColor("FF8C00"),
				Color.CreateFromHexColor("FFED00"),
				Color.CreateFromHexColor("008026"),
				Color.CreateFromHexColor("24408E"),
				Color.CreateFromHexColor("732982"),
			},
			outlineColors = {
				Color.CreateFromHexColor("E40303"),
				Color.CreateFromHexColor("FF8C00"),
				Color.CreateFromHexColor("FFED00"),
				Color.CreateFromHexColor("008026"),
				Color.CreateFromHexColor("24408E"),
				Color.CreateFromHexColor("732982"),
			},
		},
		["lgbt_month"] = {
			month = 6,
			outlineColors = {
				Color.CreateFromHexColor("E40303"),
				Color.CreateFromHexColor("FF8C00"),
				Color.CreateFromHexColor("FFED00"),
				Color.CreateFromHexColor("008026"),
				Color.CreateFromHexColor("24408E"),
				Color.CreateFromHexColor("732982"),
			},
		},
	}
	local currentDateTime = os.date("*t")
	local currentMonth = currentDateTime.month
	local currentDay = currentDateTime.day
	local fallback
	for name, evData in pairs(events) do
		if evData.month == currentMonth and evData.day == currentDay then
			return evData
		end
		if evData.day == nil and evData.month == currentMonth then
			fallback = evData
		end
	end
	return fallback
end
