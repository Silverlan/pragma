--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("/shaders/srgb_to_linear.lua")
include_component("click")

util.register_class("ents.GUI3D", BaseEntityComponent)

local ENABLE_MIPMAPS = false

function ents.GUI3D:__init()
	BaseEntityComponent.__init(self)
end
function ents.GUI3D:Initialize()
	BaseEntityComponent.Initialize(self)
	self:AddEntityComponent(ents.COMPONENT_RENDER)
	self:AddEntityComponent(ents.COMPONENT_MODEL)
	self:AddEntityComponent(ents.COMPONENT_TRANSFORM, "InitializeTransform")

	self.m_keyMods = 0
	self:SetAutoCursorUpdateEnabled(true)
	self:SetUnlit(true)

	self:SetAlphaMode(game.Material.ALPHA_MODE_MASK)
	self:SetAlphaCutoff(0.5)
	self:SetAlwaysRender(true)
	self.m_renderScheduled = true

	self:SetTickPolicy(ents.TICK_POLICY_ALWAYS)
	self.m_drawToTexInfo = gui.DrawToTextureInfo()

	self:SetMsaaEnabled(false)
	self:SetRefreshRate(24)
	self.m_lastFrameRendered = 0.0
end
function ents.GUI3D:SetRefreshRate(refreshRate)
	self.m_refreshRate = refreshRate
	self.m_tRefreshRate = 1.0 / refreshRate
	self.m_lastFrameRendered = 0
end
function ents.GUI3D:GetRefreshRate()
	return self.m_refreshRate
end
function ents.GUI3D:SetMsaaEnabled(enabled)
	self.m_drawToTexInfo.enableMsaa = enabled
end
function ents.GUI3D:IsMsaaEnabled()
	return self.m_drawToTexInfo.enableMsaa
end
function ents.GUI3D:InitializeTransform(transformC)
	transformC:SetScale(Vector(50, 50, 50)) -- Default scale
end
function ents.GUI3D:Setup(guiElement, mesh)
	self:SetGUIElement(guiElement)
	self:SetInterfaceMesh(mesh)
end
function ents.GUI3D:ScheduleRender()
	self.m_renderScheduled = true
end
function ents.GUI3D:SetGUIElement(el)
	self.m_pGui = el
end
function ents.GUI3D:GetGUIElement()
	return self.m_pGui
end
function ents.GUI3D:SetInterfaceMesh(mesh)
	self.m_interfaceMesh = mesh
end
function ents.GUI3D:CreateSquareMesh()
	local subMesh = game.Model.Mesh.Sub.create()
	local origin = Vector(0, 0, 0)
	local scale = 1.0
	local p0 = origin + Vector(-1, 1, 0) * scale
	local p1 = origin + Vector(1, 1, 0) * scale
	local p2 = origin + Vector(1, -1, 0) * scale
	local p3 = origin + Vector(-1, -1, 0) * scale

	local v0 = game.Model.Vertex(p0, Vector2(0, 0), Vector(0, 0, 1))
	local v1 = game.Model.Vertex(p1, Vector2(1, 0), Vector(0, 0, 1))
	local v2 = game.Model.Vertex(p2, Vector2(1, 1), Vector(0, 0, 1))
	local v3 = game.Model.Vertex(p3, Vector2(0, 1), Vector(0, 0, 1))
	local idx0 = subMesh:AddVertex(v0)
	local idx1 = subMesh:AddVertex(v1)
	local idx2 = subMesh:AddVertex(v2)
	local idx3 = subMesh:AddVertex(v3)
	subMesh:AddTriangle(idx0, idx3, idx1)
	subMesh:AddTriangle(idx1, idx3, idx2)
	return subMesh
end
function ents.GUI3D:InitializeModel(subMesh)
	-- TODO: Cache this model/mesh, then overwrite material with SetMaterialOverride
	subMesh = subMesh or self:CreateSquareMesh()
	local mdl = game.create_model()
	local meshGroup = mdl:GetMeshGroup(0)
	local mesh = game.Model.Mesh.Create()
	mesh:AddSubMesh(subMesh)
	meshGroup:AddMesh(mesh)

	mdl:Update(game.Model.FUPDATE_ALL)

	self:SetInterfaceMesh(subMesh)
	return mdl
