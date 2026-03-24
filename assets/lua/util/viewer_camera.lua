-- SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.viewer_camera = util.viewer_camera or {}

function util.viewer_camera.get_view_extents(ent)
	local renderC = util.is_valid(ent) and ent:GetComponent(ents.COMPONENT_RENDER) or nil
	local min = Vector()
	local max = Vector()
	if renderC ~= nil then
		min, max = renderC:GetLocalRenderBounds()
	else
		local ptC = util.is_valid(ent) and ent:GetComponent(ents.COMPONENT_PARTICLE_SYSTEM) or nil
		if ptC ~= nil then
			min, max = ptC:CalcRenderBounds()
		end
	end
	if min:Distance(max) < 0.0001 then
		local d = util.metres_to_units(5)
		min = Vector(-d, 0, -d)
		max = Vector(d, d, d)
	end
	return min, max
end

function util.viewer_camera.calc_fitting_distance(axisLength, fov, aspectRatio)
	local xFov = math.rad(fov)
	local yFov = xFov * aspectRatio
	local yZoom = axisLength / 2.0 / math.tan(yFov / 2.0)
	local xZoom = axisLength / 2.0 / math.tan(xFov / 2.0)
	return math.max(xZoom, yZoom)
end

function util.viewer_camera.calc_fitting_distance_from_extents(min, max, fov, aspectRatio)
	local maxAxisLength = math.max(max.x - min.x, max.y - min.y, max.z - min.z)
	return util.viewer_camera.calc_fitting_distance(maxAxisLength, fov, aspectRatio)
end

function util.viewer_camera.rotate_around(vec, yRot, xRot)
	vec:RotateAround(Vector(), EulerAngles(-math.deg(yRot), -math.deg(xRot), 0))
end

function util.viewer_camera.calc_camera_position(lookAtTarget, camDistance, yRot, xRot)
	-- calculate direction from focus to camera (assuming camera is at positive z)
	-- yRot rotates *around* x-axis, xRot rotates *around* y-axis
	local direction = Vector(0, 0, 1)
	util.viewer_camera.rotate_around(direction, yRot, xRot)
	local pos = direction * camDistance
	pos = pos + lookAtTarget
	return pos
end



