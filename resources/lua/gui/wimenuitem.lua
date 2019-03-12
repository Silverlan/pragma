
util.register_class("gui.WIMenuItem",gui.Base)
function gui.WIMenuItem:__init()
	gui.Base.__init(self)
end
function gui.WIMenuItem:OnInitialize()
	gui.Base.OnInitialize(self)

	self:SetMouseInputEnabled(true)
	local pBg = gui.create("WIRect",self)
	pBg:SetAutoAlignToParent(true)
	pBg:SetColor(Color.SkyBlue)
	self.m_pBg = pBg

	local pBgOutline = gui.create("WIOutlinedRect",self)
	pBgOutline:SetAutoAlignToParent(true)
	pBgOutline:SetColor(Color.RoyalBlue)
	self.m_pBgOutline = pBgOutline

	local pText = gui.create("WIText",self)
	pText:SetColor(Color.Black)
	self.m_pText = pText

	self:SetSelected(false)
end

function gui.WIMenuItem:SetRightText(text)
	if(text == nil or text == "") then
		if(util.is_valid(self.m_pTextRight)) then self.m_pTextRight:Remove() end
		return
	end
	if(util.is_valid(self.m_pTextRight) == false) then
		local pText = gui.create("WIText",self)
		pText:SetColor(Color.Black)
		self.m_pTextRight = pText
	end
	self.m_pTextRight:SetText(text)
	self.m_pTextRight:SizeToContents()
	self:UpdateRightText()
end
function gui.WIMenuItem:SetSelected(b)
	self.m_bSelected = b
	if(util.is_valid(self.m_pBg) == true) then self.m_pBg:SetVisible(b) end
	if(util.is_valid(self.m_pBgOutline) == true) then self.m_pBgOutline:SetVisible(b) end
end
function gui.WIMenuItem:IsSelected() return self.m_bSelected or false end
function gui.WIMenuItem:OnCursorEntered()
	self:SetSelected(true)
end
function gui.WIMenuItem:OnCursorExited()
	self:SetSelected(false)
end
function gui.WIMenuItem:SetAction(fcOnClick) self.m_fcAction = fcOnClick end
function gui.WIMenuItem:MouseCallback(mouseButton,keyState,modifier)
	if(mouseButton == input.MOUSE_BUTTON_LEFT and keyState == input.STATE_PRESS) then
		if(self.m_fcAction ~= nil) then self.m_fcAction(self) end
	end
end
function gui.WIMenuItem:SetKeybindCommand(cmd)
	self.m_keyboardCommand = cmd
end
function gui.WIMenuItem:GetKeybindCommand() return self.m_keyboardCommand end
function gui.WIMenuItem:SetTitle(title)
	if(util.is_valid(self.m_pText) == false) then return end
	self.m_pText:SetText(title)
	self.m_pText:SizeToContents()
end
local border = 8
function gui.WIMenuItem:UpdateRightText()
	if(util.is_valid(self.m_pTextRight) == false) then return end
	self.m_pTextRight:SetX(self:GetWidth() -self.m_pTextRight:GetWidth() -border)
	self.m_pTextRight:SetY(self:GetHeight() *0.5 -self.m_pTextRight:GetHeight() *0.5)
end
function gui.WIMenuItem:OnSizeChanged(width,height)
	self:UpdateRightText()
end
function gui.WIMenuItem:SizeToContents()
	if(util.is_valid(self) == false or util.is_valid(self.m_pText) == false) then return end
	local sz = self.m_pText:GetSize()
	sz.x = sz.x +border *2
	self:SetSize(sz)
	self.m_pText:SetX(border)
	self.m_pText:SetY(self:GetHeight() *0.5 -self.m_pText:GetHeight() *0.5)
end
gui.register("WIMenuItem",gui.WIMenuItem)
