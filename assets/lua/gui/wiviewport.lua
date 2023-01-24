include("/gui/wigridview.lua")
include("/gui/wiorthoview.lua")

util.register_class("gui.WIViewport",gui.Base)

gui.WIViewport.VIEWPORT_TYPE_TOP = 0
gui.WIViewport.VIEWPORT_TYPE_FRONT = 1
gui.WIViewport.VIEWPORT_TYPE_SIDE = 2
gui.WIViewport.VIEWPORT_TYPE_3D = 3

gui.WIViewport.viewports = {}
gui.WIViewport.get_viewports = function() return gui.WIViewport.viewports end
function gui.WIViewport:__init()
	gui.Base.__init(self)
end
function gui.WIViewport:OnInitialize()
	gui.Base.OnInitialize(self)
	gui.WIViewport.viewports[self] = true

	self:EnableThinking()
	self.m_type = -1
	self.m_zoomFactor = 1.0
	self.m_gridSize = 1
	self.m_posWorld = Vector(0,0,0)
	self:SetMouseInputEnabled(true)
	self:SetScrollInputEnabled(true)
	self:SetKeyboardInputEnabled(true)

	local pBg = gui.create("WIRect",self)
	pBg:SetColor(Color.Black)
	pBg:SetAutoAlignToParent(true)

	local pGrid = gui.create("WIGridView",self)
	pGrid:SetAutoAlignToParent(true)

	local pOrtho = gui.create("WIOrthoView",self)
	pOrtho:SetAutoAlignToParent(true)
	local pTex = pOrtho:GetTextureElement()
	if(util.is_valid(pTex) == true) then
		pTex.__lveInvertedX = false
		pTex.__lveInvertedY = false
	end
	self.m_pOrtho = pOrtho

	pGrid:AddCallback("Update",function(el)
		if(util.is_valid(self) == false) then return end
		self:CallCallbacks("Update")
	end)
	self.m_pGrid = pGrid

	local pTex = gui.create("WITexturedRect",self)
	pTex:SetAutoAlignToParent(true)
	pTex:SetVisible(false)
	self.m_pTexture = pTex

	-- Use the default game scene by default
	self:SetScene(game.get_scene())
	self:SetType(gui.WIViewport.VIEWPORT_TYPE_3D)
	self:SetMovementControlsEnabled(true)
end
--[[function gui.WIViewport:SetupScene(w,h)
	local cam = game.get_primary_camera()
	if(cam == nil) then return end
	self.m_scene = {}
	self.m_scene.scene = ents.create_scene(w,h,cam:GetFOV())
	self.m_scene.renderTarget = self.m_scene.scene:GetRenderTarget()
	pTex:SetTexture(rt:GetTexture())
end]]
function gui.WIViewport:SetMovementControlsEnabled(enabled)
	self.m_movementControlsEnabled = enabled
end
function gui.WIViewport:AreMovementControlsEnabled() return self.m_movementControlsEnabled end
function gui.WIViewport:SetScene(scene,renderer,shouldRender)
	renderer = renderer or scene:GetRenderer()
	local rasterizer = renderer and renderer:GetEntity():GetComponent(ents.COMPONENT_RASTERIZATION_RENDERER) or nil
	self.m_scene = {
		scene = scene,
		camera = scene:GetActiveCamera(),
		renderTarget = (rasterizer ~= nil) and rasterizer:GetRenderTarget() or nil
	}
	self.m_pTexture:SetTexture(renderer:GetPresentationTexture())

	util.remove(self.m_cbRenderScenes)
	if(self:IsPrimaryGameSceneViewport() or shouldRender == false) then return end
	local incMask,excMask = game.get_primary_camera_render_mask()
	self.m_cbRenderScenes = game.add_callback("RenderScenes",function(drawSceneInfo)
		if(shouldRender and shouldRender() == false) then return end
		local drawSceneInfo = game.DrawSceneInfo()
		drawSceneInfo.toneMapping = shader.TONE_MAPPING_NONE
		drawSceneInfo.scene = scene
		drawSceneInfo.renderFlags = bit.band(drawSceneInfo.renderFlags,bit.bnot(game.RENDER_FLAG_BIT_VIEW)) -- Don't render view models
		drawSceneInfo.inclusionMask = incMask
		drawSceneInfo.exclusionMask = excMask
		game.queue_scene_for_rendering(drawSceneInfo)
	end)
