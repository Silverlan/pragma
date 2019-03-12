util.register_class("gui.WIModelView",gui.Base)

local function get_nearz() return 1.0 end
local function get_farz() return 16000.0 end

function gui.WIModelView:__init()
	gui.Base.__init(self)
	self.m_bRenderFilterEnabled = true
	self:SetClearColor(Color.Clear)
	self.m_matError = game.load_material("error")
end
function gui.WIModelView:SetClearColor(col) self.m_clearColor = col end
local function draw_meshes(self,ent,meshes,shader,filter)
	local mdl = self:GetModel()
	for _,t in ipairs(meshes) do
		local mat = t[1]
		local meshGroupId = t[2]
		local meshGroup = t[3]
		local mesh = t[4]
		local subMesh = t[5]
		if(self.m_bRenderFilterEnabled == false or self.m_renderFilter(ent,mdl,meshGroupId,meshGroup,mesh,subMesh,mat) == true) then
			if(filter == nil or filter(ent,mdl,meshGroupId,meshGroup,mesh,subMesh,mat) == true) then
				if(shader.RecordBindMaterial ~= nil) then
          if(shader:RecordBindMaterial(mat) == false) then return false end
        end
				if(shader.BindVertexAnimationOffset ~= nil) then
					local vertAnimComponent = ent:GetComponent(ents.COMPONENT_VERTEX_ANIMATED)
					local vaBuffer = (vertAnimComponent ~= nil) and vertAnimComponent:GetVertexAnimationBuffer() or nil
					local bBindVertexAnimBuffer = false
					if(vaBuffer ~= nil) then
						local offset,animCount = vertAnimComponent:GetVertexAnimationBufferMeshOffset(subMesh)
						if(offset ~= nil) then
							local vaData = bit.bor(bit.rshift(bit.lshift(offset,16),16),bit.lshift(animCount,16))
							shader:BindVertexAnimationOffset(vaData)
							bBindVertexAnimBuffer = true
						end
					end
					if(bBindVertexAnimBuffer == false) then shader:BindVertexAnimationOffset(0) end
				end
				if(shader:RecordDrawMesh(subMesh) == false) then return false end
			end
		end
	end
  return true
end
function gui.WIModelView:GetScene() return self.m_scene end
function gui.WIModelView:GetRenderTarget() return self:GetScene():GetRenderTarget() end
function gui.WIModelView:SetRenderFilterEnabled(b) self.m_bRenderFilterEnabled = b end
function gui.WIModelView:SetRenderFilter(filter)
	if(filter == nil) then
		filter = function(ent,mdl,meshGroupId,meshGroup,mesh,subMesh,mat)
			return self.m_meshGroupIds[meshGroupId] == true
		end
	end
	self.m_renderFilter = filter
end
function gui.WIModelView:RenderEntity(shader,filter,drawCmd,cbBindShaderData)
	shader = shader or self.m_shaderShaded
	if(util.is_valid(self.m_entity) == false or util.is_valid(shader) == false) then return end
	local mdlComponent = self.m_entity:GetComponent(ents.COMPONENT_MODEL)
	local mdl = (mdlComponent ~= nil) and mdlComponent:GetModel() or nil
	if(util.is_valid(mdl) == false) then return end

	drawCmd = drawCmd or game.get_draw_command_buffer()
	local scene = self.m_scene
	if(shader:RecordBeginDraw(drawCmd) == true) then
		if(shader.OnDraw ~= nil) then shader:OnDraw(drawCmd) end
    if(shader:RecordBindScene(scene,false) == true and shader:RecordBindEntity(self.m_entity) == true and (cbBindShaderData == nil or cbBindShaderData(shader,drawCmd) == true)) then
      draw_meshes(self,self.m_entity,self.m_meshes,shader,filter)
      draw_meshes(self,self.m_entity,self.m_meshesTranslucent,shader,filter)
      shader:RecordEndDraw()
    end
	end
end
function gui.WIModelView:SetShowMeshes(b) self.m_bShowMeshes = b end
function gui.WIModelView:ClearLightSources()
	for _,light in ipairs(self.m_lightSources) do
		if(light:IsValid() == true) then light:Remove() end
	end
	self.m_lightSources = {}
end
function gui.WIModelView:AddLightSource(light)
	table.insert(self.m_lightSources,light)
	self:UpdateSceneLights()
end
function gui.WIModelView:SetLightingEnabled(b)
	if(b == self:IsLightingEnabled()) then return end
	self.m_bLightingEnabled = b
	self:UpdateSceneLights()
