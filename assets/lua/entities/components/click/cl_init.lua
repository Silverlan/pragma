--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

console.register_command("debug_click_raycast", function(pl, key, cmd)
	local pos, dir, vpData = ents.ClickComponent.get_ray_data()
	if pos == nil then
		console.print_warning("Failed to determine ray data!")
		return
	end
	print("Ray start position: ", pos)
	print("Ray direction: ", dir)
	local drawInfo = debug.DrawInfo()
	drawInfo:SetDuration(12)
	drawInfo:SetColor(Color.Aqua)
	debug.draw_line(pos, pos + dir * 1000, drawInfo)
end)

util.register_class("ents.ClickComponent", BaseEntityComponent)
ents.ClickComponent:RegisterMember("Priority", udm.TYPE_UINT32, 0, {}, ents.BaseEntityComponent.MEMBER_FLAG_DEFAULT)

ents.ClickComponent.impl = ents.ClickComponent.impl or {}
ents.ClickComponent.impl.numClickComponents = 0
function ents.ClickComponent:Initialize()
	BaseEntityComponent.Initialize(self)
	if ents.ClickComponent.impl.numClickComponents == 0 then
		local pl = ents.get_local_player()
		if pl ~= nil then
			local actionInputC = pl:GetEntity():GetComponent(ents.COMPONENT_ACTION_INPUT_CONTROLLER)
			if actionInputC ~= nil then
				ents.ClickComponent.cbClick = actionInputC:AddEventCallback(
					ents.ActionInputControllerComponent.EVENT_HANDLE_ACTION_INPUT,
					function(action, pressed, magnitude)
						local handled = ents.ClickComponent.inject_click_input(action, pressed)
						return handled
					end
				)
			end
		end
	end
	ents.ClickComponent.impl.numClickComponents = ents.ClickComponent.impl.numClickComponents + 1
end
local function get_viewport_data(vp)
	local vpData = {}
	if vp ~= nil then
		local absPos = vp:GetAbsolutePos()
		vpData.width = vp:GetWidth()
		vpData.height = vp:GetHeight()
		vpData.x = absPos.x
		vpData.y = absPos.y
		vpData.cursorPos = vp:GetCursorPos()
		vpData.camera = vp:GetCamera()
		vpData.vp = vp
		return vpData
	end
	local cursorPos = input.get_cursor_pos()
	-- Check if element under cursor is a viewport
	local elFocus = gui.get_element_under_cursor(function(el)
		return el:GetClass() == "wiviewport"
	end)
	if util.is_valid(elFocus) then
		local viewport
		if elFocus:GetClass() == "wiviewport" then
			viewport = elFocus
		else
			-- Check if the focused element has a viewport
			for vp, _ in pairs(gui.WIViewport.get_viewports()) do
				if vp:IsValid() and vp:IsVisible() and vp:IsDescendantOf(elFocus) then
					if viewport == nil then
						viewport = vp
					else
						-- Found more than one viewport; Pick the closest one to the mouse cursor
						local pos0 = viewport:GetAbsolutePos()
						local d0 = geometry.closest_point_on_aabb_to_point(pos0, pos0 + viewport:GetSize(), cursorPos)

						local pos1 = vp:GetAbsolutePos()
						local d1 = geometry.closest_point_on_aabb_to_point(pos1, pos1 + vp:GetSize(), cursorPos)
						if d1 < d0 then -- Viewport is closer
							viewport = vp
						end
					end
				end
			end
		end
		if viewport ~= nil then
			-- Found viewport! Using viewport as base for click input
			local absPos = viewport:GetAbsolutePos()
			vpData.width = viewport:GetWidth()
			vpData.height = viewport:GetHeight()
			vpData.x = absPos.x
			vpData.y = absPos.y
			vpData.cursorPos = viewport:GetCursorPos()
			vpData.camera = viewport:GetCamera()
			vpData.viewport = viewport
			return vpData
		end
	end

	local scene = game.get_render_scene()
	local cam = scene:GetActiveCamera()
	local res = engine.get_window_resolution()
	vpData.width = res.x
	vpData.height = res.y
	vpData.x = 0
	vpData.y = 0
	vpData.cursorPos = input.get_cursor_pos()
	vpData.camera = cam
	return vpData