end
function ents.GUI3D:GetLocalRayData(localize)
	local origin, dir = ents.ClickComponent.get_ray_data()
	if localize then
		local pose = self:GetPose():GetInverse()
		dir:Rotate(pose:GetRotation())
		origin = (pose * math.Transform(origin, Quaternion())):GetOrigin()
	end
	return origin, dir
	--[[local trComponent = self:GetEntity():GetComponent(ents.COMPONENT_TRANSFORM)
	if(trComponent == nil) then return end
	local scene = game.get_scene()
	local cam = scene:GetActiveCamera()
	local trCam = cam and cam:GetEntity():GetComponent(ents.COMPONENT_TRANSFORM) or nil
	if(trCam == nil) then return end
	local origin = trCam:GetPos()
	if(localize == true) then origin = trComponent:WorldToLocal(origin) end

	local w = scene:GetWidth()
	local h = scene:GetHeight()
	local uv = input.get_cursor_pos()
	uv.x = uv.x /w
	uv.y = uv.y /h
	local dir = util.calc_world_direction_from_2d_coordinates(cam,uv)
	if(localize == true) then dir:Rotate(trComponent:GetRotation():GetInverse()) end
	return origin,dir]]
end
function ents.GUI3D:CalcCursorPos(origin, dir)
	local trComponent = self:GetEntity():GetComponent(ents.COMPONENT_TRANSFORM)
	if trComponent == nil then
		return
	end
	if origin == nil then
		origin, dir = self:GetLocalRayData(false)
		if origin == nil then
			return
		end
	end
	origin = trComponent:WorldToLocal(origin)
	dir:Rotate(trComponent:GetRotation():GetInverse())
	local p = self.m_pGui
	if origin == nil or util.is_valid(p) == false then
		return
	end
	--local dot = dir:DotProduct(self:GetForward())
	--print(dot) -- TODO

	local verts = self.m_interfaceMesh:GetVertices()
	local uvs = self.m_interfaceMesh:GetUVs()
	local triangles = self.m_interfaceMesh:GetIndices()

	local ent = self:GetEntity()
	local trComponent = ent:GetComponent(ents.COMPONENT_TRANSFORM)
	local scale = (trComponent ~= nil) and trComponent:GetAbsMaxAxisScale() or Vector(1, 1, 1)
	for i = 1, #triangles, 3 do
		local v0 = verts[triangles[i] + 1] * scale
		local v1 = verts[triangles[i + 1] + 1] * scale
		local v2 = verts[triangles[i + 2] + 1] * scale
		local fraction, bc0, bc1 = intersect.line_with_triangle(origin, dir, v0, v1, v2)
		--self:OnEnteredUseRange() -- TODO
		if fraction ~= false and fraction > 0.0 then
			--self:OnEnteredUseRange()
			local uv0 = uvs[triangles[i] + 1]
			local uv1 = uvs[triangles[i + 1] + 1]
			local uv2 = uvs[triangles[i + 2] + 1]
			local bc = geometry.calc_barycentric_coordinates(v0, uv0, v1, uv1, v2, uv2, origin + dir * fraction)
			bc.x = bc.x % 1.0
			bc.y = bc.y % 1.0
			local x = bc.x * p:GetWidth()
			local y = bc.y * p:GetHeight()
			--print(x,y)
			--pCursor:SetPos(x,y)
			return Vector2(x, y)
		end --else self:OnExitedUseRange() end
	end
end
function ents.GUI3D:SetCursor(texture, w, h)
	if util.is_valid(self.m_pGui) == false then
		return
	end
	w = w or 8
	h = h or 8
	util.remove(self.m_cursor)
	if type(texture) == "string" then
		local el = gui.create("WITexturedRect", self.m_pGui)
		el:SetMaterial(texture)
		el:SetSize(w, h)
		self:SetCursor(el)
		return
	end
	local el = texture
	el:SetParent(self.m_pGui)
	el:SetZPos(10000)
	self.m_cursor = el
end
function ents.GUI3D:SetCursorPos(origin, dir)
	local p = self.m_pGui
	if util.is_valid(p) == false then
		return false
	end
	local pos = self:CalcCursorPos(origin, dir)
	self.m_cursorPos = pos
	if pos ~= nil then
		p:InjectMouseMoveInput(pos)
	end
	if util.is_valid(self.m_cursor) then
		self.m_cursor:SetVisible(pos ~= nil)
		if pos ~= nil then
			self.m_cursor:SetPos(pos.x - self.m_cursor:GetWidth() / 2, pos.y - self.m_cursor:GetHeight() / 2)
		end
	end
	return pos ~= nil
