-- SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

include("/tests/pfm/base.lua")

local function set_animated(ent, animationInfo)
	local channel = panima.Channel()
	channel:SetValueType(animationInfo.valueType)
	channel:InsertValues(animationInfo.times, animationInfo.values)
	channel:SetTargetPath(animationInfo.propertyPath)
	channel:Update()

	local anim = panima.Animation.create()
	anim:AddChannel(channel)
	anim:SetDuration(0.0)

	local panimaC = ent:AddComponent(ents.COMPONENT_PANIMA)
	local animManager = panimaC:AddAnimationManager("test")
	animManager:GetPlayer():SetLooping(true)
	panimaC:PlayAnimation(animManager, anim)
	panimaC:SetPropertyAlwaysDirty(animationInfo.propertyPath, true)
end

local function add_constraint(config, driver, driven)
	local entC = ents.create("entity")
	local c = entC:AddComponent(config.type)
	
	local ctC = entC:GetComponent(ents.COMPONENT_CONSTRAINT)
	
	local driver_uri = config.driver_uri or "ec/transform/position"
	local driven_uri = config.driven_uri or "ec/transform/position"
	
	ctC:SetMemberValue("driver", ents.UniversalMemberReference(ents.create_uri(driver:GetUuid(), driver_uri)))
	ctC:SetMemberValue("drivenObject", ents.UniversalMemberReference(ents.create_uri(driven:GetUuid(), driven_uri)))
	
	ctC:SetMemberValue("driverSpace", config.driver_space or math.COORDINATE_SPACE_WORLD)
	ctC:SetMemberValue("drivenObjectSpace", config.driven_space or math.COORDINATE_SPACE_WORLD)
	
	if config.propertiesBase then
		for propName, propValue in pairs(config.propertiesBase) do
			ctC:SetMemberValue(propName, propValue)
		end
	end
	if config.properties then
		for propName, propValue in pairs(config.properties) do
			c:SetMemberValue(propName, propValue)
		end
	end

	entC:Spawn()
	return entC
end

local function animate_driver(entDriver, srcPose, dstPose, duration, onComplete)
	local tm = time.real_time()
	local cb
	cb = game.add_callback("Think", function()
		local dt = time.real_time() - tm
		local f = math.clamp(dt / duration, 0.0, 1.0)
		
		local newPose = srcPose:Copy()
		newPose:Interpolate(dstPose, f)
		entDriver:SetPose(newPose)
		
		if f == 1.0 then
			util.remove(cb)
			time.create_simple_timer(0.2, function() onComplete() end)
		end
	end)
end

local function run_test(case, onComplete)
    tests.log_info("Running Test: {}", case.name)
	local resources = {}

	local entDriver = ents.create_prop("cube")
	entDriver:SetPose(case.driver_start)
	if(case.driver_animation ~= nil) then
		set_animated(entDriver, case.driver_animation)
	end
	table.insert(resources, entDriver)

	local entDriven = ents.create_prop("cube")
	entDriven:SetPose(case.driven_start)
	if(case.driven_animation ~= nil) then
		set_animated(entDriven, case.driven_animation)
	end
	table.insert(resources, entDriven)

	local entConstraint = add_constraint(case, entDriver, entDriven)
	table.insert(resources, entConstraint)

	local animEnt = (case.target_to_animate == "driven") and entDriven or entDriver
	local startPose = (case.target_to_animate == "driven") and case.driven_start or case.driver_start
	local endPose = (case.target_to_animate == "driven") and case.driven_end or case.driver_end

	animate_driver(animEnt, startPose, endPose, case.duration or 1.0, function()
		local finalPose = entDriven:GetPose()
		
		local dist = finalPose:GetOrigin():Distance(case.expected_pose:GetOrigin())
		local angularDist = finalPose:GetRotation():Distance(case.expected_pose:GetRotation())
		
		local scaleDist = 0
		if finalPose.GetScale and case.expected_pose.GetScale then
			scaleDist = finalPose:GetScale():Distance(case.expected_pose:GetScale())
		end

		local success = (dist < 0.1 and angularDist < 0.1 and scaleDist < 0.1)

		if success then
            tests.log_info("  -> PASSED")
		else
            tests.log_error("  -> FAILED")
            tests.log_info("	 Expected Origin: {} Got: {}", case.expected_pose:GetOrigin(), finalPose:GetOrigin())
			tests.log_info("	 Expected Rot: {} Got: {}", case.expected_pose:GetRotation():ToEulerAngles(), finalPose:GetRotation():ToEulerAngles())
		end

		util.remove(resources)
		onComplete(success)
	end)
end