end
local lastActorsClicked = {}
function ents.ClickComponent.inject_click_input(action, pressed, filter)
	if action ~= input.ACTION_ATTACK and action ~= input.ACTION_ATTACK2 then
		return util.EVENT_REPLY_UNHANDLED
	end
	if util.is_valid(lastActorsClicked[action]) then
		local actor = lastActorsClicked[action]
		lastActorsClicked[action] = nil
		local handled = actor:BroadcastEvent(ents.ClickComponent.EVENT_ON_CLICK, { action, false })
		if pressed == false and handled == util.EVENT_REPLY_HANDLED then
			return handled
		end
	end
	if pressed == false then
		return util.EVENT_REPLY_UNHANDLED
	end
	local clickActor, hitPos, startPos, hitData = ents.ClickComponent.find_actor_under_cursor(filter)
	local clickC = (clickActor ~= nil) and clickActor:GetComponent(ents.COMPONENT_CLICK) or nil
	if clickC ~= nil then
		if pressed then
			lastActorsClicked[action] = clickC
		else
			lastActorsClicked[action] = nil
		end
		return (
			clickC:BroadcastEvent(ents.ClickComponent.EVENT_ON_CLICK, { action, true, hitPos, hitData })
			or util.EVENT_REPLY_UNHANDLED
		),
			clickActor,
			hitPos,
			startPos,
			hitData
	end
	return util.EVENT_REPLY_UNHANDLED, clickActor, hitPos, startPos, hitData
end
function ents.ClickComponent.world_space_point_to_screen_space_uv(point, callback, vpData)
	vpData = vpData or get_viewport_data()

	local cam = vpData.camera
	if util.is_valid(cam) == false then
		return
	end
	if callback ~= nil then
		callback(vpData, cam)
	end
	return cam:WorldSpaceToScreenSpace(point)
end
function ents.ClickComponent.get_ray_data(callback)
	local vpData = get_viewport_data()

	local cam = vpData.camera
	if util.is_valid(cam) == false then
		return
	end
	if callback ~= nil then
		callback(vpData, cam)
	end
	local uv = Vector2(vpData.cursorPos.x / vpData.width, vpData.cursorPos.y / vpData.height)
	local dir = cam:CalcRayDirection(uv)
	local pos = cam:GetPlanePoint(cam:GetNearZ(), uv)
	return pos, dir, vpData
end
function ents.ClickComponent.get_camera()
	local vpData = get_viewport_data()
	return vpData.camera
end
function ents.ClickComponent.get_viewport_data(vp)
	return get_viewport_data(vp)
end
local function get_local_planes(planes, ent)
	local pose = ent:GetPose():GetInverse()
	local localPlanes = {}
	for _, plane in ipairs(planes) do
		table.insert(localPlanes, pose * plane)
	end
	return localPlanes
end
local function should_entity_pass(ent, entPl, filter)
	local mdl = ent:GetModel()
	local renderC = ent:GetComponent(ents.COMPONENT_RENDER)
	return mdl ~= nil
		and ent ~= entPl
		and renderC ~= nil
		and renderC:GetSceneRenderPass() ~= game.SCENE_RENDER_PASS_VIEW
		and renderC:GetSceneRenderPass() ~= game.SCENE_RENDER_PASS_NONE
		and (filter == nil or filter(ent, renderC) == true)
		and (
			ent:HasComponent(ents.COMPONENT_STATIC_BVH_USER) == false
			or ent:GetComponent(ents.COMPONENT_STATIC_BVH_USER):IsActive() == false
		)
end
function ents.ClickComponent.find_entities_in_kdop(planes, filter)
	local pl = ents.get_local_player()
	if pl == nil then
		return
	end
	local entPl = pl:GetEntity()

	debug.start_profiling_task("click_find_entities_in_kdop")
	local flags = ents.IntersectionHandlerComponent.INTERSECTION_FLAG_BIT_RETURN_MESHES
	local results = {}
	local entToResultIdx = {}
	local meshMap = {}
	local function populate_results(meshes)
		for _, meshInfo in ipairs(meshes) do
			local ent = meshInfo.entity
			local subMesh = meshInfo.mesh
			if subMesh ~= nil then
				local idx = entToResultIdx[ent]
				if idx == nil then
					table.insert(results, {
						entity = ent,
						meshes = {},
					})
					entToResultIdx[ent] = #results
					idx = #results
				end
				local uuid = tostring(subMesh:GetUuid())
				if meshMap[uuid] == nil then
					table.insert(results[idx].meshes, subMesh)
					meshMap[uuid] = true
				end
			end
		end
	end
	for ent, c in ents.citerator(ents.COMPONENT_STATIC_BVH_CACHE) do
		local intersectionC = ent:GetComponent(ents.COMPONENT_INTERSECTION_HANDLER)
		if intersectionC ~= nil then
			local localPlanes = get_local_planes(planes, ent)
			local r, meshes = intersectionC:IntersectionTestKDop(localPlanes, flags)
			if r ~= false then
				populate_results(meshes)
			end
		end
	end

	for ent, c in
		ents.citerator(ents.COMPONENT_INTERSECTION_HANDLER, {
			ents.IteratorFilterComponent(ents.COMPONENT_CLICK),
			ents.IteratorFilterComponent(ents.COMPONENT_MODEL),
			ents.IteratorFilterComponent(ents.COMPONENT_RENDER),
		})
	do
		if should_entity_pass(ent, entPl, filter) then
			local localPlanes = get_local_planes(planes, ent)
			local r, meshes = c:IntersectionTestKDop(localPlanes, flags)
			if r ~= false then
				populate_results(meshes)
			end
		end
	end
	debug.stop_profiling_task()
	return results