end
function ents.GUI3D:GetCursorPos()
	if util.is_valid(self.m_cursor) == false then
		return Vector2i()
	end
	return self.m_cursor:GetPos() + self.m_cursor:GetSize() / 2.0
end
function ents.GUI3D:SetUnlit(unlit)
	self.m_unlit = unlit
end
function ents.GUI3D:IsUnlit()
	return self.m_unlit
end
function ents.GUI3D:SetAutoCursorUpdateEnabled(enabled)
	self.m_autoCursorUpdateEnabled = enabled
end
function ents.GUI3D:IsAutoCursorUpdateEnabled()
	return self.m_autoCursorUpdateEnabled
end
function ents.GUI3D:OnTick()
	self:UpdateCursorPos()
end
function ents.GUI3D:UpdateCursorPos()
	if self.m_interfaceMesh == nil or self.m_autoCursorUpdateEnabled ~= true then
		return
	end
	--local pCursor = self.m_pCursor
	--if(util.is_valid(pCursor) == false) then return end
	--local lp = ents.get_local_player()
	--[[if(lp:GetDistance(self) <= 200.0) then -- TODO
		self:OnEnteredUseRange()
	else self:OnExitedUseRange() end]]
	self:SetCursorPos(self:GetLocalRayData(false))
end
function ents.GUI3D:InitializeGUICallbacks()
	local pl = ents.get_local_player()
	if util.is_valid(pl) then
		self.m_cbActionInput = pl:AddEventCallback(
			ents.PlayerComponent.EVENT_HANDLE_ACTION_INPUT,
			function(action, bPressed, magnitude)
				if util.is_valid(self.m_pGui) == false then
					return util.EVENT_REPLY_UNHANDLED
				end
				local bt
				if action == input.ACTION_ATTACK then
					bt = input.MOUSE_BUTTON_LEFT
				elseif action == input.ACTION_ATTACK2 then
					bt = input.MOUSE_BUTTON_RIGHT
				end
				if bt == nil then
					return util.EVENT_REPLY_UNHANDLED
				end
				local state = (bPressed == true) and input.STATE_PRESS or input.STATE_RELEASE

				if state == input.STATE_RELEASE and self.m_keyDown ~= nil then
					local key = self.m_keyDown
					self.m_keyDown = nil
					if key > input.KEY_BACKSPACE then
						local elFocus = gui.get_focused_element()
						self.m_pGui:InjectKeyboardInput(key, state, self.m_keyMods)
						-- We don't want the element focus to change to any of the 3D elements, so we'll restore the focus back
						if util.is_valid(elFocus) and gui.get_focused_element() ~= elFocus then
							elFocus:RequestFocus()
						end
						return util.EVENT_REPLY_HANDLED
					end
					return util.EVENT_REPLY_UNHANDLED
				end
				local pos = self:CalcCursorPos()
				if pos ~= nil then
					local elFocus = gui.get_focused_element()
					local res = self.m_pGui:InjectMouseInput(pos, bt, state)
					if res == util.EVENT_REPLY_UNHANDLED then
						self:BroadcastEvent(ents.GUI3D.EVENT_ON_UNHANDLED_MOUSE_INPUT, { pos, bt, state })
					end
					-- debug.print("InjectMouseInput ",self.m_pGui,pos,bt,state)
					-- We don't want the element focus to change to any of the 3D elements, so we'll restore the focus back
					if util.is_valid(elFocus) and gui.get_focused_element() ~= elFocus then
						elFocus:RequestFocus()
					end
				end
				return util.EVENT_REPLY_HANDLED
			end
		)
	end
	self.m_cbScrollInput = game.add_callback("OnScrollInput", function(x, y)
		if util.is_valid(self.m_pGui) == false then
			return
		end
		local pos = self:CalcCursorPos()
		if pos ~= nil then
			self.m_pGui:InjectScrollInput(pos, Vector2(x, y) * 10.0)
		end
		return false
	end)