end
function gui.WIViewport:SetType(type)
	if(type == self.m_type) then return end
	self.m_type = type
	local b3DPort = (self.m_type == gui.WIViewport.VIEWPORT_TYPE_3D) and true or false
	if(util.is_valid(self.m_pGrid)) then self.m_pGrid:SetVisible(not b3DPort) end
	if(util.is_valid(self.m_pOrtho)) then
		self.m_pOrtho:SetVisible(not b3DPort)

		local pTex = self.m_pOrtho:GetTextureElement()
		if(type == gui.WIViewport.VIEWPORT_TYPE_FRONT) then
			self.m_pOrtho:SetAxis("x")
			if(util.is_valid(pTex) == true) then
				pTex:InvertVertexUVCoordinates(not pTex.__lveInvertedX,pTex.__lveInvertedY)
				pTex.__lveInvertedX = not pTex.__lveInvertedX
				pTex.__lveInvertedY = false
			end
		elseif(type == gui.WIViewport.VIEWPORT_TYPE_SIDE) then
			self.m_pOrtho:SetAxis("z")
			if(util.is_valid(pTex) == true) then
				pTex:InvertVertexUVCoordinates(pTex.__lveInvertedX,pTex.__lveInvertedY)
				pTex.__lveInvertedX = false
				pTex.__lveInvertedY = false
			end
		else
			self.m_pOrtho:SetAxis("y")
			if(util.is_valid(pTex) == true) then
				pTex:InvertVertexUVCoordinates(not pTex.__lveInvertedX,pTex.__lveInvertedY)
				pTex.__lveInvertedX = not pTex.__lveInvertedX
				pTex.__lveInvertedY = false
			end
		end
	end
	if(util.is_valid(self.m_pTexture)) then self.m_pTexture:SetVisible(b3DPort) end
end
function gui.WIViewport:DrawScene()
	if(self.m_scene == nil) then return end
	local scene = self.m_scene.scene
	local rt = self.m_scene.renderTarget
	--game.draw_scene(scene,rt:GetFramebuffer(),bit.band(game.FRENDER_ALL,bit.bnot(game.FRENDER_VIEW)),Color.Black)
end
function gui.WIViewport:GetType() return self.m_type end
function gui.WIViewport:SetObjectManager(mngr) self.m_objectManager = mngr end
function gui.WIViewport:GetObjectManager() return self.m_objectManager end
function gui.WIViewport:GetScene() return (self.m_scene ~= nil) and self.m_scene.scene or nil end
function gui.WIViewport:GetSceneCamera() return (self.m_scene ~= nil) and self.m_scene.camera or nil end
function gui.WIViewport:IsPrimaryGameSceneViewport() return (self.m_scene == nil or util.is_same_object(self.m_scene.scene,game.get_render_scene())) end
function gui.WIViewport:GetCamera()
	local scene = self:GetScene()
	if(scene == nil) then return nil end
	return scene:GetActiveCamera()
end
function gui.WIViewport:GetRenderTarget()
	if(self.m_scene == nil) then return nil end
	return self.m_scene.renderTarget
end
function gui.WIViewport:Is3DViewport() return (self.m_type == gui.WIViewport.VIEWPORT_TYPE_3D) and true or false end
function gui.WIViewport:ScrollCallback(x,y)
	if(self:AreMovementControlsEnabled() == false) then return util.EVENT_REPLY_UNHANDLED end
	if(self:Is3DViewport() == false) then
		y = y *-0.5
		local zoom = self:GetZoomFactor()
		self:SetZoomFactor(math.clamp(zoom +y,1.0,40.0))
		self:Update()
	else
		local cam = self:GetCamera()
		local ent = cam:GetEntity()
		local pos = ent:GetPos()
		local forward = ent:GetForward()
		pos = pos +forward *(y *20.0)
		ent:SetPos(pos)
		cam:UpdateViewMatrix()
	end
	return util.EVENT_REPLY_HANDLED
end
function gui.WIViewport:KeyboardCallback(key,scanCode,action,mods)
	--[[if(action == input.STATE_PRESS) then
		if(key == input.KEY_KP_ADD or key == input.KEY_KP_SUBTRACT) then
			local gridSize = self:GetGridSize()
			if(key == input.KEY_KP_ADD) then gridSize = gridSize *2
			else gridSize = gridSize /2 end
			gridSize = math.clamp(gridSize,1,2 ^9)
			self:SetGridSize(gridSize)
			self:Update()
		end
	end]]
	return util.EVENT_REPLY_UNHANDLED
