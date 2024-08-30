--[[
	Copyright (C) 2021 Silverlan

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

util.register_class("gui.UvAtlasMeshOverlay", gui.Base)

function gui.UvAtlasMeshOverlay:__init()
	gui.Base.__init(self)
	self:SetRenderResolution(512, 512)
end
function gui.UvAtlasMeshOverlay:SetLightmapCache(lmCache)
	self.m_lightmapCache = lmCache
end
function gui.UvAtlasMeshOverlay:SetEntity(ent)
	self.m_entity = ent
end
function gui.UvAtlasMeshOverlay:SetRenderResolution(w, h)
	self.m_width = w
	self.m_height = h
end
function gui.UvAtlasMeshOverlay:OnInitialize()
	gui.Base.OnInitialize(self)

	self:SetSize(512, 512)

	self.m_elTex = gui.create("WITexturedRect", self, 0, 0, self:GetWidth(), self:GetHeight(), 0, 0, 1, 1)
end
function gui.UvAtlasMeshOverlay:SetPixelColor(x, y, color)
	if x >= self.m_imgBuffer:GetWidth() or y >= self.m_imgBuffer:GetHeight() then
		return
	end
	self.m_imgBuffer:SetPixelColor(x, y, Color.Red)
end
function gui.UvAtlasMeshOverlay:DrawLine(color1, x1, y1, color2, x2, y2)
	local xdiff = (x2 - x1)
	local ydiff = (y2 - y1)

	if xdiff == 0.0 and ydiff == 0.0 then
		self:SetPixelColor(x1, y1, color1)
		return
	end

	if math.abs(xdiff) > math.abs(ydiff) then
		local xmin, xmax

		-- set xmin to the lower x value given
		-- and xmax to the higher value
		if x1 < x2 then
			xmin = x1
			xmax = x2
		else
			xmin = x2
			xmax = x1
		end

		-- draw line in terms of y slope
		local slope = ydiff / xdiff
		for x = xmin, xmax, 1.0 do
			local y = y1 + ((x - x1) * slope)
			local color = color1 + ((color2 - color1) * ((x - x1) / xdiff))
			self:SetPixelColor(x, y, color)
		end
	else
		local ymin, ymax

		-- set ymin to the lower y value given
		-- and ymax to the higher value
		if y1 < y2 then
			ymin = y1
			ymax = y2
		else
			ymin = y2
			ymax = y1
		end

		-- draw line in terms of x slope
		local slope = xdiff / ydiff
		for y = ymin, ymax, 1.0 do
			local x = x1 + ((y - y1) * slope)
			local color = color1 + ((color2 - color1) * ((y - y1) / ydiff))
			self:SetPixelColor(x, y, color)
		end
	end
end
function gui.UvAtlasMeshOverlay:OnUpdate()
	if util.is_valid(self.m_entity) == false or self.m_lightmapCache == nil then
		return
	end
	local mdl = self.m_entity:GetModel()
	if mdl == nil then
		return
	end
	self.m_imgBuffer = nil
	self.m_elTex:ClearTexture()
	collectgarbage()
	collectgarbage()
	local imgBuf = util.ImageBuffer.Create(self.m_width, self.m_height, util.ImageBuffer.FORMAT_RGBA8)
	imgBuf:Clear(Color.Clear)
	self.m_imgBuffer = imgBuf
	for _, mg in ipairs(mdl:GetMeshGroups()) do
		for _, m in ipairs(mg:GetMeshes()) do
			for _, sm in ipairs(m:GetSubMeshes()) do
				local lightmapUvs = self.m_lightmapCache:FindLightmapUvs(self.m_entity:GetUuid(), sm:GetUuid())
				if lightmapUvs ~= nil then
					local indices = sm:GetIndices()
					for i = 1, #indices, 3 do
						local idx0 = indices[i]
						local idx1 = indices[i + 1]
						local idx2 = indices[i + 2]
						local uv0 = lightmapUvs[idx0 + 1]
						local uv1 = lightmapUvs[idx1 + 1]
						local uv2 = lightmapUvs[idx2 + 1]
						self:DrawLine(
							Vector(1, 0, 0),
							uv0.x * imgBuf:GetWidth(),
							uv0.y * imgBuf:GetHeight(),
							Vector(0, 1, 0),
							uv1.x * imgBuf:GetWidth(),
							uv1.y * imgBuf:GetHeight()
						)
						self:DrawLine(
							Vector(1, 0, 0),
							uv1.x * imgBuf:GetWidth(),
							uv1.y * imgBuf:GetHeight(),
							Vector(0, 1, 0),
							uv2.x * imgBuf:GetWidth(),
							uv2.y * imgBuf:GetHeight()
						)
						self:DrawLine(
							Vector(1, 0, 0),
							uv2.x * imgBuf:GetWidth(),
							uv2.y * imgBuf:GetHeight(),
							Vector(0, 1, 0),
							uv0.x * imgBuf:GetWidth(),
							uv0.y * imgBuf:GetHeight()
						)
					end
				end
			end
		end
	end

	local img = prosper.create_image(imgBuf)
	local tex = prosper.create_texture(
		img,
		prosper.TextureCreateInfo(),
		prosper.ImageViewCreateInfo(),
		prosper.SamplerCreateInfo()
	)
	self.m_elTex:SetTexture(tex)
end
gui.register("WIUVAtlasMeshOverlay", gui.UvAtlasMeshOverlay)
