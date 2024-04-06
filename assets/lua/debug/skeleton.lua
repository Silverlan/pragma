local function print_bone(bone, t)
	t = t or ""
	print(t .. bone:GetName())
	local tSub = t .. "\t"
	for boneId, child in pairs(bone:GetChildren()) do
		print_bone(child, tSub)
	end
end
debug.print_skeleton = function(skeleton)
	for boneId, bone in pairs(skeleton:GetRootBones()) do
		print_bone(bone)
	end
end
local function draw_skeleton(animC, rootPose, bone, parentPose, lines)
	local pose = rootPose * animC:GetEffectiveBonePose(bone:GetID())
	if parentPose ~= nil then
		table.insert(lines, parentPose:GetOrigin())
		table.insert(lines, pose:GetOrigin())
	end
	for boneId, child in pairs(bone:GetChildren()) do
		draw_skeleton(animC, rootPose, child, pose, lines)
	end
end
debug.draw_skeleton = function(ent, dur)
	local mdl = ent:GetModel()
	if mdl == nil then
		return
	end
	local animC = ent:GetComponent(ents.COMPONENT_ANIMATED)
	local skeleton = mdl:GetSkeleton()
	local parentPose = phys.ScaledTransform()
	local rootPose = ent:GetPose()
	local lines = {}
	for boneId, bone in pairs(skeleton:GetRootBones()) do
		draw_skeleton(animC, rootPose, bone, nil, lines)
	end
	debug.draw_lines(lines, Color.Red, dur or 12)
end