end
function gui.WIModelView:IsLightingEnabled() return self.m_bLightingEnabled end
function gui.WIModelView:UpdateSceneLights()
	local bEnabled = self:IsLightingEnabled()
	for _,light in ipairs(self.m_lightSources) do
		local pToggleComponent = light:GetEntity():GetComponent(ents.COMPONENT_TOGGLE)
		if(pToggleComponent ~= nil) then
			if(bEnabled == true) then pToggleComponent:TurnOn()
			else pToggleComponent:TurnOff() end
		end
	end
	self.m_scene:SetLightSources(self.m_lightSources)
end
function gui.WIModelView:OnInitialize()
	gui.Base.OnInitialize(self)
	self.m_shaderShaded = shader.get("mde_textured")

	self.m_lightSources = {}
	self.m_bLightingEnabled = false
	self.m_bShowMeshes = true
	self.m_lookAtPos = Vector()
	local ent = ents.create("mde_model")
	ent:AddComponent(ents.COMPONENT_FLEX)
	ent:AddComponent(ents.COMPONENT_VERTEX_ANIMATED)
	ent:Spawn()
	self.m_entity = ent
	self.m_lastCycle = 0.0

	local gameScene = game.get_scene()
	local gameCam = gameScene:GetCamera()
	self.m_scene = game.create_scene(
		gameScene:GetWidth(),gameScene:GetHeight(),
		gameCam:GetFOV(),gameCam:GetViewFOV(),
		gameCam:GetNearZ(),gameCam:GetFarZ(),
		vulkan.SAMPLE_COUNT_1_BIT
	)
	self.m_scene:GetRenderTarget():SetDebugName("WIModelView")
	self.m_scene:InitializeRenderTarget()
	self.m_scene:SetWorldEnvironment(gameScene:GetWorldEnvironment())
	self.m_scene:SetPrepassMode(gameScene:GetPrepassMode())
	self.m_scene:AddEntity(ent)

	self.m_cam = self.m_scene:GetCamera()
	self.m_cam:SetUp(Vector(0,1,0))
  
  self.m_drawSceneInfo = game.DrawSceneInfo()
  self.m_drawSceneInfo.clearColor = util.Color.Black
  self.m_drawSceneInfo.renderFlags = game.RENDER_FLAG_NONE
  self.m_drawSceneInfo.scene = self.m_scene

	local pBg = gui.create("WITexturedRect",self)
	pBg:SetAutoAlignToParent(true)
	self.m_pBg = pBg
	local bRender = false
	self.m_cbPrepass = game.add_callback("RenderPrepass",function()
		if(bRender == false) then return end
		local shader = self.m_scene:GetPrepassShader()
		shader:RecordBindEntity(self.m_entity)
		draw_meshes(self,self.m_entity,self.m_meshes,shader)
	end)
	self.m_cbRender = game.add_callback("Render",function()
		if(bRender == false) then return end
		if(self.m_bShowMeshes == true) then self:RenderEntity() end
		self:CallCallbacks("OnSceneRender",game.get_draw_command_buffer())
	end)
	self.m_cbPreRender = game.add_callback("PreRender",function()
		if(self:IsValid() == false or util.is_valid(self.m_entity) == false) then return end
		local animComponent = self.m_entity:GetComponent(ents.COMPONENT_ANIMATED)
		if(animComponent == nil) then return end
		local nCycle = animComponent:GetCycle()
		if(self.m_bRenderScheduled ~= true and nCycle == self.m_lastCycle) then return end
		self.m_lastCycle = nCycle
		self.m_bRenderScheduled = false
		
		local drawCmd = game.get_draw_command_buffer()
		if(self.m_updateCamera == true) then
			self.m_updateCamera = false
      self.m_scene:UpdateBuffers(drawCmd)
		end
    
    local rt = self.m_scene:GetRenderTarget()
    local img = rt:GetTexture():GetImage()
    drawCmd:RecordImageBarrier(
      img,vulkan.PIPELINE_STAGE_FRAGMENT_SHADER_BIT,vulkan.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      vulkan.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,vulkan.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      vulkan.ACCESS_SHADER_READ_BIT,vulkan.ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    )
		
		bRender = true
			local renderComponent = self.m_entity:GetComponent(ents.COMPONENT_RENDER)
			if(renderComponent ~= nil) then renderComponent:UpdateRenderBuffers(drawCmd,true) end
      self.m_drawSceneInfo.commandBuffer = drawCmd
			game.draw_scene(self.m_drawSceneInfo,rt)
		bRender = false
    
    drawCmd:RecordImageBarrier(
      img,vulkan.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,vulkan.PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      vulkan.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,vulkan.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      vulkan.ACCESS_COLOR_ATTACHMENT_WRITE_BIT,vulkan.ACCESS_SHADER_READ_BIT
    )
	end)
	self:ResetCamera()
	self:SetCameraMovementEnabled(true)
