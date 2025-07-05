-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.register_class("gui.WIOrthoView", gui.Base)

local RENDER_TARGET_SIZE = Vector2i(1024, 1024)

local function initialize_camera_buffer(self)
	local shaderGeometry = shader.get("lve_geometry")
	if shaderGeometry == nil then
		return false
	end
	self.m_shaderGeometry = shaderGeometry
	self.m_shaderShaded = shader.get("lve_textured")
	local descBuffer =
		shaderGeometry:GenerateSwapDescriptorBuffer(2, prosper.BUFFER_USAGE_UNIFORM_BUFFER_BIT, util.SIZEOF_MAT4 * 3)
	if descBuffer ~= nil then
		self.m_camDescBuffer = descBuffer
		return true
	end
	return false
end
local function get_nearz()
	return -8000.0
end
local function get_farz()
	return 16000.0
end

function gui.WIOrthoView:__init()
	gui.Base.__init(self)
end
function gui.WIOrthoView:OnRemove()
	gui.Base.OnRemove(self)
	if util.is_valid(self.m_cbRender) == true then
		self.m_cbRender:Remove()
	end
end
function gui.WIOrthoView:SetAxis(axis)
	if axis ~= "x" and axis ~= "y" and axis ~= "z" then
		return
	end
	self.m_pivotAxis = axis
	self.m_axes = {}
	if axis == "x" then
		table.insert(self.m_axes, "z")
		table.insert(self.m_axes, "y")
		self.m_upAxis = "y"
	elseif axis == "y" then
		table.insert(self.m_axes, "x")
		table.insert(self.m_axes, "z")
		self.m_upAxis = "z"
	else
		table.insert(self.m_axes, "x")
		table.insert(self.m_axes, "y")
		self.m_upAxis = "y"
	end
end
function gui.WIOrthoView:SetRegion(startPos, endPos)
	vector.to_min_max(startPos, endPos)
	self.m_startPos = startPos
	self.m_endPos = endPos
	self:UpdateCameraBuffer()
end
function gui.WIOrthoView:UpdateCameraBuffer()
	self.m_camDescBuffer:Swap()
	local camBuf = self.m_camDescBuffer:GetBuffer(0)

	local axes = self.m_axes
	local startPos = self.m_startPos
	local endPos = self.m_endPos
	local origin = (startPos + endPos) / 2.0
	startPos = startPos - origin
	endPos = endPos - origin
	local up = Vector(0, 0, 0)
	up[self.m_upAxis] = 1.0

	local offset = Vector()
	offset[self.m_pivotAxis] = -200.0 -- Distance doesn't matter, but shouldn't be too low to avoid precision errors

	local v = matrix.create_look_at_matrix(origin + offset, origin, up)
	local p = matrix.create_orthogonal_matrix(
		-startPos[axes[1]],
		-endPos[axes[1]],
		-endPos[axes[2]],
		-startPos[axes[2]],
		get_farz(),
		get_nearz()
	)
	local vp = p * v

	local dataCamera = DataStream(util.SIZEOF_MAT4 * 3)
	dataCamera:WriteMat4(v)
	dataCamera:WriteMat4(p)
	dataCamera:WriteMat4(vp)
	camBuf:MapMemory(dataCamera, true)
end
function gui.WIOrthoView:GetBounds()
	local startPos = self.m_startPos:Copy()
	local endPos = self.m_endPos:Copy()
	startPos[self.m_pivotAxis] = get_nearz()
	endPos[self.m_pivotAxis] = get_farz()
	return startPos, endPos
end
local function render_objects(self)
	if self.m_tRender == nil then
		return
	end
	local drawCmd = game.get_draw_command_buffer()

	local shaders = {}
	table.insert(shaders, self.m_shaderGeometry)
	if self.m_shaderShaded ~= nil then
		table.insert(shaders, self.m_shaderShaded)
	end

	local startPos, endPos = self:GetBounds()

	local rt = self.m_renderTarget
	local tex = rt:GetTexture()
	local img = tex:GetImage()
	local w = rt:GetWidth()
	local h = rt:GetHeight()
	drawCmd:BeginRenderPass(rt, Color(0, 0, 0, 0), 1.0)
	drawCmd:SetImageLayout(img, prosper.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	for i = #shaders, 1, -1 do
		local shader = shaders[i]
		if shader:BeginDraw() == true then
			if i == 1 then
				drawCmd:SetLineWidth(1.0)
			end
			drawCmd:SetViewport(w, h)
			drawCmd:SetScissor(w, h)
			if i == 1 then
				shader:BindColor(Color.White)
			end
			shader:BindCameraDescriptorSet(self.m_camDescBuffer:GetDescriptorSet())

			for _, data in ipairs(self.m_tRender[i]) do
				local min, max = data.entity:GetRenderBounds()
				local pos = data.entity:GetPos()
				min = min + pos
				max = max + pos
				if intersect.aabb_with_aabb(startPos, endPos, min, max) ~= intersect.RESULT_OUTSIDE then
					data.entity:UpdateRenderBuffers(true)
					shader:BindEntity(data.entity)
					for _, meshData in ipairs(data.meshes) do
						local min, max = meshData.mesh:GetBounds()
						min = min + pos
						max = max + pos
						if intersect.aabb_with_aabb(startPos, endPos, min, max) ~= intersect.RESULT_OUTSIDE then
							shader:BindMaterial(meshData.material)
							shader:Draw(meshData.mesh)
						end
					end
				end
			end
			shader:EndDraw()
		end
	end
	drawCmd:EndRenderPass()
	drawCmd:SetImageLayout(img, prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	img:GenerateMipmaps()
end
function gui.WIOrthoView:Render()
	self.m_bRenderScheduled = true
end
function gui.WIOrthoView:SetRenderObjects(tObjsWireframe, tObjsShaded)
	self.m_tRender = { tObjsWireframe or {}, tObjsShaded or {} }
end
function gui.WIOrthoView:GetTextureElement()
	return self.m_pBg
end
function gui.WIOrthoView:OnInitialize()
	gui.Base.OnInitialize(self)
	if initialize_camera_buffer(self) == false then
		return
	end

	local pBg = gui.create("WITexturedRect", self)
	pBg:SetAutoAlignToParent(true)
	self.m_pBg = pBg

	local err, rt = prosper.create_render_target(
		RENDER_TARGET_SIZE.x,
		RENDER_TARGET_SIZE.y,
		prosper.FORMAT_R8G8B8A8_UNORM,
		bit.bor(prosper.IMAGE_USAGE_COLOR_ATTACHMENT_BIT, prosper.IMAGE_USAGE_SAMPLED_BIT),
		prosper.FORMAT_D32_SFLOAT,
		true,
		false,
		true
	)
	self.m_renderTarget = rt

	pBg:SetTexture(rt:GetTexture())
	self:SetAxis("z")

	self.m_cbRender = game.add_callback("PreRender", function()
		if self:IsValid() == false or self.m_bRenderScheduled ~= true then
			return
		end
		self.m_bRenderScheduled = false
		render_objects(self, true)
	end)
	self:SetRegion(Vector(), Vector())
end
gui.register("WIOrthoView", gui.WIOrthoView)
