util.register_class("gui.WIToolBar", gui.Base)
function gui.WIToolBar:__init()
	gui.Base.__init(self)
	self.m_tTools = {}
end
function gui.WIToolBar:OnInitialize()
	gui.Base.OnInitialize(self)

	self.m_selectedTool = 0
	local pMain = gui.create("WIRect", self)
	pMain:SetColor(Color.Beige)
	pMain:SetAutoAlignToParent(true)
	pMain:Update()
	self.m_pMain = pMain
end
function gui.WIToolBar:SelectTool(identifier)
	local tool = self.m_tTools[identifier]
	if tool == nil then
		return
	end
	tool:Select()
end
function gui.WIToolBar:AddTool(identifier, x, y, w, h)
	local el = gui.create("WIToolBarTool", self)
	if el == nil then
		return
	end
	el:SetSize(256, 256)
	el:SetIdentifier(identifier)
	el:SetClipping(x, y, w, h)
	table.insert(self.m_tTools, { el, identifier })
	el:AddCallback("OnSelected", function(el)
		if util.is_valid(self) == false then
			return
		end
		for _, toolOther in ipairs(self.m_tTools) do
			local pToolOther = toolOther[1]
			if pToolOther ~= el then
				pToolOther:Deselect()
			end
		end
		self.m_selectedTool = el:GetIdentifier()
	end)
	return el
end
function gui.WIToolBar:GetSelectedTool()
	return self.m_selectedTool
end
function gui.WIToolBar:Update()
	local w = self:GetWidth()
	local h = w
	local x = 0
	local y = 0
	for _, tool in ipairs(self.m_tTools) do
		local pTool = tool[1]
		if util.is_valid(pTool) then
			pTool:SetPos(x, y)
			pTool:SetSize(w, h)
			y = y + h
		end
	end
end
gui.register("WIToolBar", gui.WIToolBar)

--------------------------------

util.register_class("gui.WIToolBarTool", gui.Base)
function gui.WIToolBarTool:__init()
	gui.Base.__init(self)
end
function gui.WIToolBarTool:SetIdentifier(identifier)
	self.m_identifier = identifier
end
function gui.WIToolBarTool:GetIdentifier()
	return self.m_identifier
end
function gui.WIToolBarTool:Select()
	if self:IsSelected() == true then
		return
	end
	self.m_bSelected = true
	self:ResetBackgroundColor()
	self:CallCallbacks("OnSelected")
end
function gui.WIToolBarTool:Deselect()
	if self:IsSelected() == false then
		return
	end
	self.m_bSelected = false
	self:ResetBackgroundColor()
	self:CallCallbacks("OnDeselected")
end
function gui.WIToolBarTool:SetClipping(x, y, w, h)
	if util.is_valid(self.m_pMain) == false then
		return
	end
	self.m_pMain:SetClipping(x, y, w, h)
end
function gui.WIToolBarTool:IsSelected()
	return self.m_bSelected
end
function gui.WIToolBarTool:ResetBackgroundColor()
	if self:IsSelected() == true then
		self:SetBackgroundColor(Color.Azure)
	else
		self:SetBackgroundColor(Color.Beige)
	end
end
function gui.WIToolBarTool:OnInitialize()
	gui.Base.OnInitialize(self)

	self.m_bEnabled = true
	self.m_bSelected = false
	self.m_identifier = ""
	self:SetMouseInputEnabled(true)

	local pBg = gui.create("WIRect", self)
	pBg:SetAutoAlignToParent(true)
	self.m_pBg = pBg

	local pMain = gui.create("WIIcon", self)
	pMain:SetMaterial("wgui/level_editor/editor_icons")
	self.m_pMain = pMain

	self:ResetBackgroundColor()
end
function gui.WIToolBarTool:SetEnabled(b)
	self.m_bEnabled = b
	if b == false and self:IsSelected() then
		self:Deselect()
	end
	if util.is_valid(self.m_pMain) == false then
		return
	end
	self.m_pMain:SetColor((b == true) and Color.White or Color.DarkSlateGray)
end
function gui.WIToolBarTool:IsEnabled()
	return self.m_bEnabled
end
function gui.WIToolBarTool:OnSizeChanged(w, h)
	if util.is_valid(self.m_pMain) == true then
		local wIcon = w * 0.8
		local hIcon = h * 0.8
		self.m_pMain:SetSize(wIcon, hIcon)
		self.m_pMain:SetPos((w - wIcon) * 0.5, (h - hIcon) * 0.5)
	end
end
function gui.WIToolBarTool:SetBackgroundColor(col)
	if util.is_valid(self.m_pBg) == false then
		return
	end
	self.m_pBg:SetColor(col)
end
function gui.WIToolBarTool:OnCursorEntered()
	if util.is_valid(self.m_pBg) == false or self:IsEnabled() == false then
		return
	end
	self:SetBackgroundColor(Color.GhostWhite)
end
function gui.WIToolBarTool:OnCursorExited()
	if util.is_valid(self.m_pBg) == false or self:IsEnabled() == false then
		return
	end
	self:ResetBackgroundColor()
end
function gui.WIToolBarTool:OnMouseEvent(mouseButton, keyState, modifier)
	if self:IsEnabled() == false then
		return
	end
	if mouseButton == input.MOUSE_BUTTON_LEFT and keyState == input.STATE_PRESS then
		self:Select()
	end
end
gui.register("WIToolBarTool", gui.WIToolBarTool)