end
function gui.WIModelView:ResetCamera()
	self:SetCameraPosition(Vector(-30.0,70.0,40.0))
	if(util.is_valid(self.m_entity)) then
		local mdlComponent = self.m_entity:GetComponent(ents.COMPONENT_MODEL)
		local mdl = (mdlComponent ~= nil) and mdlComponent:GetModel() or nil
		if(mdl ~= nil) then
			local min,max = mdl:GetRenderBounds()
			local center = (min +max) /2.0
			self:SetLookAtPosition(center)
			return
		end
	end
	self:SetLookAtPosition(Vector())
end
function gui.WIModelView:OnRemove()
	gui.Base.OnRemove(self)
	if(util.is_valid(self.m_cbPrepass) == true) then self.m_cbPrepass:Remove() end
	if(util.is_valid(self.m_cbRender) == true) then self.m_cbRender:Remove() end
	if(util.is_valid(self.m_cbPreRender) == true) then self.m_cbPreRender:Remove() end
end
function gui.WIModelView:GetCamera() return self.m_cam end
function gui.WIModelView:GetModel()
	if(util.is_valid(self.m_entity) == false) then return end
	local mdlComponent = self.m_entity:GetComponent(ents.COMPONENT_MODEL)
	if(mdlComponent == nil) then return end
	return mdlComponent:GetModel()
end
function gui.WIModelView:UpdateModel()
	self.m_bRenderScheduled = true
	self.m_meshes = {}
	self.m_meshesTranslucent = {}
	self.m_meshGroupIds = {}
	local mdlComponent = util.is_valid(self.m_entity) and self.m_entity:GetComponent(ents.COMPONENT_MODEL) or nil
	if(mdlComponent == nil) then return end
	local mdl = mdlComponent:GetModel()
	for _,id in ipairs(mdl:GetBaseMeshGroupIds()) do self.m_meshGroupIds[id] = true end
	for meshGroupId,meshGroup in ipairs(mdl:GetMeshGroups()) do
		local mats = mdl:GetMaterials()
		for _,mesh in ipairs(meshGroup:GetMeshes()) do
			for _,subMesh in ipairs(mesh:GetSubMeshes()) do
				local matId = subMesh:GetMaterialIndex()
				local mat = mats[matId +1] or self.m_matError
				if(mat ~= nil) then
					if(mat:IsTranslucent() == true) then table.insert(self.m_meshesTranslucent,{mat,meshGroupId -1,meshGroup,mesh,subMesh})
					else table.insert(self.m_meshes,{mat,meshGroupId -1,meshGroup,mesh,subMesh}) end
				end
			end
		end
	end
end
function gui.WIModelView:SetModel(mdl)
	self.m_meshes = {}
	self.m_meshesTranslucent = {}
	if(util.is_valid(self.m_entity) == false) then return end
	local bValid = util.is_valid(mdl)
	local pMdlComponent = self.m_entity:GetComponent(ents.COMPONENT_MODEL)
	if(pMdlComponent ~= nil) then
		if(bValid == false) then pMdlComponent:SetModel()
		else pMdlComponent:SetModel(mdl) end
	end
	self.m_bRenderScheduled = true
	if(bValid == false) then return end
	local min,max = mdl:GetRenderBounds()
	local center = (min +max) /2.0
	self:SetLookAtPosition(center)
	self:UpdateModel()
end
function gui.WIModelView:MouseCallback(button,action,mods)
	gui.Base.MouseCallback(self,button,action,mods)
	if(self.m_bCameraMovementEnabled == false) then return end
	if(button == input.MOUSE_BUTTON_LEFT) then
		if(action == input.STATE_PRESS) then
			self.m_bRotate = true
			self.m_tLastCursorPos = self:GetCursorPos()
		else self.m_bRotate = false end
	elseif(button == input.MOUSE_BUTTON_RIGHT) then
		if(action == input.STATE_PRESS) then
			self.m_bMove = true
			self.m_tLastCursorPos = self:GetCursorPos()
		else self.m_bMove = false end
	end
