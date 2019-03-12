include("/gui/wimenubar.lua")
include("/gui/witoolbar.lua")
include("/gui/wifiledialog.lua")

util.register_class("gui.WIBaseEditor",gui.Base)
function gui.WIBaseEditor:__init()
	gui.Base.__init(self)
end

function gui.WIBaseEditor:OnVisibilityChanged(bVisible)
	if(bVisible == false) then gui.close_context_menu() end
end

function gui.WIBaseEditor:OnInitialize()
	self:SetMouseInputEnabled(true)
	self:TrapFocus(true)
	self:RequestFocus()

	local pMain = gui.create("WIRect",self)
	pMain:SetColor(Color.Beige)--Color(40,40,40,255))
	pMain:SetAutoAlignToParent(true)
	pMain:Update()
	self.m_pMain = pMain

	self.m_menuBar = gui.create("WIMenuBar",self)
	self.m_menuBar:SetHeight(20)
	self.m_menuBar:AddCallback("OnClose",function(pMenuBar)
		if(util.is_valid(self)) then return end
		self:Close()
	end)
end
function gui.WIBaseEditor:OnSizeChanged(w,h)
	if(util.is_valid(self.m_menuBar) == true) then
		self.m_menuBar:SetWidth(w)
	end
end
function gui.WIBaseEditor:Close()
	self:SetVisible(false)
end
function gui.WIBaseEditor:Open()

end
