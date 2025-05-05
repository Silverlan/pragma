util.register_class("gui.WITabbedPanelTab", gui.Base)
local TAB_HEIGHT = 14
function gui.WITabbedPanelTab:__init()
	gui.Base.__init(self)
end
function gui.WITabbedPanelTab:OnInitialize()
	gui.Base.OnInitialize(self)

	self.m_bSelected = false
	local pTab = gui.create("WIRect", self)
	pTab:SetMouseInputEnabled(true)
	pTab:SetAutoAlignToParent(true)
	pTab:AddCallback("OnMousePressed", function(el)
		if self:IsValid() == false then
			return
		end
		self:SetSelected(true)
	end)
	self.m_pTab = pTab

	local pOutline = gui.create("WIOutlinedRect", pTab)
	pOutline:SetAutoAlignToParent(true)
	pOutline:SetColor(Color(38, 38, 38, 255))

	local pText = gui.create("WIText", pTab)
	pText:AddStyleClass("tab_title")
	pText:SizeToContents()
	self.m_pText = pText

	self:Update()
end
function gui.WITabbedPanelTab:Update()
	if util.is_valid(self.m_pTab) == false then
		return
	end
	self.m_pTab:SetColor((self:IsSelected() == true) and Color(38, 38, 38, 255) or Color(10, 10, 10, 255))
	self.m_pText:SetColor((self:IsSelected() == true) and Color.LightGrey or Color.Gray)
end
function gui.WITabbedPanelTab:SetSelected(b)
	if b == self.m_bSelected then
		return
	end
	self:CallCallbacks("OnSelected", b)
	self.m_bSelected = b
	self:Update()
end
function gui.WITabbedPanelTab:IsSelected()
	return self.m_bSelected
end
function gui.WITabbedPanelTab:GetTabElement()
	return self.m_pTab
end
function gui.WITabbedPanelTab:SetTitle(title)
	if util.is_valid(self.m_pText) == false then
		return
	end
	local margin = 5
	self.m_pText:SetText(title)
	self.m_pText:SizeToContents()
	self:SetSize(self.m_pText:GetWidth() + margin * 2, TAB_HEIGHT + margin * 2)
	self.m_pText:SetPos(margin, self:GetHeight() * 0.5 - self.m_pText:GetHeight() * 0.5)
end
function gui.WITabbedPanelTab:GetTitle()
	if util.is_valid(self.m_pText) == false then
		return ""
	end
	return self.m_pText:GetText()
end
gui.register("WITabbedPanelTab", gui.WITabbedPanelTab)

---------------

util.register_class("gui.WITabbedPanel", gui.Base)
function gui.WITabbedPanel:__init()
	gui.Base.__init(self)
end
function gui.WITabbedPanel:OnInitialize()
	gui.Base.OnInitialize(self)

	self.m_tTabs = {}
	self.m_tabIdentifierToTab = {}
end
function gui.WITabbedPanel:FindTab(name)
	return self.m_tabIdentifierToTab[name]
end
function gui.WITabbedPanel:AddTab(name)
	local t = gui.create("WITabbedPanelTab", self)
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
function gui.WITabbedPanel:Update()
	local sz = self:GetSize()
	local xOffset = 0
	local bGotSelected = false
	for _, tabData in ipairs(self.m_tTabs) do
		local elTab = tabData.tab
		if elTab:IsValid() == true then
			if elTab:IsSelected() == true then
				bGotSelected = true
			end
			elTab:SetPos(xOffset, 0)
			xOffset = xOffset + elTab:GetWidth()
		end
		local elBase = tabData.panel
		if elBase:IsValid() == true then
			elBase:SetPos(0, elTab:GetY() + elTab:GetHeight())
			elBase:SetSize(sz.x, sz.y - elBase:GetY())
		end
	end
	if bGotSelected == false and #self.m_tTabs > 0 then
		local el = self.m_tTabs[1].tab
		if el:IsValid() == true then
			el:SetSelected(true)
		end
	end
end
function gui.WITabbedPanel:OnSizeChanged(w, h)
	self:Update()
end
gui.register("WITabbedPanel", gui.WITabbedPanel)