local function run_all_tests(cases, onFinished)
	local index = 1
	local failures = 0

	local function next_test()
		if index > #cases then
            tests.log_info("Tests complete ({}/{} Passed)", #cases - failures, #cases)
			if onFinished then onFinished(failures == 0) end
			return
		end

		run_test(cases[index], function(success)
			if not success then failures = failures + 1 end
			index = index + 1
			next_test()
		end)
	end

    tests.log_info("Starting {} cases", #cases)
	next_test()
end

local POSE_ZERO = math.ScaledTransform(Vector(0,0,0), EulerAngles(0,0,0):ToQuaternion())
local POSE_A = math.ScaledTransform(Vector(20,10,8), EulerAngles(10,5,7):ToQuaternion())
local POSE_B = math.ScaledTransform(Vector(50,0,0), EulerAngles(0,90,0):ToQuaternion())

local function MakeTransform(pos, rot, scale)
	local t = math.ScaledTransform(pos, rot)
	if t.SetScale then t:SetScale(scale or Vector(1,1,1)) end
	return t
end

local TEST_CASES = {
	{
		name = "Copy Location",
		type = ents.COMPONENT_CONSTRAINT_COPY_LOCATION,
		driver_start = POSE_ZERO,
		driver_end = POSE_B,
		driven_start = POSE_A,
		expected_pose = MakeTransform(POSE_B:GetOrigin(), POSE_A:GetRotation())
	},
	{
		name = "Copy Rotation (50% Influence)",
		type = ents.COMPONENT_CONSTRAINT_COPY_ROTATION,
		driver_uri = "ec/transform/rotation",
		driven_uri = "ec/transform/rotation",
		driver_start = POSE_ZERO,
		driver_end = MakeTransform(Vector(0,0,0), EulerAngles(0,90,0):ToQuaternion()),
		driven_start = POSE_ZERO,
		driven_animation = {
			valueType = udm.TYPE_QUATERNION,
			times = {0},
			values = {Quaternion()},
			propertyPath = "ec/transform/rotation",
		},
		expected_pose = MakeTransform(Vector(0,0,0), EulerAngles(0,45,0):ToQuaternion()),
		propertiesBase = { influence = 0.5 }
	},

	{
		name = "Limit Distance (Inside)",
		type = ents.COMPONENT_CONSTRAINT_LIMIT_DISTANCE,
		driver_start = MakeTransform(Vector(0,0,0), EulerAngles(0,0,0):ToQuaternion()),
		driver_end = MakeTransform(Vector(100,0,0), EulerAngles(0,0,0):ToQuaternion()),
		driven_start = MakeTransform(Vector(0,0,0), EulerAngles(0,0,0):ToQuaternion()),
		
		expected_pose = MakeTransform(Vector(70,0,0), EulerAngles(0,0,0):ToQuaternion()),
		properties = {
			distance = 30.0,
			clampRegion = ents.ConstraintLimitDistanceComponent.CLAMP_REGION_INSIDE
		}
	},

	{
		name = "Limit Location (Max X)",
		type = ents.COMPONENT_CONSTRAINT_LIMIT_LOCATION,
		target_to_animate = "driven",
		
		driver_start = POSE_ZERO,
		driver_end = POSE_ZERO, 
		
		driven_start = POSE_ZERO,
		driven_end = MakeTransform(Vector(100, 50, -50), EulerAngles(0,0,0):ToQuaternion()),
		
		expected_pose = MakeTransform(Vector(25, 50, -50), EulerAngles(0,0,0):ToQuaternion()),
		properties = {
			maximumXEnabled = true, maximumX = 25.0,
			minimumXEnabled = false, minimumX = 0.0,
		}
	},
	{
		name = "Look At (Track Z to Driver)",
		type = ents.COMPONENT_CONSTRAINT_LOOK_AT,
		driver_start = POSE_ZERO,
		driver_end = MakeTransform(Vector(50, 50, 50), EulerAngles(0,0,0):ToQuaternion()),
		
		driven_start = POSE_ZERO,
		
		expected_pose = MakeTransform(Vector(0,0,0), EulerAngles(-35.2644, 45, 0):ToQuaternion()),
		properties = {
			trackAxis = ents.ConstraintLookAtComponent.TRACK_AXIS_Z
		}
	},
	{
		name = "Child Of (Offset copy)",
		type = ents.COMPONENT_CONSTRAINT_CHILD_OF,
		
		driver_start = POSE_ZERO,
		driver_end = MakeTransform(Vector(50, 10, 0), EulerAngles(0, 0, 0):ToQuaternion()),
		
		driven_start = MakeTransform(Vector(10, 0, 0), EulerAngles(13, 35, 24):ToQuaternion()),
		
		driven_animation = {
			valueType = udm.TYPE_VECTOR3,
			times = {0},
			values = {Vector(10,5,4)},
			propertyPath = "ec/transform/position",
		},

		expected_pose = MakeTransform(Vector(50 +10, 10 +5, 0 +4), EulerAngles(13, 35, 24):ToQuaternion()),
		propertiesBase = {
			influence = 1.0
		}
	}
}

run_all_tests(TEST_CASES, function(all_passed)
	if(all_passed) then
		tests.complete(true)
	else
		tests.complete(false, "One or more constraint cases have failed. Check log for more information.")
	end
end)
