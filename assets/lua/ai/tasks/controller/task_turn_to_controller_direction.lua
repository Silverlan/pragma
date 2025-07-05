-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.register_class("ai.TaskTurnToControllerDirection", ai.BaseBehaviorTask)
include("task_get_controller_look_target.lua")
function ai.TaskTurnToControllerDirection:__tostring()
	return "Turn to controller direction"
end
function ai.TaskTurnToControllerDirection:__init(taskType, selectorType)
	ai.BaseBehaviorTask.__init(self, taskType, selectorType)

	local taskGetTarget = self:CreateTask(ai.TASK_GET_CONTROLLER_LOOK_TARGET)
	local taskLook = self:CreateTask(ai.TASK_TURN_TO_TARGET)
	taskGetTarget:LinkParameter(ai.TaskGetControllerLookTarget.RETURN_LOOK_TARGET, taskLook, 0)
end
ai.TASK_TURN_TO_CONTROLLER_DIRECTION = ai.TASK_TURN_TO_CONTROLLER_DIRECTION
	or ai.register_task(ai.TaskTurnToControllerDirection)