end
local keys = {
	[input.KEY_LEFT] = false,
	[input.KEY_RIGHT] = false,
	[input.KEY_UP] = false,
	[input.KEY_DOWN] = false
}
local moveKeys = {
	[input.KEY_W] = false,
	[input.KEY_A] = false,
	[input.KEY_S] = false,
	[input.KEY_D] = false
}
function gui.WIViewport:OnThink()
	-- Check if scene presentation texture has changed (e.g. if scene has been resized).
	-- TODO: Probably better to handle this via callbacks?
	local curTex = self.m_pTexture:GetTexture()
	local sceneTex = self.m_scene.scene:GetRenderer():GetPresentationTexture()
	if(curTex ~= sceneTex) then self.m_pTexture:SetTexture(sceneTex) end

	if(self:HasFocus() == false or self:AreMovementControlsEnabled() == false) then return end
	local bAnyKey = false
	for key,v in pairs(keys) do
		keys[key] = (input.get_key_state(key) == input.STATE_PRESS) and true or false
		if(keys[key] == true) then bAnyKey = true end
	end
	if(self:Is3DViewport() == false) then
		if(bAnyKey == true) then
			local origin = self:GetWorldOrigin()
			local offset = time.frame_time() *20.0 *self:GetZoomFactor()
			if(keys[input.KEY_LEFT] == true) then origin.x = origin.x -offset end
			if(keys[input.KEY_RIGHT] == true) then origin.x = origin.x +offset end
			if(keys[input.KEY_UP] == true) then origin.y = origin.y +offset end
			if(keys[input.KEY_DOWN] == true) then origin.y = origin.y -offset end
			self:SetWorldOrigin(origin)
			self:Update()
		end
	elseif(self.m_scene ~= nil) then
		for key,v in pairs(moveKeys) do
			keys[key] = (input.get_key_state(key) == input.STATE_PRESS) and true or false
			if(keys[key] == true) then bAnyKey = true end
		end
		if(bAnyKey == true) then
			local cam = self:GetCamera()
			local ent = cam:GetEntity()
			local forward = ent:GetForward()
			local up = ent:GetUp()
			local right = forward:Cross(up)
			local pos = ent:GetPos()
			local ang = Quaternion(forward,right,up):ToEulerAngles()

			local tFrame = time.frame_time()
			local rotOffset = tFrame *250.0
			if(keys[input.KEY_LEFT] == true) then ang.y = ang.y +rotOffset end
			if(keys[input.KEY_RIGHT] == true) then ang.y = ang.y -rotOffset end
			if(keys[input.KEY_UP] == true) then ang.p = ang.p -rotOffset end
			if(keys[input.KEY_DOWN] == true) then ang.p = ang.p +rotOffset end
			ang.p = math.clamp(ang.p,-90.0,90.0)

			local moveOffset = tFrame *250.0
			if(keys[input.KEY_W] == true) then pos = pos +forward *moveOffset end
			if(keys[input.KEY_A] == true) then pos = pos -right *moveOffset end
			if(keys[input.KEY_S] == true) then pos = pos -forward *moveOffset end
			if(keys[input.KEY_D] == true) then pos = pos +right *moveOffset end

			ent:SetPos(pos)
			cam:SetForward(ang:GetForward())
			cam:SetUp(ang:GetUp())
			cam:UpdateViewMatrix()
		end
	end
end
-- Returns a world position and direction for a 2d position on a 3d viewport
function gui.WIViewport:GetWorldDirection(uv)
	uv.x = uv.x /self:GetWidth()
	uv.y = uv.y /self:GetHeight()
	local cam = self:GetCamera()
	local np = cam:GetNearPlanePoint(uv)
	local fp = cam:GetFarPlanePoint(uv)
	local dir = fp -np
	dir:Normalize()
	return np,dir
end
function gui.WIViewport:OnCursorEntered()
	self:RequestFocus()
end
function gui.WIViewport:OnCursorExited()
	self:KillFocus()
end
function gui.WIViewport:MouseCallback(button,action,mods)
	if(self:AreMovementControlsEnabled() == false) then return util.EVENT_REPLY_UNHANDLED end
	if(action == input.STATE_PRESS and (button == input.MOUSE_BUTTON_LEFT or button == input.MOUSE_BUTTON_RIGHT)) then
		self:RequestFocus()
		return util.EVENT_REPLY_HANDLED
	end
	return util.EVENT_REPLY_UNHANDLED
