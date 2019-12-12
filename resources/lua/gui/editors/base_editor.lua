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
	self.m_menuBar:AddCallback("OnClose",function(pMenuBar)
		if(util.is_valid(self)) then return end
		self:Close()
	end)

	self:SetSize(1024,768)
	self.m_menuBar:SetSize(self:GetWidth(),20)
	self.m_menuBar:SetAnchor(0,0,1,0)
end

function gui.WIBaseEditor:GetMenuBar() return self.m_menuBar end
function gui.WIBaseEditor:CreateWindow(class)
	local pFrame = gui.create("WIFrame")
	if(pFrame == nil) then return end
	local p = gui.create(class or "WIEditorWindow",pFrame)
	if(p == nil) then
		pFrame:Remove()
		return
	end
	p:SetFrame(pFrame)
	return p
end

function gui.WIBaseEditor:Close()
	self:Remove()
end
function gui.WIBaseEditor:Open()

end
