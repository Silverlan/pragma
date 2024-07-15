tests = tests or {}

util.register_class("tests.TestManager", util.CallbackHandler)
function tests.TestManager:__init()
	util.CallbackHandler.__init(self)
	self.m_testQueue = {}
end

function tests.TestManager:__finalize()
	self:Clear()
end

function tests.TestManager:AddTest(name, fc)
	table.insert(self.m_testQueue, {
		name = name,
		fc = fc,
	})
end

function tests.TestManager:Clear()
	util.remove(self.m_cbThink)
end

function tests.TestManager:IsTestActive()
	return self:GetActiveTest() ~= nil
end

function tests.TestManager:GetActiveTest()
	return self.m_currentTest
end

function tests.TestManager:CompleteTest(success, msg)
	local curTest = self.m_currentTest
	self.m_currentTest = nil
	if success == false then
		error("Test '" .. self.m_currentTest .. "' failed: " .. tostring(msg))
	end

	print("Test '" .. curTest .. "' has completed!")
	self:CallCallbacks("OnTestComplete", success, msg)

	if #self.m_testQueue == 0 then
		print("All tests have been completed!")
		self:CallCallbacks("OnAllTestsComplete")
	end
end

function tests.TestManager:StartTest(test)
	print("Starting test '" .. test.name .. "'...")
	self.m_currentTest = test.name
	self:CallCallbacks("OnTestStart", test.name)

	local retVals
	if test.fc ~= nil then
		retVals = { test.fc() }
	else
		retVals = { include(test.name) }
	end
	if retVals[1] ~= nil then
		self:CompleteTest(retVals[1], retVals[2])
	end
end

function tests.TestManager:Start()
	if util.is_valid(self.m_cbThink) then
		return
	end
	self.m_cbThink = game.add_callback("Think", function()
		if self:IsTestActive() == false then
			self:RunNextTest()
		end
	end)
end

function tests.TestManager:RunNextTest()
	if #self.m_testQueue == 0 then
		-- self:Clear()
		return
	end
	local test = self.m_testQueue[1]
	table.remove(self.m_testQueue, 1)
	self:StartTest(test)
end

tests.manager = tests.manager or tests.TestManager()
tests.add_event_listener = function(...)
	return tests.manager:AddCallback(...)
end
tests.is_test_running = function()
	return tests.manager:IsTestActive()
end
tests.queue = function(name, fc)
	tests.manager:AddTest(name, fc)
end
tests.run = function(scriptName)
	tests.manager:Start()
end
tests.complete = function(success, errMsg)
	tests.manager:CompleteTest(success, errMsg)
end
