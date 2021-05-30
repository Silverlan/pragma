util.register_class("gui.WIGridView",gui.Base)

local UNIT_SIZE = 7.0 -- Size of 1 world unit in screen pixels with no zoom (width and height)

local LINES_HORIZONTAL = 1
local LINES_VERTICAL = 2
local LINES = {LINES_HORIZONTAL,LINES_VERTICAL}

local AXIS_X = 1
local AXIS_Y = 2
local INVERT_AXES = {
	[AXIS_X] = false,
	[AXIS_Y] = true
} -- Should the axis be inverted? (e.g. left to right = -x instead of +x)

local function get_line_count(sz,unitSize,gridSize)
	return {math.ceil(math.floor(sz.y /unitSize) /gridSize),math.ceil(math.floor(sz.x /unitSize) /gridSize)}
end

function gui.WIGridView:__init()
	gui.Base.__init(self)
	self.m_tLines = {{},{}} -- Horizontal and vertical lines
	self.m_posWorld = Vector(0,0,0)
	self.m_zoomFactor = 1.0
	self.m_gridSize = 1
end
function gui.WIGridView:OnInitialize()
	gui.Base.OnInitialize(self)

	self.m_tOriginLines = {}
	local col = Color.Lime:Copy()
	col.a = 200
	local pLine1 = gui.create("WIRect",self)
	pLine1:SetSize(Vector2(64,3))
	pLine1:SetColor(col)
	pLine1:SetZPos(1)
	table.insert(self.m_tOriginLines,pLine1)

	local pLine2 = gui.create("WIRect",self)
	pLine2:SetSize(Vector2(3,64))
	pLine2:SetColor(col)
	pLine2:SetZPos(1)
	table.insert(self.m_tOriginLines,pLine2)
end
function gui.WIGridView:SetZoomFactor(factor) self.m_zoomFactor = factor end
function gui.WIGridView:GetZoomFactor() return self.m_zoomFactor end
function gui.WIGridView:SetGridSize(size) self.m_gridSize = size end
function gui.WIGridView:GetGridSize() return self.m_gridSize end
function gui.WIGridView:GetMinGridSize() return math.floor(self.m_zoomFactor) end
function gui.WIGridView:GetClampedGridSize() return math.max(self:GetGridSize(),self:GetMinGridSize()) end
function gui.WIGridView:GetUnitSize() return UNIT_SIZE /self:GetZoomFactor() end
function gui.WIGridView:SetWorldOrigin(pos)
	self.m_posWorld = pos:Copy()
end
function gui.WIGridView:GetWorldOrigin() return self.m_posWorld end
function gui.WIGridView:GetWorldBounds()
	local origin = self:GetWorldOrigin()
	local halfExtents = (self:GetSize() /self:GetUnitSize()) *0.5
	return halfExtents
end
function gui.WIGridView:ToLocalPosition(pos)
	pos = Vector2(pos.x,pos.y)
	local origin = self:GetWorldOrigin()
	origin = Vector2(origin.x,origin.y)
	local halfExtents = self:GetWorldBounds()
	local min = origin -halfExtents
	pos = pos -min

	pos.x = (pos.x /(halfExtents.x *2.0)) *self:GetWidth()
	if(INVERT_AXES[AXIS_X] == true) then pos.x = self:GetWidth() -pos.x end

	pos.y = (pos.y /(halfExtents.y *2.0)) *self:GetHeight()
	if(INVERT_AXES[AXIS_Y] == true) then pos.y = self:GetHeight() -pos.y end
	return pos
end
function gui.WIGridView:ToWorldPosition(pos)
	local sz = self:GetSize()
	local posRelative = Vector2(0,0)

	if(INVERT_AXES[AXIS_X] == true) then posRelative.x = (self:GetWidth() -pos.x) /sz.x
	else posRelative.x = pos.x /sz.x end

	if(INVERT_AXES[AXIS_Y] == true) then posRelative.y = (self:GetHeight() -pos.y) /sz.y
	else posRelative.y = pos.y /sz.y end

	local origin = self:GetWorldOrigin()
	local halfExtents = self:GetWorldBounds()
	local min = Vector2(origin.x,origin.y) -halfExtents
	local extents = halfExtents *2.0
	local r = min +Vector2(extents.x *posRelative.x,extents.y *posRelative.y)
	return Vector2(r.x,r.y)