end
function gui.WIModelView:OnThink()
	gui.Base.OnThink(self)
	if(self.m_bRotate ~= true and self.m_bMove ~= true) then return end
	local cursorPos = self:GetCursorPos()
	local offset = cursorPos -self.m_tLastCursorPos
	if(self.m_bRotate == true) then
		local posCam = self:GetCameraPosition()
		local posOrigin = self:GetLookAtPosition()
		local ang = (posOrigin -posCam):ToEulerAngles()
		ang.p = math.clamp(ang.p +offset.y,-89.8,89.8)
		ang.y = ang.y -offset.x
		posCam = posOrigin -ang:GetForward() *posOrigin:Distance(posCam)
		self:SetCameraPosition(posCam)
	end
	if(self.m_bMove == true) then
		local forward = self.m_cam:GetForward()
		local right = self.m_cam:GetRight()
		forward = forward:ProjectToPlane(Vector(0,1,0),0.0)
		right = right:ProjectToPlane(Vector(0,1,0),0.0)
		local moveOffset = right *offset.x
		if(input.get_key_state(input.KEY_LEFT_SHIFT) == input.STATE_RELEASE and input.get_key_state(input.KEY_RIGHT_SHIFT) == input.STATE_RELEASE) then
			moveOffset = moveOffset +forward *-offset.y
		else
			moveOffset = moveOffset +Vector(0,1,0) *-offset.y
		end
		self:SetCameraPosition(self:GetCameraPosition() +moveOffset)
		self:SetLookAtPosition(self:GetLookAtPosition() +moveOffset)
	end
	self.m_tLastCursorPos = cursorPos
end
function gui.WIModelView:ScrollCallback(xoffset,yoffset)
	gui.Base.ScrollCallback(self,xoffset,yoffset)
	if(self.m_bCameraMovementEnabled == false) then return end
	local posCam = self:GetCameraPosition()
	local posLookAt = self:GetLookAtPosition()
	local dir = posLookAt -posCam
	local l = dir:Length()
	if(l == 0.0) then return end
	yoffset = yoffset *20.0
	dir = dir /l
	posCam = posCam +dir *math.min(math.max(l -1.0,1.0),math.abs(yoffset)) *math.sign(yoffset)
	self:SetCameraPosition(posCam)
end
function gui.WIModelView:SetCameraMovementEnabled(b)
	self.m_bCameraMovementEnabled = b
	self:SetMouseInputEnabled(b)
	self:SetScrollInputEnabled(b)
end
function gui.WIModelView:SetCameraPosition(pos)
	self.m_cam:SetPos(pos)
	self.m_cam:LookAt(self:GetLookAtPosition())
	self.m_cam:UpdateViewMatrix()
	self.m_updateCamera = true
	self.m_bRenderScheduled = true
end
function gui.WIModelView:SetLookAtPosition(pos)
	self.m_lookAtPos = pos
	self.m_cam:LookAt(pos)
	self.m_cam:UpdateViewMatrix()
	self.m_updateCamera = true
	self.m_bRenderScheduled = true
end
function gui.WIModelView:GetCameraPosition() return self.m_cam:GetPos() end
function gui.WIModelView:GetLookAtPosition() return self.m_lookAtPos end
function gui.WIModelView:Update()
	if(util.is_valid(self.m_pBg) == false) then return end
	local size = self:GetSize()
	if(size.x == 0 or size.y == 0) then
		self.m_cam:SetAspectRatio(1.0)
		self.m_cam:UpdateProjectionMatrix()
		return
	end
	self.m_cam:SetAspectRatio(size.x /size.y)
	self.m_cam:UpdateProjectionMatrix()

	self.m_pBg:SetTexture(self.m_scene:GetRenderTarget():GetTexture())

	self:UpdateModel()
	self:Render()
end
function gui.WIModelView:Render()
	self.m_bRenderScheduled = true
	self.m_updateCamera = true
end
function gui.WIModelView:OnSizeChanged(w,h)
	gui.Base.OnSizeChanged(self,w,h)
	self:Update()
end
function gui.WIModelView:GetEntity() return self.m_entity end
function gui.WIModelView:PlayAnimation(anim)
	if(util.is_valid(self.m_entity) == false) then return end
	local animComponent = self.m_entity:GetComponent(ents.COMPONENT_ANIMATED)
	if(animComponent == nil) then return end
	animComponent:PlayAnimation(anim)
end
function gui.WIModelView:OnRemove()
	gui.Base.OnRemove(self)
	if(util.is_valid(self.m_entity) == false) then return end
	self.m_entity:RemoveSafely()
end
gui.register("WIModelView",gui.WIModelView)