end
function ents.GUI3D:InitializeGUIDrawCallback()
	self.m_cbDrawGUI = game.add_callback("PostGUIDraw", function()
		self:DrawGUIElement()
	end)
end
function ents.GUI3D:SetClearColor(clearColor)
	self.m_drawToTexInfo.clearColor = clearColor
end
function ents.GUI3D:SetAlwaysRender(alwaysRender)
	self.m_alwaysRender = alwaysRender
end
function ents.GUI3D:GetRenderTarget()
	return self.m_renderTarget
end
function ents.GUI3D:DrawGUIElement()
	if self.m_alwaysRender then
		self.m_renderScheduled = true
	end
	if self.m_renderScheduled ~= true then
		local cursorPos = self.m_cursorPos
		if cursorPos == self.m_lastRenderCursorPos then
			return
		end
		self.m_lastRenderCursorPos = self.m_cursorPos and self.m_cursorPos:Copy() or nil
	end

	if self.m_renderScheduled and self.m_refreshRate ~= -1 then
		local tSinceLastRender = time.real_time() - self.m_lastFrameRendered
		if tSinceLastRender < self.m_tRefreshRate then
			return
		end
		self.m_lastFrameRendered = time.real_time()
	end

	local p = self.m_pGui
	local rt = self.m_renderTarget
	if util.is_valid(p) == false or rt == nil then
		return
	end
	self.m_renderScheduled = nil
	local drawInfo = self.m_drawToTexInfo
	local drawCmd = game.get_draw_command_buffer()
	drawInfo.commandBuffer = drawCmd
	if self.m_drawToTexInfo.enableMsaa then
		local img = self.m_resolvedTexture:GetImage()
		drawCmd:RecordImageBarrier(
			img,
			prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			prosper.IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		)
	end
	p:DrawToTexture(rt, drawInfo)
	-- Image now in shader read-only optimal layout

	-- The interface will be rendered on a 3D object, which means it will be subject to gamma-correction.
	-- We don't actually want that, so we'll 'invert' the gamma correction here before it is applied later
	-- to neutralize it.
	local rtDst = self.m_renderTargetDst
	local imgDst = rtDst:GetTexture():GetImage()
	drawCmd:RecordImageBarrier(
		imgDst,
		prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		prosper.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	)
	drawCmd:RecordBeginRenderPass(prosper.RenderPassInfo(self.m_renderTargetDst))
	local srgbToLinearShader = shader.get("srgb_to_linear")
	srgbToLinearShader:GetWrapper():Draw(drawCmd, self.m_dsTex)
	drawCmd:RecordEndRenderPass()
	drawCmd:RecordImageBarrier(
		imgDst,
		prosper.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	)

	if ENABLE_MIPMAPS then
		drawCmd:RecordImageBarrier(
			imgDst,
			prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			prosper.IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		)
		drawCmd:RecordGenerateMipmaps(
			imgDst,
			prosper.IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			prosper.ACCESS_TRANSFER_WRITE_BIT,
			prosper.PIPELINE_STAGE_TRANSFER_BIT
		)
	end