end
function gui.WIGridView:UpdateLines() -- Make sure we have enough line elements allocated
	local sz = self:GetSize()
	local numLines = get_line_count(self:GetSize(),self:GetUnitSize(),self:GetClampedGridSize())
	for i=1,#numLines do
		local am = numLines[i] -#self.m_tLines[i]
		local size = (i == 1) and Vector2(sz.x,1.0) or Vector2(1.0,sz.y)
		-- Update size of our existing lines
		for j=1,math.min(#self.m_tLines[i],numLines[i]) do
			local pLine = self.m_tLines[i][j]
			if(util.is_valid(pLine) == true) then
				pLine:SetSize(size)
			end
		end
		-- Allocate new lines if required
		for j=1,am do
			local pLine = gui.create("WIRect",self)
			pLine:SetSize(size)
			pLine:Update()
			table.insert(self.m_tLines[i],pLine)
		end
	end
end
function gui.WIGridView:GetLineColor(c)
	c = math.round(c)
	if(c == 0.0) then return Color.Turquoise -- Line goes through the world origin
	elseif(c %16384.0 == 0.0) then return Color.OrangeRed
	elseif(c %1024.0 == 0.0) then return Color.Brown
	elseif(c %64.0 == 0.0) then return Color.Olive
	elseif(c %8.0 == 0.0) then return Color.LightGrey end
	return Color.Gray
end
function gui.WIGridView:Update()
	self:UpdateLines()

	local origin = self:GetWorldOrigin()
	local halfExtents = self:GetWorldBounds()
	local startPos = {origin.y -halfExtents.y,origin.x -halfExtents.x}
	local gridSize = self:GetClampedGridSize()
	local numLines = get_line_count(self:GetSize(),self:GetUnitSize(),gridSize)
	local unitSize = self:GetUnitSize()
	for _,v in ipairs(LINES) do -- Horizontal and vertical
		local offset = (gridSize -math.abs(startPos[v] %gridSize)) *unitSize
		if(v == LINES_VERTICAL and INVERT_AXES[AXIS_X] == true) then offset = self:GetWidth() -offset
		elseif(v == LINES_HORIZONTAL and INVERT_AXES[AXIS_Y] == true) then offset = self:GetHeight() -offset end
		local c = offset
		for i=1,numLines[v] do
			local pLine = self.m_tLines[v][i]
			if(util.is_valid(pLine)) then
				local scale = (i -1) /(numLines[v] -1)
				local cpos = self:ToWorldPosition(Vector2(c,c))
				cpos = {cpos.y,cpos.x}
				local col = self:GetLineColor(cpos[v])
				if(v == LINES_HORIZONTAL) then pLine:SetY(c)
				else pLine:SetX(c) end
				pLine:SetVisible(true)
				pLine:SetColor(col)
			end
			local gridScale = 1.0
			if((v == LINES_VERTICAL and INVERT_AXES[AXIS_X] == true) or (v == LINES_HORIZONTAL and INVERT_AXES[AXIS_Y] == true)) then gridScale = -1.0 end
			c = c +(unitSize *gridSize) *gridScale
		end
		-- Hide all lines which are out of bounds/unused
		for i=math.max(numLines[v] +1,1),#self.m_tLines[v] do
			local pLine = self.m_tLines[v][i]
			if(util.is_valid(pLine)) then
				pLine:SetVisible(false)
			end
		end
	end

	local pLine1 = self.m_tOriginLines[1]
	local pLine2 = self.m_tOriginLines[2]
	pLine1:SetPos(self:GetSize() *0.5 -Vector2(pLine1:GetWidth() *0.5,1))
	pLine2:SetPos(self:GetSize() *0.5 -Vector2(1,pLine2:GetHeight() *0.5))
	self:CallCallbacks("Update")
end
gui.register("WIGridView",gui.WIGridView)