end
function gui.WIViewport:ToLocalPosition(v)
	if(util.is_valid(self.m_pGrid) == false) then return Vector(0,0,0) end
	local r = Vector2(0,0)
	for localAxis,axis in pairs(self:GetAxes()) do
		r[localAxis] = v[axis]
	end
	return self.m_pGrid:ToLocalPosition(r)
end
function gui.WIViewport:ToWorldPosition(v)
	if(util.is_valid(self.m_pGrid) == false) then return Vector(0,0,0) end
	if(self:GetType() == gui.WIViewport.VIEWPORT_TYPE_3D) then
		return self:GetWorldDirection(v)
	end
	local v = self.m_pGrid:ToWorldPosition(v)
	local r = Vector(0,0,0)
	for localAxis,axis in pairs(self:GetAxes()) do
		r[axis] = v[localAxis]
	end
	return r
end
function gui.WIViewport:GetAxes()
	local type = self:GetType()
	if(type == gui.WIViewport.VIEWPORT_TYPE_TOP) then return {x = "x",y = "z"}
	elseif(type == gui.WIViewport.VIEWPORT_TYPE_FRONT) then return {x = "z",y = "y"}
	elseif(type == gui.WIViewport.VIEWPORT_TYPE_SIDE) then return {x = "x",y = "y"} end
	return {x = "x",y = "y",z = "z"}
end
function gui.WIViewport:GetUnusedAxis()
	local type = self:GetType()
	if(type == gui.WIViewport.VIEWPORT_TYPE_TOP) then return "y"
	elseif(type == gui.WIViewport.VIEWPORT_TYPE_FRONT) then return "x"
	elseif(type == gui.WIViewport.VIEWPORT_TYPE_SIDE) then return "z" end
end
function gui.WIViewport:GetGrid() return self.m_pGrid end
function gui.WIViewport:GetNearZ()
	local cam = self:GetCamera()
	if(cam == nil) then return 0.0 end
	return cam:GetNearZ()
end
function gui.WIViewport:GetFarZ()
	local cam = self:GetCamera()
	if(cam == nil) then return 0.0 end
	return cam:GetFarZ()
end
function gui.WIViewport:OnRemove()
	util.remove(self.m_cbRenderScenes)
	gui.WIViewport.viewports[self] = nil
end
function gui.WIViewport:SetZoomFactor(factor)
	self.m_zoomFactor = factor
	if(self.m_pGrid ~= nil and util.is_valid(self.m_pGrid)) then self.m_pGrid:SetZoomFactor(factor) end
end
function gui.WIViewport:GetZoomFactor() return self.m_zoomFactor end
function gui.WIViewport:SetGridSize(size)
	self.m_gridSize = size
	if(self.m_pGrid ~= nil and util.is_valid(self.m_pGrid)) then self.m_pGrid:SetGridSize(size) end
end
function gui.WIViewport:GetGridSize() return self.m_gridSize end
function gui.WIViewport:SetWorldOrigin(pos)
	self.m_posWorld = pos:Copy()
	if(self.m_pGrid ~= nil and util.is_valid(self.m_pGrid)) then self.m_pGrid:SetWorldOrigin(pos) end
end
function gui.WIViewport:GetWorldOrigin() return self.m_posWorld end
function gui.WIViewport:SetRenderObjects(tObjsWireframe,tObjsShaded)
	if(util.is_valid(self.m_pOrtho) == false) then return end
	self.m_pOrtho:SetRenderObjects(tObjsWireframe,tObjsShaded)
	self.m_pOrtho:Render()
end
function gui.WIViewport:RenderGeometry()
	if(util.is_valid(self.m_pOrtho) == false) then return end
	self.m_pOrtho:Render()
end
function gui.WIViewport:Update()
	if(self.m_pGrid ~= nil and util.is_valid(self.m_pGrid)) then self.m_pGrid:Update() end
	if(util.is_valid(self.m_pOrtho) == false) then return end
	local sz = self:GetSize()
	local startPos = self:ToWorldPosition(Vector2(0,0))
	local endPos = self:ToWorldPosition(Vector2(sz.x,sz.y))
	
	self.m_pOrtho:SetRegion(startPos,endPos)
	self:RenderGeometry()
end
gui.register("WIViewport",gui.WIViewport)
