-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

local TabbedPanelTab = util.register_class("gui.TabbedPanelTab", gui.Base)
local TAB_HEIGHT = 14
function TabbedPanelTab:OnInitialize()
	gui.Base.OnInitialize(self)

	self:SetSize(64,64)

	self.m_bSelected = false
	local pTab = gui.create("WIRect", self)
	pTab:SetMouseInputEnabled(true)
	pTab:SetAutoAlignToParent(true)
	pTab:AddStyleClass("tab_button_background_unpressed")
	pTab:AddCallback("OnMousePressed", function(el)
		if self:IsValid() == false then
			return
		end
		self:SetSelected(true)
	end)
	self.m_pTab = pTab

	local pOutline = gui.create("WIOutlinedRect", pTab, 0, 0, self:GetWidth(), self:GetHeight() +1, 0, 0, 1, 1)
	pOutline:SetColor(Color(38, 38, 38, 255))
	pOutline:AddStyleClass("outline")

	local pText = gui.create("WIText", pTab)
	pText:AddStyleClass("tab_title")
	pText:SetAutoCenterToParent(true)
	self.m_pText = pText

	self:UpdateStyles()
end
function TabbedPanelTab:UpdateStyles()
	if util.is_valid(self.m_pTab) == false then
		return
	end
	
	-- Default colors
	self.m_pTab:SetColor((self:IsSelected() == true) and Color(38, 38, 38, 255) or Color(10, 10, 10, 255))
	-- self.m_pText:SetColor((self:IsSelected() == true) and Color.LightGrey or Color.Gray)

	-- Custom classes for override
	if(self:IsSelected()) then
		self.m_pTab:RemoveStyleClass("tab_button_background_unpressed")
		self.m_pTab:AddStyleClass("tab_button_background_pressed")
	else
		self.m_pTab:RemoveStyleClass("tab_button_background_pressed")
		self.m_pTab:AddStyleClass("tab_button_background_unpressed")
	end
	self.m_pTab:RefreshSkin()
end
function TabbedPanelTab:SetSelected(b)
	if b == self.m_bSelected then
		return
	end
	self:CallCallbacks("OnSelected", b)
	self.m_bSelected = b
	self:UpdateStyles()
end
function TabbedPanelTab:IsSelected()
	return self.m_bSelected
end
function TabbedPanelTab:GetTabElement()
	return self.m_pTab
end
function TabbedPanelTab:SetTitle(title)
	if util.is_valid(self.m_pText) == false then
		return
	end
	local margin = 5
	self.m_pText:SetText(title)
	self:SetSize(self.m_pText:GetWidth() + margin * 2, TAB_HEIGHT + margin * 2)
end
function TabbedPanelTab:GetTitle()
	if util.is_valid(self.m_pText) == false then
		return ""
	end
	return self.m_pText:GetText()
end
gui.register("tabbed_panel_tab", TabbedPanelTab)

---------------

local TabbedPanel = util.register_class("gui.TabbedPanel", gui.Base)
function TabbedPanel:OnInitialize()
	gui.Base.OnInitialize(self)

	self.m_tTabs = {}
	self.m_tabIdentifierToTab = {}
end
function TabbedPanel:FindTab(name)
	return self.m_tabIdentifierToTab[name]
end
function TabbedPanel:AddTab(name)
	local t = gui.create("tabbed_panel_tab", self)
	t:SetTitle(name)
	t:AddCallback("OnSelected", function(t, b)
		if b == false or self:IsValid() == false then
			return
		end
		local idx
		for tIdx, tData in ipairs(self.m_tTabs) do
			if t ~= tData.tab then
				tData.panel:SetVisible(false)
				tData.tab:SetSelected(false)
			else
				tData.panel:SetVisible(true)
				idx = tIdx
			end
		end
		if idx == nil then
			return
		end
		self:CallCallbacks("OnTabSelected", self.m_tTabs[idx].tab, self.m_tTabs[idx].panel)
	end)
	local p = gui.create("WIBase", self)
	p:SetVisible(false)
	p:AddStyleClass("window_frame")
	table.insert(self.m_tTabs, {
		tab = t,
		panel = p,
	})
	self.m_tabIdentifierToTab[name] = t
	return p
end
function TabbedPanel:UpdateLayout()
	local sz = self:GetSize()
	local xOffset = 0
	local bGotSelected = false
	for _, tabData in ipairs(self.m_tTabs) do
		local elTab = tabData.tab
		if elTab:IsValid() == true then
			if elTab:IsSelected() == true then
				bGotSelected = true
			end
			elTab:ApplyPos(xOffset, 0)
			xOffset = xOffset + elTab:GetWidth()
		end
		local elBase = tabData.panel
		if elBase:IsValid() == true then
			elBase:ApplyPos(0, elTab:GetY() + elTab:GetHeight())
			elBase:ApplySize(sz.x, sz.y - elBase:GetY())
		end
	end
	if bGotSelected == false and #self.m_tTabs > 0 then
		local el = self.m_tTabs[1].tab
		if el:IsValid() == true then
			el:SetSelected(true)
		end
	end
end
function TabbedPanel:OnSizeChanged(w, h)
	self:UpdateLayout()
end
gui.register("tabbed_panel", TabbedPanel)
