-- Regression test: CrabBehaviors.Sentry must finish its sweep when the crab's aim
-- is clamped to narrowed (and tilted) AimRangeUpperLimit/AimRangeLowerLimit values.
-- Run from the repository root: lua Tools/test_crab_sentry_sweep.lua

-- Minimal engine stand-ins so the behavior script can be loaded outside the game.
local CTRL_UP, CTRL_DOWN, CTRL_SHARP = 1, 2, 3
Controller = { AIM_UP = CTRL_UP, AIM_DOWN = CTRL_DOWN, AIM_SHARP = CTRL_SHARP }
ACrab = { AIMING = "aiming", POINTING = "pointing" }
AHuman = { AIMING = "aiming" }
SceneMan = { CastStrengthRay = function() return false end }
Vector = setmetatable({}, { __call = function()
	return { RadRotate = function(self) return self end }
end })
Timer = function() return {} end
RangeRand = function(a, b) return a end

dofile("Data/Base.rte/AI/CrabBehaviors.lua")

local FRAME_LIMIT = 5000
local STEP = 0.05

local function clamp(value, lo, hi)
	return math.max(lo, math.min(hi, value))
end

-- Runs one sentry sweep for a crab with the given aim configuration.
-- Returns true if the behavior reached the scan phase (deviceState == POINTING).
local function sweepFinishes(label, upperLimit, lowerLimit, rotAngle, hFlipped)
	local aim = 0
	local pressed = {}
	local owner = {
		AimRange = math.pi / 2,
		AimRangeUpperLimit = upperLimit,
		AimRangeLowerLimit = lowerLimit,
		RotAngle = rotAngle,
		HFlipped = hFlipped,
		EyePos = Vector(),
		Pos = Vector(),
	}
	-- Mirrors ACrab::Update (Source/Entities/ACrab.cpp, the adjustedAimRange* lines around 867-868 and the
	-- final Clamp around 922): the aim is clamped to the adjusted limits every frame. This is a hand-copied
	-- formula, not a call into the engine; the sign and mirror convention was checked by reading that code.
	local function adjustedLimits()
		if hFlipped then
			return upperLimit - rotAngle, -lowerLimit - rotAngle
		end
		return upperLimit + rotAngle, -lowerLimit + rotAngle
	end
	function owner:GetAimAngle(_)
		return aim
	end

	local ai = {
		isPlayerOwned = true,
		SentryFacing = hFlipped,
		Ctrl = {
			SetState = function(self, state, value)
				pressed[state] = value
			end,
		},
	}

	local co = coroutine.create(CrabBehaviors.Sentry)
	for _ = 1, FRAME_LIMIT do
		if coroutine.status(co) == "dead" then
			break
		end
		-- Mirrors Controller::Update: command states are cleared each frame, then the AI sets them.
		pressed = {}
		coroutine.resume(co, ai, owner, false)

		-- Mirrors ACrab::Update: AIM_UP and AIM_DOWN are an if / else-if pair, then aim is clamped.
		local maxAng, minAng = adjustedLimits()
		if pressed[CTRL_UP] then
			aim = aim + STEP
		elseif pressed[CTRL_DOWN] then
			aim = aim - STEP
		end
		aim = clamp(aim, minAng, maxAng)

		if ai.deviceState == ACrab.POINTING then
			return true
		end
	end

	local maxAng, minAng = adjustedLimits()
	print(string.format("FAIL: %s: CrabBehaviors.Sentry did not finish its sweep within %d frames (last aim %.3f, limits %.3f..%.3f)",
		label, FRAME_LIMIT, aim, minAng, maxAng))
	return false
end

local cases = {
	{ "untilted, limits 0.3", 0.3, 0.3, 0, false },
	{ "tilted down (rot -0.2)", 0.3, 0.3, -0.2, false },
	{ "tilted up (rot 0.2)", 0.3, 0.3, 0.2, false },
	{ "flipped, tilted (rot 0.1)", 0.3, 0.3, 0.1, true },
}

local failures = 0
for _, case in ipairs(cases) do
	if not sweepFinishes(table.unpack(case)) then
		failures = failures + 1
	end
end

if failures > 0 then
	print(string.format("%d of %d sentry sweep cases failed", failures, #cases))
	os.exit(1)
end
print(string.format("PASS: all %d sentry sweep cases finished", #cases))