end
function ents.GUI3D:InitializeRenderTarget(w, h)
	local msaa = self.m_drawToTexInfo.enableMsaa
	local sampling = true
	local rt = gui.create_render_target(w, h, msaa, sampling)
	self.m_renderTarget = rt

	local colTex = rt:GetColorAttachmentTexture()
	local colImg = colTex:GetImage()
	local dsTex = colTex
	if msaa then
		local usageFlags = bit.bor(
			prosper.IMAGE_USAGE_SAMPLED_BIT,
			prosper.IMAGE_USAGE_TRANSFER_DST_BIT,
			prosper.IMAGE_USAGE_TRANSFER_SRC_BIT
		)
		local imgResolved =
			gui.create_color_image(w, h, usageFlags, prosper.IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, false --[[msaa]])
		self.m_resolvedTexture = prosper.create_texture(
			imgResolved,
			prosper.TextureCreateInfo(),
			prosper.ImageViewCreateInfo(),
			prosper.SamplerCreateInfo()
		)
		dsTex = self.m_resolvedTexture
		self.m_drawToTexInfo.resolvedImage = imgResolved
	end

	local imgCreateInfo = colImg:GetCreateInfo()
	imgCreateInfo.samples = prosper.SAMPLE_COUNT_1_BIT
	if ENABLE_MIPMAPS then
		imgCreateInfo.flags = prosper.ImageCreateInfo.FLAG_FULL_MIPMAP_CHAIN_BIT
	end
	local imgViewCreateInfo = prosper.ImageViewCreateInfo()
	imgViewCreateInfo.format = imgCreateInfo.format
	imgViewCreateInfo.levelCount = 1
	local imgDst = prosper.create_image(imgCreateInfo)
	local texDst =
		prosper.create_texture(imgDst, prosper.TextureCreateInfo(), imgViewCreateInfo, prosper.SamplerCreateInfo())
	local rpCreateInfo = prosper.RenderPassCreateInfo()
	rpCreateInfo.format = colImg:GetFormat()
	rpCreateInfo.samples = prosper.SAMPLE_COUNT_1_BIT
	rpCreateInfo.loadOp = prosper.ATTACHMENT_LOAD_OP_DONT_CARE
	rpCreateInfo.storeOp = prosper.ATTACHMENT_STORE_OP_STORE
	rpCreateInfo.initialLayout = prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	rpCreateInfo.finalLayout = prosper.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	local colAttId = rpCreateInfo:AddAttachment(
		imgCreateInfo.format,
		prosper.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		prosper.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		prosper.ATTACHMENT_LOAD_OP_DONT_CARE,
		prosper.ATTACHMENT_STORE_OP_STORE
	)
	local subPassId = rpCreateInfo:AddSubPass()
	rpCreateInfo:AddSubPassColorAttachment(subPassId, colAttId)

	local rp = prosper.create_render_pass(rpCreateInfo)
	self.m_renderTargetDst = prosper.create_render_target(prosper.RenderTargetCreateInfo(), texDst, rp)

	self.m_shaderSrgbToLinear = shader.get("srgb_to_linear")
	local ds = self.m_shaderSrgbToLinear:CreateDescriptorSet(0)
	ds:SetBindingTexture(0, dsTex)
	self.m_dsTex = ds

	return self.m_renderTargetDst
end
function ents.GUI3D:SetAlphaMode(alphaMode)
	self.m_alphaMode = alphaMode
end
function ents.GUI3D:SetAlphaCutoff(cutoff)
	self.m_alphaCutoff = cutoff
end
function ents.GUI3D:SetShader(shaderName)
	self.m_shaderName = shaderName
end
function ents.GUI3D:OnEntitySpawn()
	self:InitializeGUICallbacks()
	if self.m_pGui == nil then
		return
	end
	local mdl = self:InitializeModel(self.m_interfaceMesh)
	local mdlComponent = self:GetEntity():GetComponent(ents.COMPONENT_MODEL)
	if mdl == nil or mdlComponent == nil then
		return
	end

	if self.m_pGui:IsUpdateScheduled() then
		self.m_pGui:Update()
	end
	self:InitializeGUIDrawCallback()
	self:InitializeRenderTarget(self.m_pGui:GetWidth(), self.m_pGui:GetHeight())
	if self.m_renderTargetDst == nil then
		return
	end
	local mat = game.create_material(self.m_shaderName or (self:IsUnlit() and "unlit" or "pbr"))
	mat:GetData():SetValue("int", "alpha_mode", tostring(self.m_alphaMode))
	mat:GetData():SetValue("float", "alpha_cutoff", tostring(self.m_alphaCutoff))
	if self:IsUnlit() == false then
		mat:GetData():SetValue("float", "metalness_factor", "0.0")
		mat:GetData():SetValue("float", "roughness_factor", "1.0")
	end
	mat:SetTexture("albedo_map", self.m_renderTargetDst:GetTexture())

	local matIdx = mdl:AddMaterial(0, mat)
	self.m_interfaceMesh:SetSkinTextureIndex(matIdx)

	mdlComponent:SetModel(mdl)
end
function ents.GUI3D:OnRemove()
	util.remove(self.m_pGui)
	util.remove(self.m_cbActionInput)
	util.remove(self.m_cbScrollInput)
	util.remove(self.m_cbDrawGUI)
end
ents.COMPONENT_GUI3D = ents.register_component("gui_3d", ents.GUI3D)
ents.GUI3D.EVENT_ON_UNHANDLED_MOUSE_INPUT = ents.register_component_event(ents.COMPONENT_GUI3D, "unhandled_mouse_input")
