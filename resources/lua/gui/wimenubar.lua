include("wicontextmenu.lua")

util.register_class("gui.WIMenuBar",gui.Base)

function gui.WIMenuBar:__init()
	gui.Base.__init(self)
end
function gui.WIMenuBar:OnRemove()

end
function gui.WIMenuBar:OnInitialize()
	gui.Base.OnInitialize(self)

	local pMain = gui.create("WIRect",self)
	pMain:SetColor(Color.Beige)
	self.m_pMain = pMain

	self.m_tItems = {}
	local pClose = gui.create("WIButton",self)
	pClose:SetText("X")
	pClose:AddCallback("OnPressed",function(pClose)
		if(util.is_valid(self) == false) then return end
		self:CallCallbacks("OnClose")
	end)
	self.m_pClose = pClose

	self:SetSize(128,24)

	pMain:SetSize(self:GetSize())
	pMain:SetAnchor(0,0,1,1)

	pClose:SetSize(self:GetHeight(),self:GetHeight())
	pClose:SetRight(self:GetRight())
	pClose:SetAnchor(1,0,1,0)
end
function gui.WIMenuBar:AddItem(name,fcContextCallback)
	local pItem = gui.create("WIMenuItem",self)
	if(util.is_valid(pItem) == false) then return end
	pItem:SetTitle(name)
	pItem:SetAction(function()
		if(gui.is_context_menu_open() == true) then return end
		local pContext = gui.open_context_menu()
		if(pContext ~= nil) then
			local pos = pItem:GetAbsolutePos()
			pos.y = pos.y +pItem:GetHeight()
			pContext:SetPos(pos)
			-- pContext:SetZPos(10000)
			fcContextCallback(pContext)
		end
	end)
	table.insert(self.m_tItems,pItem)
	return pItem
end
function gui.WIMenuBar:Update()
	local x = 0
	local h = self:GetHeight()
	for _,pItem in ipairs(self.m_tItems) do
		if(util.is_valid(pItem) == true) then
			pItem:SizeToContents()
			pItem:SetX(x)
			pItem:SetHeight(h)
			x = x +pItem:GetWidth()
		end
	end
end
function gui.WIMenuBar:OnSizeChanged(w,h)
	self:Update()
end
gui.register("WIMenuBar",gui.WIMenuBar)
