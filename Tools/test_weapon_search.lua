-- Regression test for HumanBehaviors.WeaponSearch: an AHuman without a FG arm must not walk to pick up a two-handed device.
-- Run from the repository root with: lua Tools/test_weapon_search.lua
-- The game API is stubbed out, so this only needs a plain Lua interpreter.

local failures = 0;

local function check(condition, message)
	if condition then
		print("PASS: " .. message);
	else
		failures = failures + 1;
		print("FAIL: " .. message);
	end
end

-- minimal vector with the members used by the behaviour
local function V(x, y)
	local v = {X = x, Y = y};
	v.Largest = math.max(math.abs(x), math.abs(y));
	v.SqrMagnitude = x * x + y * y;
	v.MagnitudeIsGreaterThan = function(self, n) return self.SqrMagnitude > n * n end;
	v.MagnitudeIsLessThan = function(self, n) return self.SqrMagnitude < n * n end;
	return v;
end

Actor = {AIMODE_NONE = 0, AIMODE_GOTO = 1, AIMODE_SENTRY = 2, AIMODE_SQUAD = 3, AIMODE_GOLDDIG = 4};
PathRequest = {NoSolution = 1};
FrameMan = {PlayerScreenWidth = 1024};

local devices = {};
local function makeDevice(id, oneHanded)
	local device = {
		UniqueID = id,
		ID = id,
		RootID = id,
		ClassName = "HDFirearm",
		Pos = V(50, 0),
		Vel = V(0, 0),
		IsPickupableBy = function() return true end,
		IsActivated = function() return false end,
		IsTool = function() return false end,
		IsDevice = function() return true end,
		IsOneHanded = function() return oneHanded end,
		HasObjectInGroup = function(self, group) return group == "Weapons - Primary" end,
	};
	devices[id] = device;
	return device;
end

MovableMan = {
	ValidMO = function(_, mo) return mo ~= nil end,
	FindObjectByUniqueID = function(_, id) return devices[id] end,
	GetMOsInRadius = function(_, pos, radius, team, ignoreSpawn)
		local list, i = {}, 0;
		for _, device in pairs(devices) do
			table.insert(list, device);
		end
		return function()
			i = i + 1;
			return list[i];
		end
	end,
};
function IsHeldDevice(mo) return mo ~= nil and mo.IsDevice ~= nil end
function ToHeldDevice(mo) return mo end

SceneMan = {
	SceneWrapsX = false,
	SceneWrapsY = false,
	ShortestDistance = function(_, a, b) return V(b.X - a.X, b.Y - a.Y) end,
	IsUnseen = function() return false end,
	MovePointToGround = function(_, point) return point end,
	Scene = {
		-- answers synchronously with a short, solvable path
		CalculatePathAsync = function(_, callback, from, to)
			callback({PathLength = 20, Status = 0});
		end,
	},
};

-- runs WeaponSearch for an AHuman with the given arms and devices; returns the device it chose to pick up
local function runWeaponSearch(hasFGArm, hasBGArm, device)
	devices = {};
	devices[device.UniqueID] = device;

	local Owner = {
		Pos = V(0, 0),
		Radius = 10,
		Height = 20,
		Team = 0,
		AIMode = Actor.AIMODE_GOTO,
		JumpHeight = 10,
		DigStrength = 1,
		FGArm = hasFGArm and {} or nil,
		BGArm = hasBGArm and {} or nil,
		MOMoveTarget = nil,
		IsWaitingOnNewMovePath = false,
		HasObjectInGroup = function() return false end,
		GetLastAIWaypoint = function() return V(0, 0) end,
		ClearAIWaypoints = function() end,
		AddAIMOWaypoint = function() end,
		AddAISceneWaypoint = function() end,
		UpdateMovePath = function() end,
	};
	local AI = {
		isPlayerOwned = false,
		CreateGoToBehavior = function() end,
	};

	local done = HumanBehaviors.WeaponSearch(AI, Owner, false);
	assert(done == true, "WeaponSearch should finish in one step with the stubs");
	return AI.PickupHD;
end

dofile("Data/Base.rte/AI/HumanBehaviors.lua");

-- a two-handed weapon cannot be picked up without an FG arm
local twoHanded = makeDevice(1, false);
local picked = runWeaponSearch(false, true, twoHanded);
check(picked == nil, "AI without FG arm does not go for a two-handed weapon");

-- with an FG arm the same weapon is still a valid target
picked = runWeaponSearch(true, true, makeDevice(2, false));
check(picked ~= nil and picked.UniqueID == 2, "AI with FG arm goes for a two-handed weapon");

-- a one-handed weapon can still be picked up with only a BG arm
picked = runWeaponSearch(false, true, makeDevice(3, true));
check(picked ~= nil and picked.UniqueID == 3, "AI without FG arm still goes for a one-handed weapon");

if failures > 0 then
	print(failures .. " check(s) failed");
	os.exit(1);
end
print("All WeaponSearch checks passed");
