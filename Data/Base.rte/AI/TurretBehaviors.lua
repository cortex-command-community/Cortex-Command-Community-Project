
TurretBehaviors = {};

function TurretBehaviors.Patrol(AI, Owner, Abort)
	local TurnTimer = Timer();
	local turn = 3000;

	while true do
		if TurnTimer:IsPastSimMS(turn) then
			TurnTimer:Reset();
			turn = RangeRand(2000, 6000);
			Owner:SetAimAngle(RangeRand(-0.7, 0.7));
			Owner.HFlipped = not Owner.HFlipped;
		end

		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end
	end

	return true;
end

-- stop the user fom inadvertently modifying the storage table
local Proxy = {};
local Mt = {
	__index = TurretBehaviors,
	__newindex = function(Table, k, v)
		error("The TurretBehaviors table is read-only.", 2);
	end
};
setmetatable(Proxy, Mt);
TurretBehaviors = Proxy;