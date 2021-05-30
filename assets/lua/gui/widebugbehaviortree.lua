local RESULT_INITIAL = 4294967295
local RESULT_PENDING = 0
local RESULT_FAILED = 1
local RESULT_SUCCEEDED = 2

local NODE_BACKGROUND_COLOR_INITIAL = util.Color.DarkGray
local NODE_BACKGROUND_COLOR_SUCCESS = util.Color.GreenYellow
local NODE_BACKGROUND_COLOR_FAILURE = util.Color.FireBrick
local NODE_BACKGROUND_COLOR_PENDING = util.Color.Aquamarine

local NODE_WIDTH = 180
local NODE_HEIGHT = 32
local NODE_HORIZONTAL_SPACING = 5
local NODE_HORIZONTAL_SPACING_GROUP = 15
local NODE_VERTICAL_SPACING = 50

local TASK_FADE_DURATION = 0.5

util.register_class("gui.WIDebugBehaviorTree",gui.Base)
function gui.WIDebugBehaviorTree:__init()
	gui.Base.__init(self)
end
function gui.WIDebugBehaviorTree:OnInitialize()
	self.m_nodes = {}
	self.m_rootNodes = {}
	
	local pHighlight = gui.create("WIRoundedRect",self)
	pHighlight:SetColor(Color.Red)
	pHighlight:SetVisible(false)
	pHighlight:SetCornerSize(0.08)
	pHighlight:SetRoundness(2)
	pHighlight:Update()
	self.m_pHighlight = pHighlight

	self:EnableThinking()
end
function gui.WIDebugBehaviorTree:OnThink()
  self:FocusStep()
end
function gui.WIDebugBehaviorTree:FocusStep()
  local tFocus = self.m_focusInfo
  if(tFocus == nil) then return end
  local tDelta = time.real_time() -tFocus.tStart
  local sc = (tFocus.duration > 0.0) and math.min(math.smooth_step(0.0,1.0,tDelta /tFocus.duration),1.0) or 1.0
  local pos = tFocus.src +(tFocus.dst -tFocus.src) *sc
  self:SetPos(pos)
  if(sc >= 1.0) then self.m_focusInfo = nil end
end
function gui.WIDebugBehaviorTree:GetRootNodes() return self.m_rootNodes end
function gui.WIDebugBehaviorTree:GetNodes() return self.m_nodes end
function gui.WIDebugBehaviorTree:GetNode(i) return self.m_nodes[i] end
function gui.WIDebugBehaviorTree:AddNode(node,parent)
	local w = NODE_WIDTH
	local h = NODE_HEIGHT
	local x = 0
	local pConnector
	if(parent ~= nil) then x = parent:GetRight() +NODE_VERTICAL_SPACING end
  local p = gui.create("WIDebugBehaviorTreeNode",self)
  p:SetX(x)
  p:SetSize(w,h)
  --p:SizeToContents()
  p:SetNode(node)
  table.insert(self.m_nodes,p)
	
	if(parent ~= nil) then
		pConnector = gui.create("WILine",self)
		pConnector:SetColor(Color.LightGrey)
		pConnector:SetLineWidth(1)
		pConnector:SizeToContents()
		pConnector:SetSize(Vector2(20000,20000)) -- TODO
		
		pConnector:GetStartPosProperty():Link(parent:GetAnchorPosProperty("child_connector"))
		pConnector:GetEndPosProperty():Link(p:GetAnchorPosProperty("parent_connector"))
	end
	
  if(parent ~= nil) then
		p:RemoveElementOnRemoval(pConnector)
    p:SetParentNode(parent,pConnector)
    parent:AddChildNode(p,pConnector)
  else table.insert(self.m_rootNodes,p) end
  return p
end
function gui.WIDebugBehaviorTree:ClearNodes()
	for _,pNode in ipairs(self:GetNodes()) do
		if(pNode:IsValid()) then
			pNode:Remove()
		end
	end
	self.m_nodes = {}
	self.m_rootNodes = {}
	if(util.is_valid(self.m_pHighlight)) then self.m_pHighlight:SetVisible(false) end