end
function ents.ClickComponent.is_entity_valid(ent)
	return util.is_valid(ent) and ent:IsTurnedOn()
end
function ents.ClickComponent.raycast(pos, dir, filter, maxDist)
	maxDist = maxDist or 32768.0
	local pl = ents.get_local_player()
	if pl == nil then
		return
	end
	local entPl = pl:GetEntity()
	--[[local drawInfo = debug.DrawInfo()
	drawInfo:SetDuration(12)
	drawInfo:SetColor(Color.Aqua)
	debug.draw_line(pos,pos +dir *1000,drawInfo)]]
	local distClosest = math.huge
	local priorityClosest = 0
	local actorClosest = nil
	local hitDataClosest
	local hitPos
	debug.start_profiling_task("click_component_picking")

	-- Check static BVH caches
	for ent, c in ents.citerator(ents.COMPONENT_STATIC_BVH_CACHE) do
		local intersectionC = ent:GetComponent(ents.COMPONENT_INTERSECTION_HANDLER)
		if intersectionC ~= nil then
			local hitData = intersectionC:IntersectionTest(pos, dir, 0.0, maxDist)
			if
				hitData ~= nil
				and ents.ClickComponent.is_entity_valid(hitData.entity)
				and (filter == nil or filter(hitData.entity))
			then
				if hitData.distance < distClosest then -- and hitData.distance > 0.0) then
					--debug.print("Clicked actor: ",hitData.entity)
					distClosest = hitData.distance
					hitPos = pos + dir * hitData.distance
					actorClosest = hitData.entity
					hitDataClosest = hitData
				end
			end
		end
	end

	-- Check dynamic actors
	local function testEntity(ent)
		local mdl = ent:GetModel()
		local renderC = ent:GetComponent(ents.COMPONENT_RENDER)
		if ents.ClickComponent.is_entity_valid(ent) and should_entity_pass(ent, entPl, filter) then
			local scale = ent:GetScale()
			if scale.x > 0.001 and scale.y > 0.001 and scale.z > 0.001 then
				local pose = ent:GetPose():GetInverse()
				pose:SetScale(Vector(1, 1, 1))

				-- Move ray into entity space
				local lpos = pose * pos
				local ldir = dir:Copy()
				ldir:Rotate(pose:GetRotation())

				lpos = Vector(lpos.x / scale.x, lpos.y / scale.y, lpos.z / scale.z)
				ldir = ldir * maxDist
				ldir = Vector(ldir.x / scale.x, ldir.y / scale.y, ldir.z / scale.z)
				local lMaxDist = ldir:Length()
				ldir = ldir / lMaxDist

				local intersectionHandlerC = ent:GetComponent(ents.COMPONENT_INTERSECTION_HANDLER)
				local hitData = intersectionHandlerC:IntersectionTest(lpos, ldir, 0.0, lMaxDist)
				if hitData ~= nil then
					local clickC = ent:GetComponent(ents.COMPONENT_CLICK)
					local priority = (clickC ~= nil) and clickC:GetPriority() or 0
					local hitDist = hitData.distance

					if math.abs(scale:LengthSqr() - 1.0) > 0.001 then
						-- Object is scaled; We have to calculate hit distance
						-- for unscaled space
						local lhitPos = lpos + ldir * hitDist
						lhitPos = lhitPos * scale
						lhitPos = pose:GetInverse() * lhitPos
						local diff = lhitPos - pos
						hitDist = diff:Length()
					end

					if priority > priorityClosest or (priority == priorityClosest and hitDist < distClosest) then -- and hitData.distance > 0.0) then
						distClosest = hitDist
						hitPos = pos + dir * hitDist
						actorClosest = hitData.entity
						hitDataClosest = hitData
						priorityClosest = priority
					end
				end

				--[[local r,hitData = renderC:CalcRayIntersection(pos,dir *maxDist,false)
				-- print("Intersection with ",ent,": ",r)
				-- Note: Distance of 0 usually means we're inside the object, in which case we probably don't intend to select it
				if(r == intersect.RESULT_INTERSECT and hitData.distance < distClosest) then -- and hitData.distance > 0.0) then
					-- print("Clicked actor: ",ent)
					distClosest = hitData.distance
					hitPos = hitData.position
					hitDataClosest = hitData
					actorClosest = ent
				end]]
			end
		end
	end

	local entCache = {}
	for ent in
		ents.iterator({
			ents.IteratorFilterComponent(ents.COMPONENT_INTERSECTION_HANDLER),
			ents.IteratorFilterComponent(ents.COMPONENT_CLICK),
			ents.IteratorFilterComponent(ents.COMPONENT_MODEL),
			ents.IteratorFilterComponent(ents.COMPONENT_RENDER),
		})
	do
		entCache[ent:GetLocalIndex()] = true
		testEntity(ent)
	end

	--[[for ent in ents.iterator({ents.IteratorFilterComponent(ents.COMPONENT_PANIMA),ents.IteratorFilterComponent(ents.COMPONENT_CLICK),ents.IteratorFilterComponent(ents.COMPONENT_MODEL),ents.IteratorFilterComponent(ents.COMPONENT_RENDER)}) do
		if(entCache[ent:GetLocalIndex()] ~= true) then testEntity(ent) end
	end

	for ent in ents.iterator({ents.IteratorFilterComponent(ents.COMPONENT_PANIMA),ents.IteratorFilterComponent(ents.COMPONENT_CLICK),ents.IteratorFilterComponent(ents.COMPONENT_MODEL),ents.IteratorFilterComponent(ents.COMPONENT_RENDER)}) do
		if(entCache[ent:GetLocalIndex()] ~= true) then testEntity(ent) end
	end

	for ent in ents.iterator({ents.IteratorFilterComponent(ents.COMPONENT_PHYSICS),ents.IteratorFilterComponent(ents.COMPONENT_CLICK),ents.IteratorFilterComponent(ents.COMPONENT_MODEL),ents.IteratorFilterComponent(ents.COMPONENT_RENDER)}) do
		if(ent:HasComponent(ents.COMPONENT_STATIC_BVH_USER) == false and entCache[ent:GetLocalIndex()] ~= true) then testEntity(ent) end
	end]]
	--

	--[[if(hitPos ~= nil) then
		local drawInfo = debug.DrawInfo()
		drawInfo:SetDuration(12)
		drawInfo:SetColor(Color.Aqua)
		debug.draw_line(hitPos,hitPos +Vector(0,100,0),drawInfo)
	end]]

	--[[for ent in ents.iterator({ents.IteratorFilterComponent(ents.COMPONENT_CLICK),ents.IteratorFilterComponent(ents.COMPONENT_MODEL),ents.IteratorFilterComponent(ents.COMPONENT_RENDER)}) do
		local mdl = ent:GetModel()
		local renderC = ent:GetComponent(ents.COMPONENT_RENDER)
		if(mdl ~= nil and ent ~= entPl and renderC ~= nil and renderC:GetSceneRenderPass() ~= game.SCENE_RENDER_PASS_VIEW and renderC:GetSceneRenderPass() ~= game.SCENE_RENDER_PASS_NONE and (filter == nil or filter(ent,renderC) == true)) then
			local r,hitData = renderC:CalcRayIntersection(pos,dir *32768,false)
			-- print("Intersection with ",ent,": ",r)
			-- Note: Distance of 0 usually means we're inside the object, in which case we probably don't intend to select it
			if(r == intersect.RESULT_INTERSECT and hitData.distance < distClosest) then -- and hitData.distance > 0.0) then
				-- print("Clicked actor: ",ent)
				distClosest = hitData.distance
				hitPos = hitData.position
				actorClosest = ent
			end
		end
	end]]
	debug.stop_profiling_task()
	return actorClosest, hitPos, pos, hitDataClosest
end
function ents.ClickComponent.find_actor_under_cursor(filter)
	local pos, dir, vpData = ents.ClickComponent.get_ray_data()
	--print("find_actor_under_cursor: ", pos, dir)
	if pos == nil then
		return
	end
	return ents.ClickComponent.raycast(pos, dir, filter)
end
function ents.ClickComponent:OnRemove()
	ents.ClickComponent.impl.numClickComponents = ents.ClickComponent.impl.numClickComponents - 1
	if ents.ClickComponent.impl.numClickComponents == 0 then
		if util.is_valid(ents.ClickComponent.cbClick) then
			ents.ClickComponent.cbClick:Remove()
		end
	end
end
ents.COMPONENT_CLICK = ents.register_component("click", ents.ClickComponent)
ents.ClickComponent.EVENT_ON_CLICK = ents.register_component_event(ents.COMPONENT_CLICK, "on_clicked")
