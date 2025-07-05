-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.register_class("util.SurfaceMeshGenerator")
function util.SurfaceMeshGenerator:__init(w, h, nx, ny)
	ny = ny or nx
	self.m_width = w
	self.m_height = h
	self.m_numVertsX = nx
	self.m_numVertsY = ny

	self.m_verts = {}
	self.m_uvs = {}
	local dx = w / (nx - 1)
	local dy = h / (ny - 1)
	for x = 0, nx - 1 do
		for y = 0, ny - 1 do
			table.insert(self.m_verts, Vector(x * dx - w / 2.0, y * dy - h / 2.0, 0))
			table.insert(self.m_uvs, self:GetUVCoords(#self.m_verts - 1))
		end
	end

	self.m_tris = {}
	for x = 0, nx - 2 do
		for y = 0, ny - 2 do
			table.insert(self.m_tris, self:GetVertexIndex(x + 1, y))
			table.insert(self.m_tris, self:GetVertexIndex(x, y + 1))
			table.insert(self.m_tris, self:GetVertexIndex(x + 1, y + 1))

			table.insert(self.m_tris, self:GetVertexIndex(x, y))
			table.insert(self.m_tris, self:GetVertexIndex(x, y + 1))
			table.insert(self.m_tris, self:GetVertexIndex(x + 1, y))
		end
	end
end

function util.SurfaceMeshGenerator:GetVertexIndex(x, y)
	return y * self.m_numVertsX + x
end

function util.SurfaceMeshGenerator:GetVertexCoords(i)
	local y = math.floor(i / self.m_numVertsY)
	local x = i % self.m_numVertsX
	return x, y
end

function util.SurfaceMeshGenerator:GetUVCoords(i)
	local x, y = self:GetVertexCoords(i)
	return Vector2(y / (self.m_numVertsY - 1), 1.0 - x / (self.m_numVertsX - 1))
end

function util.SurfaceMeshGenerator:GetVertices()
	return self.m_verts
end
function util.SurfaceMeshGenerator:GetTriangles()
	return self.m_tris
end
function util.SurfaceMeshGenerator:GetUVCoordinates()
	return self.m_uvs
end

function util.SurfaceMeshGenerator:Generate(f)
	for i, v in ipairs(self.m_verts) do
		if f ~= nil then
			f(i, v, self.m_uvs[i])
		end
	end
end