end
function gui.WIDebugBehaviorTree:BuildTree(tree)
	local update = #self:GetNodes() > 0
	local hNode = NODE_HEIGHT
	local nodePending
	local nodePendingChildIndex
	local nodePendingDepth = -1
	
	local nodeLast
	local nodeLastExecutionIndex = -1
	
	-- Populate tree
	local childIdx = 1
	local function iterate_tree(children,parent,depth)
		depth = depth or 0
		for i,child in ipairs(children) do
			local p
			if(update == false) then
				p = self:AddNode(child,parent)
			else
				p = self:GetNode(childIdx)
				if(util.is_valid(p)) then p:UpdateNode(child) end
			end
			
			if(nodePending == nil or (depth > nodePendingDepth and p:IsActive())) then
				nodePendingDepth = depth
				nodePending = p
				nodePendingChildIndex = childIdx
			end
			if(child.executionIndex > nodeLastExecutionIndex) then
				nodeLast = p
				nodeLastExecutionIndex = child.executionIndex
			end
			childIdx = childIdx +1
			
			iterate_tree(child.children,p,depth +1)
		end
	end
	iterate_tree({tree})
	
	if(nodePendingChildIndex == 1) then
		-- Pending node is root node, which probably the means the last task has just finished executing.
		-- Since the root node is uninteresting, and we'd rather know what the last actual task was, we change
		-- it to the task with the highest execution index (i.e. the task that was executed last).
		nodePending = nodeLast
	end
	
	if(update == false) then
		local nodeLevels = {}
		local function populate_node_levels(nodes,i)
			i = i or 1
			nodeLevels[i] = nodeLevels[i] or {}
			for _,n in ipairs(nodes) do
				table.insert(nodeLevels[i],n)
				populate_node_levels(n:GetChildNodes(),i +1)
			end
		end
		populate_node_levels(self:GetRootNodes())
		
		local function update_node_positions(i,yStart)
			i = i or 1
			yStart = yStart or 0
			local p
			for _,n in ipairs(nodeLevels[i]) do
				local np = n:GetParentNode()
				if(p ~= nil and np ~= p) then yStart = yStart +NODE_HORIZONTAL_SPACING_GROUP end
				n:SetY(yStart -n:GetHeight() *0.5)
				yStart = yStart +n:GetHeight() +NODE_HORIZONTAL_SPACING
				p = np
			end
			
			local yCenter = 0
			for childIdx,n in ipairs(nodeLevels[i]) do
				if(childIdx == 1) then yCenter = yCenter -n:GetY() end
				yCenter = yCenter +n:GetBottom()
			end
			yCenter = yCenter /(#nodeLevels[i] +1)
			
			local hChildren = 0
			if(#nodeLevels > i) then
				p = nil
				for childIdx,n in ipairs(nodeLevels[i +1]) do
					hChildren = hChildren +n:GetHeight()
					if(childIdx > 1) then hChildren = hChildren +NODE_HORIZONTAL_SPACING end
					
					local np = n:GetParentNode()
					if(p ~= nil and np ~= p) then hChildren = hChildren +NODE_HORIZONTAL_SPACING_GROUP end
					p = np
				end
				local yStart = yCenter -hChildren *0.5
				update_node_positions(i +1,yStart)
			end
		end
		if(#nodeLevels > 0) then
			update_node_positions()
		end
		
		-- Update positions
		local yMin = 0
		local nodes = self:GetNodes()
		for _,p in ipairs(nodes) do
			if(p:IsValid()) then
				yMin = math.min(yMin,p:GetY())
			end
		end
		
		for _,p in ipairs(nodes) do
			if(p:IsValid()) then
				p:SetY(p:GetY() -yMin)
			end
		end
		self:SizeToContents()
	end
	
	if(util.is_valid(nodePending)) then
		self:FocusOnNode(nodePending,0.8,0.3,TASK_FADE_DURATION)
		self.m_pHighlight:SetVisible(true)
		
		local pos = nodePending:GetPos()
		local size = nodePending:GetSize()
		self.m_pHighlight:SetPos(pos.x -2,pos.y -2)
		self.m_pHighlight:SetSize(size.x +4,size.y +4)
	elseif(util.is_valid(self.m_pHighlight)) then self.m_pHighlight:SetVisible(false) end
end
function gui.WIDebugBehaviorTree:FocusOnNode(node,xPos,yPos,duration)
	if(node == self.m_focusedNode) then return end
  yPos = yPos or 0.5
	
  self.m_focusedNode = node
  local parent = self:GetParent()
  local xFocus = node:GetX() +node:GetWidth() *0.5
  local yFocus = node:GetY() +node:GetHeight() *0.5
  local posSrc = self:GetPos()
  local posTgt = Vector2(math.min(parent:GetWidth() *xPos -xFocus,100),parent:GetHeight() *yPos -yFocus)
  self.m_focusInfo = {
    src = posSrc,
    dst = posTgt,
    tStart = time.real_time(),
    duration = duration or 0.0
  }
  
  self:FocusStep()
end
gui.register("WIDebugBehaviorTree",gui.WIDebugBehaviorTree)

local texGradient = prosper.create_gradient_texture(128,64,prosper.FORMAT_R8G8B8A8_UNORM,Vector2(0,-1),{
	{offset = 0.0,color = Color.White},
	{offset = 1.0,color = Color(160,160,160,255)}
})

util.register_class("gui.WIDebugBehaviorTreeNode",gui.Base)
function gui.WIDebugBehaviorTreeNode:__init()
	gui.Base.__init(self)
end
function gui.WIDebugBehaviorTreeNode:OnInitialize()
	local pMain = gui.create("WIRoundedTexturedRect",self)
	pMain:SetTexture(texGradient)
	pMain:SetCornerSize(0.08)
	pMain:SetRoundness(2)
	pMain:Update()
  pMain:SetAutoAlignToParent(true)
  pMain:GetColorProperty():Link(self:GetColorProperty())
  
  local pText = gui.create("WIText",self)
  pText:SetColor(Color.Honeydew)
	pText:EnableShadow(true)
	pText:SetShadowOffset(math.Vector2(1,1))
	pText:SetShadowColor(Color.Black)
	--pText:SetAutoBreakMode(gui.Text.AUTO_BREAK_WHITESPACE) -- TODO
  self.m_pText = pText
  
  self.m_childNodes = {}
  self.m_childConnectors = {}
	self.m_bActive = false
	self.m_tLastEnd = 0.0
	self.m_name = ""
	self:SetResult(RESULT_INITIAL)
	
	self:AddAnchor("parent_connector",Vector2(0,0.5))
	self:AddAnchor("child_connector",Vector2(1.0,0.5))
end
function gui.WIDebugBehaviorTreeNode:OnThink()
	local sc = 1.0
	if(self:IsActive() == false) then
		local tDelta = time.cur_time() -self.m_tLastEnd
		sc = 1.0 -math.min(tDelta /2.0,0.4)
	end
	local col = self.m_bgColor:Copy()
	col.r = col.r *sc
	col.g = col.g *sc
	col.b = col.b *sc
	self:SetColor(col)
end
function gui.WIDebugBehaviorTreeNode:SetBackgroundColor(col)
	self.m_bgColor = col
end
function gui.WIDebugBehaviorTreeNode:GetName() return self.m_name end
function gui.WIDebugBehaviorTreeNode:IsActive() return self.m_bActive end
function gui.WIDebugBehaviorTreeNode:GetResult() return self.m_result end
function gui.WIDebugBehaviorTreeNode:SetResult(result)
	local col = NODE_BACKGROUND_COLOR_INITIAL
	if(result == RESULT_SUCCEEDED) then col = NODE_BACKGROUND_COLOR_SUCCESS
	elseif(result == RESULT_FAILED) then col = NODE_BACKGROUND_COLOR_FAILURE
	elseif(result == RESULT_PENDING) then col = NODE_BACKGROUND_COLOR_PENDING end
	self:SetBackgroundColor(col)
	--util.fade_color_property(self:GetColorProperty(),col,TASK_FADE_DURATION)
	self.m_result = result
	
  --local parentConnector = self:GetParentConnector()
  --if(util.is_valid(parentConnector)) then util.fade_color_property(parentConnector:GetColorProperty(),Color.Lime,TASK_FADE_DURATION) end
end
function gui.WIDebugBehaviorTreeNode:SetNode(node)
	self.m_name = node.name
	self.m_pText:SetFont("default")
  self.m_pText:SetText(node.name)
  self.m_pText:SizeToContents()
	if(self.m_pText:GetWidth() > self:GetWidth()) then
		self.m_pText:SetFont("default_small")
		self.m_pText:SizeToContents()
		if(self.m_pText:GetWidth() > self:GetWidth()) then
			self.m_pText:SetFont("default_tiny")
			self.m_pText:SizeToContents()
		end
	end
  self.m_pText:SetPos(self:GetWidth() *0.5 -self.m_pText:GetWidth() *0.5,self:GetHeight() *0.5 -self.m_pText:GetHeight() *0.5)
	
	self:UpdateNode(node)
end
function gui.WIDebugBehaviorTreeNode:UpdateNode(node)
	self.m_bActive = node.active
	self.m_tLastEnd = node.lastEndTime
	self:SetResult(node.state)
end
function gui.WIDebugBehaviorTreeNode:SetParentNode(parent,connector)
  self.m_parentNode = parent
  self.m_parentConnector = connector
end
function gui.WIDebugBehaviorTreeNode:GetParentNode() return self.m_parentNode end
function gui.WIDebugBehaviorTreeNode:GetParentConnector() return self.m_parentConnector end
function gui.WIDebugBehaviorTreeNode:AddChildNode(node,connector)
  table.insert(self.m_childNodes,node)
  table.insert(self.m_childConnectors,connector)
end
function gui.WIDebugBehaviorTreeNode:SetChildNodes(childNodes) self.m_childNodes = childNodes end
function gui.WIDebugBehaviorTreeNode:SetChildConnectors(connectors) self.m_childConnectors = connectors end
function gui.WIDebugBehaviorTreeNode:GetChildNodes() return self.m_childNodes end
function gui.WIDebugBehaviorTreeNode:GetChildConnectors() return self.m_childConnectors end
gui.register("WIDebugBehaviorTreeNode",gui.WIDebugBehaviorTreeNode)
