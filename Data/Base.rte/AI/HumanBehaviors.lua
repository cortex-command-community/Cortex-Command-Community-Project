
HumanBehaviors = {};

-- spot targets by casting a ray in a random direction
function HumanBehaviors.LookForTargets(AI, Owner, Skill)
	local viewAngDeg = RangeRand(50, 120) * Owner.Perceptiveness * (0.5 + Skill/200);
	if AI.deviceState == AHuman.AIMING then
		AI.Ctrl:SetState(Controller.AIM_SHARP, true); -- reinforce sharp aim controller state to enable SharpLength in LookForMOs
		viewAngDeg = 15 * Owner.Perceptiveness + (Skill/10);
	end

	local FoundMO = Owner:LookForMOs(viewAngDeg, rte.grassID, false);
	if FoundMO then
		local HitPoint = SceneMan:GetLastRayHitPos();
		-- AI-teams ignore the fog
		if not AI.isPlayerOwned or not SceneMan:IsUnseen(HitPoint.X, HitPoint.Y, Owner.Team) or not SceneMan:IsUnseen(FoundMO.Pos.X, FoundMO.Pos.Y, Owner.Team) then
			return FoundMO, HitPoint;
		end
	end
end

-- brains and snipers spot targets by casting rays at all nearby enemy actors
function HumanBehaviors.CheckEnemyLOS(AI, Owner, Skill)
	if not AI.Enemies then	-- add all enemy actors on our screen to a table and check LOS to them, one per frame
		AI.Enemies = {};
		local box = Box();
		local skillFactor = (0.4 + Skill * 0.005);
		box.Corner = Vector(Owner.ViewPoint.X - (FrameMan.PlayerScreenWidth/2) * skillFactor, Owner.ViewPoint.Y - (FrameMan.PlayerScreenHeight/2) * skillFactor);
		box.Width = FrameMan.PlayerScreenWidth * skillFactor;
		box.Height = FrameMan.PlayerScreenHeight * skillFactor;
		for Act in MovableMan:GetMOsInBox(box, Owner.Team, true) do
			if IsActor(Act) and not AI.isPlayerOwned or not SceneMan:IsUnseen(Act.Pos.X, Act.Pos.Y, Owner.Team) then	-- AI-teams ignore the fog
				table.insert(AI.Enemies, Act);
			end
		end

		return HumanBehaviors.LookForTargets(AI, Owner, Skill); -- cast rays like normal actors occasionally
	else
		local Enemy = table.remove(AI.Enemies);
		if Enemy then
			if MovableMan:ValidMO(Enemy) then
				local Origin;
				if Owner.EquippedItem and AI.deviceState == AHuman.AIMING then
					Origin = Owner.EquippedItem.Pos;
				else
					Origin = Owner.EyePos;
				end

				local LookTarget;
				if Enemy.ClassName == "ADoor" then
					-- TO-DO: use explosive weapons on doors?

					local Door = ToADoor(Enemy).Door;
					if Door and Door:IsAttached() then
						LookTarget = Door.Pos;
					else
						return HumanBehaviors.LookForTargets(AI, Owner, Skill); -- this door is destroyed, cast rays like normal actors
					end
				else
					LookTarget = Enemy.Pos;
				end

				-- cast at body
				if not AI.isPlayerOwned or not SceneMan:IsUnseen(LookTarget.X, LookTarget.Y, Owner.Team) then	-- AI-teams ignore the fog
					local Dist = SceneMan:ShortestDistance(Owner.ViewPoint, LookTarget, false);
					if (math.abs(Dist.X) - Enemy.Radius < FrameMan.PlayerScreenWidth * 0.52) and (math.abs(Dist.Y) - Enemy.Radius < FrameMan.PlayerScreenHeight * 0.52) then
						local Trace = SceneMan:ShortestDistance(Origin, LookTarget, false);
						local ID = SceneMan:CastMORay(Origin, Trace, Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, false, 5);
						if ID ~= rte.NoMOID then
							local MO = MovableMan:GetMOFromID(ID);
							if MO and ID ~= MO.RootID then
								MO = MovableMan:GetMOFromID(MO.RootID);
							end

							return MO, SceneMan:GetLastRayHitPos();
						end
					end
				end

				-- no LOS to the body, cast at head
				if Enemy.EyePos and (not AI.isPlayerOwned or not SceneMan:IsUnseen(Enemy.EyePos.X, Enemy.EyePos.Y, Owner.Team)) then	-- AI-teams ignore the fog
					local Dist = SceneMan:ShortestDistance(Owner.ViewPoint, Enemy.EyePos, false);
					if (math.abs(Dist.X) < FrameMan.PlayerScreenWidth * 0.52) and (math.abs(Dist.Y) < FrameMan.PlayerScreenHeight * 0.52) then
						local Trace = SceneMan:ShortestDistance(Origin, Enemy.EyePos, false);
						local ID = SceneMan:CastMORay(Origin, Trace, Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, false, 5);
						if ID ~= rte.NoMOID then
							local MO = MovableMan:GetMOFromID(ID);
							if MO and ID ~= MO.RootID then
								MO = MovableMan:GetMOFromID(MO.RootID);
							end

							return MO, SceneMan:GetLastRayHitPos();
						end
					end
				end
			end
		else
			AI.Enemies = nil;
			return HumanBehaviors.LookForTargets(AI, Owner, Skill); -- cast rays like normal actors occasionally
		end
	end
end

function HumanBehaviors.GetGrenadeAngle(AimPoint, TargetVel, StartPos, muzVel)
	local Dist = SceneMan:ShortestDistance(StartPos, AimPoint, false);

	-- compensate for gravity if the point we are trying to hit is more than 2m away
	if Dist:MagnitudeIsGreaterThan(40) then
		local timeToTarget = Dist.Magnitude / muzVel;

		-- lead the target if target speed and projectile TTT is above the threshold
		if (timeToTarget * TargetVel.Magnitude) > 0.5 then
			AimPoint = AimPoint + TargetVel * timeToTarget;
			Dist = SceneMan:ShortestDistance(StartPos, AimPoint, false);
		end

		Dist = Dist / GetPPM(); -- convert from pixels to meters
		local velSqr = math.pow(muzVel, 2);
		local gravity = SceneMan.GlobalAcc.Y * 0.67; -- underestimate gravity
		local root = math.sqrt(velSqr*velSqr - gravity*(gravity*Dist.X*Dist.X+2*-Dist.Y*velSqr));

		if root ~= root then
			return nil; -- no solution exists if the root is NaN
		end

		return math.atan2(velSqr-root, gravity*Dist.X);
	end

	return Dist.AbsRadAngle;
end

-- deprecated since B30. make sure we equip our preferred device if we have one. return true if we must run this function again to be sure
function HumanBehaviors.EquipPreferredWeapon(AI, Owner)
	if AI.squadShoot == false then
		if AI.PlayerPreferredHD then
			Owner:EquipNamedDevice(AI.PlayerPreferredHD, true);
		elseif not Owner:EquipDeviceInGroup("Weapons - Primary", true) then
			Owner:EquipDeviceInGroup("Weapons - Secondary", true);
		end
		return false;
	end
end

-- deprecated since B30. make sure we equip a primary weapon if we have one. return true if we must run this function again to be sure
function HumanBehaviors.EquipPrimaryWeapon(AI, Owner)
	Owner:EquipDeviceInGroup("Weapons - Primary", true);
	return false;
end

-- deprecated since B30. make sure we equip a secondary weapon if we have one. return true if we must run this function again to be sure
function HumanBehaviors.EquipSecondaryWeapon(AI, Owner)
	Owner:EquipDeviceInGroup("Weapons - Secondary", true);
	return false;
end

-- in sentry behavior the agent only looks for new enemies, it sometimes sharp aims to increase spotting range
function HumanBehaviors.Sentry(AI, Owner, Abort)
	local sweepUp = true;
	local sweepDone = false;
	local maxAng = math.min(1.4, Owner.AimRange);
	local minAng = -maxAng;
	local aim;

	if AI.PlayerPreferredHD then
		Owner:EquipNamedDevice(AI.PlayerPreferredHD, true);
	elseif not Owner:EquipDeviceInGroup("Weapons - Primary", true) then
		Owner:EquipDeviceInGroup("Weapons - Secondary", true);
	end

	if AI.OldTargetPos then	-- try to reacquire an old target
		local Dist = SceneMan:ShortestDistance(Owner.EyePos, AI.OldTargetPos, false);
		AI.OldTargetPos = nil;
		if (Dist.X < 0 and Owner.HFlipped) or (Dist.X > 0 and not Owner.HFlipped) then	-- we are facing the target
			AI.deviceState = AHuman.AIMING;
			AI.Ctrl.AnalogAim = Dist.Normalized;

			for _ = 1, math.random(20, 30) do
				local _ai, _ownr, _abrt = coroutine.yield(); -- aim here for ~0.25s
				if _abrt then return true end
			end
		end
	elseif not AI.isPlayerOwned and Owner.AIMode ~= Actor.AIMODE_GOTO then -- face the most likely enemy approach direction
		for _ = 1, math.random(5) do	-- wait for a while
			local _ai, _ownr, _abrt = coroutine.yield(); -- aim here for ~0.25s
			if _abrt then return true end
		end

		Owner:ClearMovePath();
		Owner:AddAISceneWaypoint(Vector(Owner.Pos.X, 0));
		Owner:UpdateMovePath();

		-- wait until movepath is updated
		while Owner.IsWaitingOnNewMovePath do
			local _ai, _ownr, _abrt = coroutine.yield();
			if _abrt then return true end
		end

		-- face the direction of the first waypoint
		for WptPos in Owner.MovePath do
			local Dist = SceneMan:ShortestDistance(Owner.Pos, WptPos, false);
			if Dist.X > 5 then
				AI.SentryFacing = false;
				AI.Ctrl.AnalogAim = Dist.Normalized;
			elseif Dist.X < -5 then
				AI.SentryFacing = true;
				AI.Ctrl.AnalogAim = Dist.Normalized;
			end

			break;
		end

		Owner:ClearMovePath();
	end

	if not AI.SentryPos then
		AI.SentryPos = Vector(Owner.Pos.X, Owner.Pos.Y);
	end

	while true do	-- start by looking forward
		aim = Owner:GetAimAngle(false);

		if sweepUp then
			if aim < maxAng/3 then
				AI.Ctrl:SetState(Controller.AIM_UP, false);
				local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
				if _abrt then return true end
				AI.Ctrl:SetState(Controller.AIM_UP, true);
			else
				sweepUp = false;
			end
		else
			if aim > minAng/3 then
				AI.Ctrl:SetState(Controller.AIM_DOWN, false);
				local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
				if _abrt then return true end
				AI.Ctrl:SetState(Controller.AIM_DOWN, true);
			else
				sweepUp = true;
				if sweepDone then
					break;
				else
					sweepDone = true;
				end
			end
		end

		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end
	end

	if AI.SentryFacing ~= nil and Owner.HFlipped ~= AI.SentryFacing then
		Owner.HFlipped = AI.SentryFacing; -- turn to the direction we have been order to guard
		return true; -- restart this behavior
	end

	while true do	-- look down
		aim = Owner:GetAimAngle(false);
		if aim > minAng then
			AI.Ctrl:SetState(Controller.AIM_DOWN, true);
		else
			break;
		end

		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end
	end

	local Hit = Vector();
	local NoObstacle = {};
	local StartPos;
	AI.deviceState = AHuman.AIMING;

	while true do	-- scan the area for obstacles
		aim = Owner:GetAimAngle(false);
		if aim < maxAng then
			AI.Ctrl:SetState(Controller.AIM_UP, true);
		else
			break;
		end

		if Owner:EquipFirearm(false) and Owner.EquippedItem then
			StartPos = ToHeldDevice(Owner.EquippedItem).MuzzlePos;
		else
			StartPos = Owner.EyePos;
		end

		-- save the angle to a table if there is no obstacle
		if not SceneMan:CastStrengthRay(StartPos, Vector(60, 0):RadRotate(Owner:GetAimAngle(true)), 5, Hit, 2, 0, true) then
			table.insert(NoObstacle, aim); -- TODO: don't use a table for this
		end

		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end
	end

	local SharpTimer = Timer();
	local IdleAimTimer = Timer();
	local sharpAimTime = AI.idleAimTime;
	local angDiff = 1;
	AI.deviceState = AHuman.POINTING;

	if #NoObstacle > 1 then	-- only aim where we know there are no obstacles, e.g. out of a gun port
		minAng = NoObstacle[1] * 0.95;
		maxAng = NoObstacle[#NoObstacle] * 0.95;
		angDiff = 1 / math.max(math.abs(maxAng - minAng), 0.1); -- sharp aim longer from a small aiming window
	end

	while true do
		if not Owner:EquipFirearm(false) and not Owner:EquipThrowable(false) then
			break;
		end

		aim = Owner:GetAimAngle(false);

		if IdleAimTimer:IsPastSimMS(AI.idleAimTime) then
			if sweepUp then
				if aim < maxAng then
					if aim < maxAng/5 and aim > minAng/5 and PosRand() > 0.3 then
						AI.Ctrl:SetState(Controller.AIM_UP, false);
					else
						AI.Ctrl:SetState(Controller.AIM_UP, true);
					end
				else
					sweepUp = false;
					IdleAimTimer:Reset();
				end
			else
				if aim > minAng then
					if aim < maxAng/5 and aim > minAng/5 and PosRand() > 0.3 then
						AI.Ctrl:SetState(Controller.AIM_DOWN, false);
					else
						AI.Ctrl:SetState(Controller.AIM_DOWN, true);
					end
				else
					sweepUp = true;
					IdleAimTimer:Reset();
				end
			end
		end

		if SharpTimer:IsPastSimMS(AI.idleAimTime) then
			SharpTimer:Reset();

			-- make sure that we have any preferred weapon equipped
			if AI.PlayerPreferredHD then
				Owner:EquipNamedDevice(AI.PlayerPreferredHD, true);
			elseif not Owner:EquipDeviceInGroup("Weapons - Primary", true) then
				Owner:EquipDeviceInGroup("Weapons - Secondary", true);
			end

			if AI.deviceState == AHuman.AIMING then
				sharpAimTime = RangeRand(AI.idleAimTime * 0.5, AI.idleAimTime * 1.5);
				AI.deviceState = AHuman.POINTING;
			else
				sharpAimTime = RangeRand(AI.idleAimTime * 3, AI.idleAimTime * 6) * angDiff;
				AI.deviceState = AHuman.AIMING;
			end
			if Owner.AIMode ~= Actor.AIMODE_SQUAD then
				if SceneMan:ShortestDistance(Owner.Pos, AI.SentryPos, false):MagnitudeIsGreaterThan(Owner.Height*0.7) then
					AI.SentryPos = SceneMan:MovePointToGround(AI.SentryPos, Owner.Height*0.25, 3);
					Owner:ClearAIWaypoints();
					Owner:AddAISceneWaypoint(AI.SentryPos);
					AI:CreateGoToBehavior(Owner); -- try to return to the sentry pos
					break;
				elseif AI.SentryFacing and Owner.HFlipped ~= AI.SentryFacing then
					Owner.HFlipped = AI.SentryFacing; -- turn to the direction we have been order to guard
					break; -- restart this behavior
				elseif AI.TargetLostTimer:IsPastSimTimeLimit() and math.random() < Owner.Perceptiveness then
					-- turn around occasionally if there is open space behind our back
					local backAreaRay = Vector(-math.random(FrameMan.PlayerScreenWidth/4, FrameMan.PlayerScreenWidth/2) * Owner.FlipFactor, 0):DegRotate(math.random(-25, 25) * Owner.Perceptiveness);
					if not SceneMan:CastStrengthRay(Owner.EyePos, backAreaRay, 10, Vector(), 10, rte.grassID, SceneMan.SceneWrapsX) then
						Owner.HFlipped = Owner.FlipFactor == 1 and true or false;
					end
				end
			end
		end

		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end
	end

	return true;
end

function HumanBehaviors.GoldDig(AI, Owner, Abort)
	-- make sure our weapon have ammo before we start to dig, just in case we encounter an enemy while digging
	if Owner.EquippedItem and (Owner.FirearmNeedsReload or Owner.FirearmIsEmpty) and Owner.EquippedItem:HasObjectInGroup("Weapons") then
		Owner:ReloadFirearms();

		repeat
			local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
			if _abrt then return true end
		until not Owner.FirearmIsEmpty
	end

	-- select a digger
	if not Owner:EquipDiggingTool(true) then
		return true; -- our digger is gone, abort this behavior
	end

	local aimAngle = 0.45;
	local BestGoldLocation = {X = 0, Y = 0};
	local smallestPenalty = math.huge;

	for aimAngle = 0.4, -3.54, -0.033 do
		local Digger;
		if Owner.EquippedItem then
			Digger = ToHeldDevice(Owner.EquippedItem);
			if not Digger then
				break;
			end
		else
			break;
		end

		local LookVec;
		if aimAngle < -0.8 and aimAngle > -2.4 then
			LookVec = Vector(60,0):RadRotate(aimAngle);
		else	-- search further away horizontally
			LookVec = Vector(180,0):RadRotate(aimAngle);
		end

		AI.Ctrl.AnalogAim = LookVec.Normalized;
		local GoldPos = Vector();
		if SceneMan:CastMaterialRay(Digger.MuzzlePos, LookVec, rte.goldID, GoldPos, 1, true) then
			-- avoid gold close to the edges of the scene
			if GoldPos.Y < SceneMan.SceneHeight - 25 and (SceneMan.SceneWrapsX or (GoldPos.X > 50 and GoldPos.X < SceneMan.SceneWidth - 50)) then
				local Dist = SceneMan:ShortestDistance(Owner.Pos, GoldPos, false); -- prioritize gold close to us
				local str = SceneMan:CastStrengthSumRay(Owner.EyePos, GoldPos, 3, rte.goldID) / 30; -- prioritize gold in soft ground
				local penalty = str + Dist.Magnitude + math.abs(Dist.Y*5);
				local DigArea = SceneMan:ShortestDistance(GoldPos, Owner.EyePos+LookVec, false);
				local digLength = math.min(DigArea.Magnitude, 180); -- sanity check to circumvent infinite loops

				-- prioritize gold located horizontally or below us
				if Dist.Y > -20 then
					penalty = penalty - 5;
				end

				local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
				if _abrt then return true end

				-- prioritize areas with more gold
				DigArea:Normalize();
				for i = 5, digLength, 5 do
					local Step = GoldPos + DigArea * i;
					if Step.X >= SceneMan.SceneWidth then
						if SceneMan.SceneWrapsX then
							Step.X = Step.X - SceneMan.SceneWidth;
						else
							break;
						end
					elseif Step.X < 0 then
						if SceneMan.SceneWrapsX then
							Step.X = SceneMan.SceneWidth - Step.X;
						else
							break;
						end
					end

					if Step.Y > SceneMan.SceneHeight - 50 then
						break;
					end

					if SceneMan:GetTerrMatter(Step.X, Step.Y) == rte.goldID then
						penalty = penalty - 4;
					end
				end

				-- prioritize gold located horizontally relative to us
				if math.abs(Dist.X) > math.abs(Dist.Y) then
					if math.abs(Dist.X) * 0.5 > math.abs(Dist.Y) then
						penalty = penalty - 80;
					else
						penalty = penalty - 40;
					end
				end

				if penalty < smallestPenalty then
					if Dist:MagnitudeIsLessThan(50) then	-- dig to a point behind the gold
						GoldPos = Owner.Pos + Dist:SetMagnitude(55);
					end

					-- make sure there is no metal in our path
					if not SceneMan:CastStrengthRay(Owner.Pos, Dist:SetMagnitude(60), 95, Vector(), 2, rte.grassID, SceneMan.SceneWrapsX) then
						smallestPenalty = penalty + RangeRand(-7, 7);
						BestGoldLocation.X, BestGoldLocation.Y = GoldPos.X, GoldPos.Y;
					end
				end
			end
		end

		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end
	end

	local BestGoldPos = Vector(BestGoldLocation.X, BestGoldLocation.Y);
	if BestGoldPos.Largest == 0 then
		if Owner.Pos.Y < SceneMan.SceneHeight - 50 then	-- don't dig beyond the scene limit
			-- no gold found, so dig down and try again
			local rayLenghtY = math.min(80, SceneMan.SceneHeight-100);
			local rayLenghtX = rayLenghtY * 0.5;
			local Target = Owner.Pos + Vector(rayLenghtX, rayLenghtY);
			local str_r = SceneMan:CastStrengthSumRay(Owner.Pos, Target, 6, rte.goldID);
			local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
			if _abrt then return true end

			Target = Owner.Pos + Vector(-rayLenghtX, rayLenghtY);
			local str_l = SceneMan:CastStrengthSumRay(Owner.Pos, Target, 6, rte.goldID);
			local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
			if _abrt then return true end

			if str_r < str_l then
				BestGoldPos = Owner.Pos + Vector(rayLenghtX, rayLenghtY);
			else
				BestGoldPos = Owner.Pos + Vector(-rayLenghtX, rayLenghtY);
			end
		else
			-- no gold here, and we cannot dig deeper, calculate average horizontal strength
			local rayLenght = 80;
			local Target = Owner.Pos + Vector(rayLenght, -5);
			local Trace = SceneMan:ShortestDistance(Owner.Pos, Target, false);
			local str_r = SceneMan:CastStrengthSumRay(Owner.Pos, Target, 5, rte.goldID);
			local obst_r = SceneMan:CastStrengthRay(Owner.Pos, Trace, 95, Vector(), 2, rte.grassID, SceneMan.SceneWrapsX);
			local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
			if _abrt then return true end

			Target = Owner.Pos + Vector(-rayLenght, -5);
			Trace = SceneMan:ShortestDistance(Owner.Pos, Target, false);
			local str_l = SceneMan:CastStrengthSumRay(Owner.Pos, Target, 5, rte.goldID);
			local obst_l = SceneMan:CastStrengthRay(Owner.Pos, Trace, 95, Vector(), 2, rte.grassID, SceneMan.SceneWrapsX);
			local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
			if _abrt then return true end

			local goLeft;
			if obst_l then
				goLeft = false;
			elseif obst_r then
				goLeft = true;
			else
				goLeft = math.random() > 0.5;

				-- go towards the larger obstacle, unless metal
				if math.abs(str_l - str_r) > 200 then
					if str_r > str_l and not obst_r then
						goLeft = false;
					elseif str_r < str_l and not obst_l then
						goLeft = true;
					end
				end
			end

			if goLeft then
				BestGoldPos = Owner.Pos + Vector(-rayLenght, -5);
			else
				BestGoldPos = Owner.Pos + Vector(rayLenght, -5);
			end
		end
	end

	BestGoldPos.Y = math.min(BestGoldPos.Y, SceneMan.SceneHeight-30);
	Owner:ClearAIWaypoints();
	Owner:AddAISceneWaypoint(BestGoldPos);
	AI:CreateGoToBehavior(Owner);

	return true;
end

-- find a weapon to pick up
function HumanBehaviors.WeaponSearch(AI, Owner, Abort)
	local pickupDiggers = not Owner:HasObjectInGroup("Tools - Diggers");

	local maxSearchDistance;
	if AI.isPlayerOwned then
		maxSearchDistance = 100; -- don't move player actors too far
	else
		maxSearchDistance = FrameMan.PlayerScreenWidth * 0.45;
	end

	if Owner.AIMode == Actor.AIMODE_SENTRY then
		maxSearchDistance = maxSearchDistance * 0.6;
	end
	
	local devices = {};
	local mosSearched = 0;
	for movableObject in MovableMan:GetMOsInRadius(Owner.Pos, maxSearchDistance, -1, true) do
		mosSearched = mosSearched + 1;
		if mosSearched % 30 == 0 then
			local _ai, _ownr, _abrt = coroutine.yield();
			if _abrt then return true end
		end
		
		if MovableMan:ValidMO(movableObject) and IsHeldDevice(movableObject) then
			local device = ToHeldDevice(movableObject);
			if device:IsPickupableBy(Owner) and not device:IsActivated() and device.Vel.Largest < 3 and not SceneMan:IsUnseen(device.Pos.X, device.Pos.Y, Owner.Team) then
				local distanceToDevice = SceneMan:ShortestDistance(Owner.Pos, device.Pos, SceneMan.SceneWrapsX or SceneMan.SceneWrapsY);
				if distanceToDevice:MagnitudeIsGreaterThan(Owner.Radius * 0.75) then
					table.insert(devices, { device = device, distance = distanceToDevice });
				else
					devices = {{ device = device, distance = distanceToDevice }};
					break;
				end
			end
		end
	end
	table.sort(devices, function(device, otherDevice) return device.distance.SqrMagnitude < otherDevice.distance.SqrMagnitude end);
	
	if #devices > 0 then
		local maxPathLength = 36; --TODO when this gets turned into a part of pathing calc, use it that way instead
		if AI.isPlayerOwned then
			maxPathLength = 10;
		end
		
		local searchesRemaining = #devices;
		local devicesToPickUp = {};
		for _, deviceEntry in pairs(devices) do
			local device = deviceEntry.device;
			if MovableMan:ValidMO(device) then
				local pathMultipler = 1;
				if device:HasObjectInGroup("Weapons - Primary") or device:HasObjectInGroup("Weapons - Heavy") then
					pathMultipler = 0.4; -- prioritize primary or heavy weapons
				elseif device.ClassName == "TDExplosive" then
					pathMultipler = 1.4; -- avoid grenades if there are other weapons
				elseif device:IsTool() then
					if pickupDiggers and device:HasObjectInGroup("Tools - Diggers") then
						pathMultipler = 1.8; -- avoid diggers if there are other weapons
					else
						pathMultipler = -1; -- disregard non-digger tools
					end
				end

				if pathMultipler ~= -1 then
					local deviceID = device.UniqueID;
					SceneMan.Scene:CalculatePathAsync(
						function(pathRequest)
							local pathLength = pathRequest.PathLength;
							if pathRequest.Status ~= PathRequest.NoSolution and pathLength < maxPathLength then
								local score = pathLength * pathMultipler;
								table.insert(devicesToPickUp, {deviceId = deviceID, score = score});
							end
							searchesRemaining = searchesRemaining - 1;
						end, 
						Owner.Pos, device.Pos, Owner.JumpHeight, Owner.DigStrength, Owner.Team
					);
				end
			end
		end
		
		while searchesRemaining > 0 do
			local _ai, _ownr, _abrt = coroutine.yield();
			if _abrt then return true end
		end
		
		AI.PickupHD = nil;
		table.sort(devicesToPickUp, function(A,B) return A.score < B.score end);
		for _, deviceToPickupEntry in ipairs(devicesToPickUp) do
			local device = MovableMan:FindObjectByUniqueID(deviceToPickupEntry.deviceId);
			if MovableMan:ValidMO(device) and device:IsDevice() then
				AI.PickupHD = device;
				break;
			end
		end

		if AI.PickupHD then
			-- where do we move after pick up?
			local prevMoveTarget, prevSceneWaypoint;
			if Owner.MOMoveTarget and MovableMan:ValidMO(Owner.MOMoveTarget) then
				prevMoveTarget = Owner.MOMoveTarget;
			else
				prevSceneWaypoint = SceneMan:MovePointToGround(Owner:GetLastAIWaypoint(), Owner.Height/5, 4); -- last wpt or current pos
			end

			Owner:ClearMovePath();
			Owner:AddAIMOWaypoint(AI.PickupHD);

			if prevMoveTarget then
				Owner:AddAIMOWaypoint(prevMoveTarget);
			elseif prevSceneWaypoint then
				Owner:AddAISceneWaypoint(prevSceneWaypoint);
			end

			if Owner.AIMode == Actor.AIMODE_SENTRY then
				AI.SentryFacing = Owner.HFlipped;
			end

			Owner:UpdateMovePath();

			-- wait until movepath is updated
			while Owner.IsWaitingOnNewMovePath do
				local _ai, _ownr, _abrt = coroutine.yield();
				if _abrt then return true end
			end

			AI:CreateGoToBehavior(Owner);
		end
	end

	return true;
end


-- find a tool to pick up
function HumanBehaviors.ToolSearch(AI, Owner, Abort)
	local maxSearchDistance;
	if Owner.AIMode == Actor.AIMODE_GOLDDIG then
		maxSearchDistance = FrameMan.PlayerScreenWidth * 0.5; -- move up to half a screen when digging
	elseif AI.isPlayerOwned then
		maxSearchDistance = 60; -- don't move player actors too far
	else
		maxSearchDistance = FrameMan.PlayerScreenWidth * 0.3;
	end

	if Owner.AIMode == Actor.AIMODE_SENTRY then
		maxSearchDistance = maxSearchDistance * 0.6;
	end
		
	local devices = {};
	local mosSearched = 0;
	for movableObject in MovableMan:GetMOsInRadius(Owner.Pos, maxSearchDistance, -1, true) do
		mosSearched = mosSearched + 1;
		
		if MovableMan:ValidMO(movableObject) and IsHeldDevice(movableObject) then
			local device = ToHeldDevice(movableObject);
			if device:IsPickupableBy(Owner) and not device:IsActivated() and device.Vel.Largest < 3 and not SceneMan:IsUnseen(device.Pos.X, device.Pos.Y, Owner.Team) and device:HasObjectInGroup("Tools - Diggers") then
				local distanceToDevice = SceneMan:ShortestDistance(Owner.Pos, device.Pos, SceneMan.SceneWrapsX or SceneMan.SceneWrapsY);
				if distanceToDevice:MagnitudeIsGreaterThan(20) then
					table.insert(devices, { device = device, distance = distanceToDevice });
				else
					devices = {{ device = device, distance = distanceToDevice }};
					break;
				end
			end
		end
	end
	table.sort(devices, function(device, otherDevice) return device.distance.SqrMagnitude < otherDevice.distance.SqrMagnitude end);
	
	if #devices > 0 then
		local maxPathLength = 16;
		if Owner.AIMode == Actor.AIMODE_GOLDDIG then
			maxPathLength = 30;
		elseif AI.isPlayerOwned then
			maxPathLength = 5;
		end
		
		local searchesRemaining = #devices;
		local devicesToPickUp = {};
		for _, deviceEntry in pairs(devices) do
			local device = deviceEntry.device;
			if MovableMan:ValidMO(device) then
				local deviceId = device.UniqueID;
				SceneMan.Scene:CalculatePathAsync(
					function(pathRequest)
						local pathLength = pathRequest.PathLength;
						if pathRequest.Status ~= PathRequest.NoSolution and pathLength < maxPathLength then
							table.insert(devicesToPickUp, {deviceId = deviceId, score = pathLength});
						end
						searchesRemaining = searchesRemaining - 1;
					end, 
					Owner.Pos, device.Pos, Owner.JumpHeight, Owner.DigStrength, Owner.Team
				);
			end
		end
		
		while searchesRemaining > 0 do
			local _ai, _ownr, _abrt = coroutine.yield();
			if _abrt then return true end
		end

		AI.PickupHD = nil;
		table.sort(devicesToPickUp, function(A,B) return A.score < B.score end); -- sort the items in order of discounted distance
		for _, deviceToPickupEntry in ipairs(devicesToPickUp) do
			local device = MovableMan:FindObjectByUniqueID(deviceToPickupEntry.deviceId);
			if MovableMan:ValidMO(device) and device:IsDevice() then
				AI.PickupHD = device;
				break;
			end
		end

		if AI.PickupHD then
			-- where do we move after pick up?
			local prevMoveTarget, prevSceneWaypoint;
			if Owner.MOMoveTarget and MovableMan:ValidMO(Owner.MOMoveTarget) then
				prevMoveTarget = Owner.MOMoveTarget;
			else
				prevSceneWaypoint = SceneMan:MovePointToGround(Owner:GetLastAIWaypoint(), Owner.Height/5, 4); -- last wpt or current pos
			end

			Owner:ClearMovePath();
			Owner:AddAIMOWaypoint(AI.PickupHD);

			if Owner.AIMode ~= Actor.AIMODE_GOLDDIG then
				if prevMoveTarget then
					Owner:AddAIMOWaypoint(prevMoveTarget);
				elseif prevSceneWaypoint then
					Owner:AddAISceneWaypoint(prevSceneWaypoint);
				end

				if Owner.AIMode == Actor.AIMODE_SENTRY then
					AI.SentryFacing = Owner.HFlipped;
				end
			end

			Owner:UpdateMovePath();

			-- wait until movepath is updated
			while Owner.IsWaitingOnNewMovePath do
				local _ai, _ownr, _abrt = coroutine.yield();
				if _abrt then return true end
			end

			AI:CreateGoToBehavior(Owner);
		end
	end

	return true;
end

-- go prone if we can shoot from the prone position and return the result
function HumanBehaviors.GoProne(AI, Owner, TargetPos, targetID)
	if (not Owner.Head or AI.proneState == AHuman.PRONE) or (Owner:NumberValueExists("AIDisableProne")) then
		return false;
	end

	-- only go prone if we can see the ground near the target
	local AimPoint = SceneMan:MovePointToGround(TargetPos, 10, 3);
	local ground = Owner.Pos.Y + Owner.Height * 0.25;
	local Dist = SceneMan:ShortestDistance(Owner.Pos, AimPoint, false);
	local PronePos;

	-- check if there is room to go prone here
	if Dist.X > Owner.Height then
		-- to the right
		PronePos = Owner.EyePos + Vector(Owner.Height*0.3, 0);

		local x_pos = Owner.Pos.X + 10;
		for _ = 1, math.ceil(Owner.Height/16) do
			x_pos = x_pos + 7;
			if SceneMan.SceneWrapsX and x_pos >= SceneMan.SceneWidth then
				x_pos = SceneMan.SceneWidth - x_pos;
			end

			if 0 == SceneMan:GetTerrMatter(x_pos, ground) then
				return false;
			end
		end
	elseif Dist.X < -Owner.Height then
		-- to the left
		PronePos = Owner.EyePos + Vector(-Owner.Height*0.3, 0);

		local x_pos = Owner.Pos.X - 10;
		for _ = 1, math.ceil(Owner.Height/16) do
			x_pos = x_pos - 7;
			if SceneMan.SceneWrapsX and x_pos < 0 then
				x_pos = x_pos + SceneMan.SceneWidth;
			end

			if 0 == SceneMan:GetTerrMatter(x_pos, ground) then
				return false;
			end
		end
	else
		return false; -- target is too close
	end

	PronePos = SceneMan:MovePointToGround(PronePos, Owner.Head.Radius+3, 2);
	Dist = SceneMan:ShortestDistance(PronePos, AimPoint, false);

	-- check LOS from the prone position
	--if not SceneMan:CastFindMORay(PronePos, Dist, targetID, Hit, rte.grassID, false, 8) then
	if SceneMan:CastObstacleRay(PronePos, Dist, Vector(), Vector(), targetID, Owner.IgnoresWhichTeam, rte.grassID, 9) > -1 then
		return false;
	else
		-- check for obstacles more more carefully
		Dist:CapMagnitude(60);
		if SceneMan:CastObstacleRay(PronePos, Dist, Vector(), Vector(), 0, Owner.IgnoresWhichTeam, rte.grassID, 1) > -1 then
			return false;
		end
	end

	AI.proneState = AHuman.PRONE;
	if not Owner.EquippedBGItem then
		if Dist.X > 0 then
			AI.lateralMoveState = Actor.LAT_RIGHT;
		else
			AI.lateralMoveState = Actor.LAT_LEFT;
		end
	end

	return true;
end

-- open fire on the selected target
function HumanBehaviors.ShootTarget(AI, Owner, Abort)
	if not MovableMan:ValidMO(AI.Target) then
		return true;
	end

	AI.canHitTarget = false;
	AI.TargetLostTimer:SetSimTimeLimitMS(1000);

	local LOSTimer = Timer();
	LOSTimer:SetSimTimeLimitMS(170);

	local ImproveAimTimer = Timer();
	local ShootTimer = Timer();
	local shootDelay = RangeRand(440, 590) * AI.aimSpeed + 150;
	local AimPoint = AI.Target.Pos + AI.TargetOffset;
	if not AI.flying and AI.Target.Vel.Largest < 4 and HumanBehaviors.GoProne(AI, Owner, AimPoint, AI.Target.ID) then
		shootDelay = shootDelay + 250;
	end

	-- spin up asap
	if Owner.FirearmActivationDelay > 0 then
		shootDelay = math.max(50*AI.aimSpeed, shootDelay-Owner.FirearmActivationDelay);
	end

	local PrjDat;
	local openFire = 0;
	local checkAim = true;
	local TargetAvgVel = Vector(AI.Target.Vel.X, AI.Target.Vel.Y);
	local Dist = SceneMan:ShortestDistance(Owner.Pos, AimPoint, false);

	-- make sure we are facing the right direction
	if Owner.HFlipped then
		if Dist.X > 0 then
			Owner.HFlipped = false;
		end
	elseif Dist.X < 0 then
		Owner.HFlipped = true;
	end

	local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
	if _abrt then return true end

	local distMultiplier = AI.aimSkill * math.max(math.min(0.0035*Dist.Largest, 1.0), 0.01);
	local ErrorOffset = Vector(RangeRand(40, 80)*distMultiplier, 0):RadRotate(RangeRand(1, 6));
	local aimTarget = SceneMan:ShortestDistance(Owner.Pos, AimPoint+ErrorOffset, false).AbsRadAngle;
	local f1, f2 = 0.5, 0.5; -- aim noise filter

	while true do
		if not AI.Target or AI.Target:IsDead() then
			AI.Target = nil;

			-- the target is gone, try to find another right away
			local ClosestEnemy = MovableMan:GetClosestEnemyActor(Owner.Team, AimPoint, 200, Vector());
			if ClosestEnemy and not ClosestEnemy:IsDead() then
				if ClosestEnemy.ClassName == "AHuman" then
					ClosestEnemy = ToAHuman(ClosestEnemy);
				elseif ClosestEnemy.ClassName == "ACrab" then
					ClosestEnemy = ToACrab(ClosestEnemy);
				else
					ClosestEnemy = nil;
				end

				if ClosestEnemy then
					-- check if the target is inside our "screen"
					local ViewDist = SceneMan:ShortestDistance(Owner.ViewPoint, ClosestEnemy.Pos, false);
					if (math.abs(ViewDist.X) - ClosestEnemy.Radius < FrameMan.PlayerScreenWidth * 0.5) and (math.abs(ViewDist.Y) - ClosestEnemy.Radius < FrameMan.PlayerScreenHeight * 0.5) then
						if not AI.isPlayerOwned or not SceneMan:IsUnseen(ClosestEnemy.Pos.X, ClosestEnemy.Pos.Y, Owner.Team) then	-- AI-teams ignore the fog
							if SceneMan:CastStrengthSumRay(Owner.EyePos, ClosestEnemy.Pos, 6, rte.grassID) < 120 then
								AI.Target = ClosestEnemy;
								AI.TargetOffset = Vector();
								AimPoint = AimPoint * 0.3 + AI.Target.Pos * 0.7;
							end
						end
					end
				end
			end

			-- no new target found
			if not AI.Target then
				break;
			end
		end

		if Owner.FirearmIsReady then
			-- it is now safe to get the ammo stats since FirearmIsReady
			local Weapon = ToHDFirearm(Owner.EquippedItem);
			if not PrjDat or PrjDat.MagazineName ~= Weapon.Magazine.PresetName then
				PrjDat = SharedBehaviors.GetProjectileData(Owner);

				-- uncomment these to get the range of the weapon
				--ConsoleMan:PrintString(Weapon.PresetName .. " range = " .. PrjDat.rng .. " px");
				--ConsoleMan:PrintString(AI.Target.PresetName .. " range = " .. SceneMan:ShortestDistance(Owner.Pos, AI.Target.Pos, false).Magnitude .. " px");

				-- Aim longer with low capacity weapons
				if ((Weapon.Magazine.Capacity > -1 and Weapon.Magazine.Capacity < 10) or Weapon:HasObjectInGroup("Weapons - Sniper")) and Dist.Largest > 100 then
					-- reduce ErrorOffset and increase shootDelay when the target is further away
					local mag = Dist.Magnitude;
					ErrorOffset = ErrorOffset * Clamp(-0.001*mag+1, 1.0, 0.8);
					shootDelay = shootDelay + Clamp(2*mag-200, 600, 50);
				end
			else
				AimPoint = AI.Target.Pos + AI.TargetOffset + ErrorOffset;

				-- TODO: make low skill AI lead worse
				TargetAvgVel = TargetAvgVel * 0.8 + AI.Target.Vel * 0.2; -- smooth the target's velocity
				Dist = SceneMan:ShortestDistance(Weapon.Pos, AimPoint, false);
				local range = Dist.Magnitude;
				if range < 100 then
					-- move the aim-point towards the center of the target at close ranges
					if range < 50 then
						AI.TargetOffset = AI.TargetOffset * 0.95;
						TargetAvgVel:Reset(); -- high velocity at close range confuse the AI tracking
					else
						TargetAvgVel = TargetAvgVel * 0.5;
					end
				end

				if checkAim then
					checkAim = false; -- only check every second frame

					if range < PrjDat.blast then
						-- it is not safe to fire an explosive projectile at this distance
						aimTarget = Dist.AbsRadAngle;
						AI.canHitTarget = true;
						if Owner.InventorySize > 0 then	-- we have more things in the inventory
							if range < 60 and (Owner:HasObjectInGroup("Tools - Diggers") or Owner:HasObjectInGroup("Weapons - Melee")) then
								AI:CreateHtHBehavior(Owner);
								break;
							elseif Owner:EquipLoadedFirearmInGroup("Any", "Weapons - Explosive", true) then
								PrjDat = nil
								local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
								if _abrt then return true end
								if not Owner.FirearmIsReady then
									break;
								end
							else
								PrjDat.blast = 0; -- no other loaded weapon, ignore the blast radius
							end
						else
							PrjDat.blast = 0; -- no other weapon, ignore the blast radius
						end
					elseif range < PrjDat.rng then
						-- lead the target if target speed and projectile TTT is above the threshold
						local timeToTarget = range / PrjDat.vel;
						if (timeToTarget * TargetAvgVel.Magnitude) > 2 then
							-- ~double this value since we only do this every second update
							if PosRand() > 0.5 then
								timeToTarget = timeToTarget * (2 - RangeRand(0, 0.4) * AI.aimSkill);
							else
								timeToTarget = timeToTarget * (2 + RangeRand(0, 0.4) * AI.aimSkill);
							end

							Dist = SceneMan:ShortestDistance(Weapon.Pos, AimPoint+(Owner.Vel*0.5+TargetAvgVel)*timeToTarget, false);
						end

						aimTarget = HumanBehaviors.GetAngleToHit(PrjDat, Dist);
						if aimTarget then
							AI.canHitTarget = true;
						else
							AI.canHitTarget = false;

							-- the target is too far away
							if not AI.isPlayerOwned or Owner.AIMode ~= Actor.AIMODE_SENTRY then
								if not Owner.MOMoveTarget or not MovableMan:ValidMO(Owner.MOMoveTarget) or Owner.MOMoveTarget.RootID ~= AI.Target.RootID then	-- move towards the target
									local OldWaypoint = SceneMan:MovePointToGround(Owner:GetLastAIWaypoint(), Owner.Height/5, 4); -- move back here later
									Owner:ClearAIWaypoints();
									Owner:AddAIMOWaypoint(AI.Target);
									Owner:AddAISceneWaypoint(OldWaypoint);
									AI:CreateGoToBehavior(Owner);
									AI.proneState = AHuman.NOTPRONE;
								end
							else
								-- TODO: switch weapon properly
								if Weapon:HasObjectInGroup("Weapons - Primary") then
									if Owner:EquipDeviceInGroup("Weapons - Secondary", true) then
										local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
										if _abrt then return true end
										PrjDat = nil;
										if not Owner.FirearmIsReady then
											break;
										end
									end
								elseif Weapon:HasObjectInGroup("Weapons - Secondary") then
									if Owner:EquipDeviceInGroup("Weapons - Primary", true) then
										local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
										if _abrt then return true end
										PrjDat = nil;
										if not Owner.FirearmIsReady then
											break;
										end
									end
								end
							end
						end
					elseif not AI.isPlayerOwned or not (Owner.AIMode == Actor.AIMODE_SENTRY or Owner.AIMode == Actor.AIMODE_SQUAD) then -- target out of reach; move towards it
						-- check if we are already moving towards an actor
						if not Owner.MOMoveTarget or not MovableMan:ValidMO(Owner.MOMoveTarget) or Owner.MOMoveTarget.RootID ~= AI.Target.RootID then	-- move towards the target
							local OldWaypoint = SceneMan:MovePointToGround(Owner:GetLastAIWaypoint(), Owner.Height/5, 4); -- move back here later
							Owner:ClearAIWaypoints();
							Owner:AddAIMOWaypoint(AI.Target);
							Owner:AddAISceneWaypoint(OldWaypoint);
							AI:CreateGoToBehavior(Owner);
							AI.proneState = AHuman.NOTPRONE;
							AI.canHitTarget = false;
						end
					end
				else
					checkAim = true;

					-- periodically check that we have LOS to the target
					if LOSTimer:IsPastSimTimeLimit() then
						LOSTimer:Reset();
						AI.TargetLostTimer:SetSimTimeLimitMS(700);
						local TargetPoint = AI.Target.Pos + AI.TargetOffset;

						if (range < Owner.AimDistance + Weapon.SharpLength + FrameMan.PlayerScreenWidth*0.5) and
							(not AI.isPlayerOwned or not SceneMan:IsUnseen(TargetPoint.X, TargetPoint.Y, Owner.Team))
						then
							if PrjDat.pen > 0 then
								if SceneMan:CastStrengthSumRay(Weapon.Pos, TargetPoint, 6, rte.grassID) * 5 < PrjDat.pen then
									AI.TargetLostTimer:Reset(); -- we can shoot at the target
									AI.OldTargetPos = Vector(AI.Target.Pos.X, AI.Target.Pos.Y);
								end
							else
								if SceneMan:CastStrengthSumRay(Weapon.Pos, TargetPoint, 6, rte.grassID) < 120 then
									AI.TargetLostTimer:Reset(); -- we can shoot at the target
									AI.OldTargetPos = Vector(AI.Target.Pos.X, AI.Target.Pos.Y);
								end
							end
						end
					end
				end

				if AI.canHitTarget then
					if not Owner.aggressive then
						AI.lateralMoveState = Actor.LAT_STILL;
					end
					if not AI.flying then
						AI.deviceState = AHuman.AIMING;
					end
				end

				-- add some filtered noise to the aim
				local aim = Owner:GetAimAngle(true);
				local sharpLen = SceneMan:ShortestDistance(Owner.Pos, Owner.ViewPoint, false).Magnitude;
				local noise = RangeRand(-30, 30) * AI.aimSpeed * (1 + (150 / sharpLen)) * 0.5;
				f1, f2 = 0.9*f1+noise*0.1, 0.7*f2+noise*0.3;
				noise = f1 + f2 + noise * 0.1;
				aimTarget = (aimTarget or aim) + math.min(math.max(noise/(range+30), -0.12), 0.12);

				if AI.flying then
					aimTarget = aimTarget + RangeRand(-0.05, 0.05);
				end

				local angDiff = aim - aimTarget;
				if angDiff > math.pi then
					angDiff = angDiff - math.pi*2;
				elseif angDiff < -math.pi then
					angDiff = angDiff + math.pi*2;
				end

				local angChange = math.max(math.min(angDiff*(0.1/AI.aimSkill), 0.17/AI.aimSkill), -0.17/AI.aimSkill);
				if (angDiff > 0 and angChange > angDiff) or (angDiff < 0 and angChange < angDiff) then
					angChange = angDiff;
				end
				AI.Ctrl.AnalogAim = Vector(1,0):RadRotate(aim-angChange);

				if PrjDat and ShootTimer:IsPastSimMS(shootDelay) then
					-- reduce the aim point error
					if ImproveAimTimer:IsPastSimMS(50) then
						ImproveAimTimer:Reset();
						ErrorOffset = ErrorOffset * 0.93;
					end

					if AI.canHitTarget and angDiff < 0.7 then
						local overlap = AI.Target.Diameter * math.max(AI.aimSkill, 0.4);
						if Weapon.FullAuto then	-- open fire if our aim overlap the target
							if math.abs(angDiff) < math.tanh((overlap*2)/(range+10)) then
								openFire = 5; -- don't stop shooting just because we lose the target for a few frames
							else
								openFire = openFire - 1;
							end
						elseif not AI.fire then	-- open fire if our aim overlap the target
							if math.abs(angDiff) < math.tanh((overlap*1.25)/(range+10)) then
								openFire = 1;
							else
								openFire = 0;
							end
						else
							openFire = openFire - 1; -- release the trigger if semi auto
						end

						-- check for obstacles if the ammo have a blast radius
						if openFire > 0 and PrjDat.blast > 0 then
							if SceneMan:CastObstacleRay(Weapon.MuzzlePos, Weapon:RotateOffset(Vector(50, 0)), Vector(), Vector(), Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, 2) > -1 then
								-- equip another primary if possible
								if Owner:EquipLoadedFirearmInGroup("Any", "Weapons - Explosive", true) then
									PrjDat = nil;
									openFire = 0;
									local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
									if _abrt then return true end
									if not Owner.FirearmIsReady then
										break;
									end
								else
									PrjDat.blast = 0;
								end
							end
						end
					else
						openFire = openFire - 1;
					end
				else
					-- reduce the aim point error
					if ImproveAimTimer:IsPastSimMS(50) then
						ImproveAimTimer:Reset();
						ErrorOffset = ErrorOffset * 0.97;
					end

					openFire = 0;
				end

				if openFire > 0 then
					AI.fire = true;
				else
					AI.fire = false;
				end
			end
		else
			if Owner.EquippedItem and ToHeldDevice(Owner.EquippedItem):IsReloading() then
				ShootTimer:Reset();
				AI.Ctrl.AnalogAim = SceneMan:ShortestDistance(Owner.Pos, AI.Target.Pos, false).Normalized;
				if AI.lateralMoveState == Actor.LAT_STILL then
					AI.proneState = AHuman.PRONE;
					--AI.Ctrl:SetState(Controller.BODY_CROUCH, true);
				end
			elseif Owner:EquipFirearm(true) then
				local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame, just in case the magazine is replenished by another script
				if _abrt then return true end
				AI.TargetLostTimer:SetSimTimeLimitMS(1400);

				if Owner.FirearmIsEmpty then
					AI.deviceState = AHuman.POINTING;
					AI.fire = false;
					openFire = 0;

					if AI.Target then
						-- equip another primary if possible
						if Owner:EquipLoadedFirearmInGroup("Weapons - Primary", "None", true) then
							PrjDat = nil;
						else
							-- select a secondary instead of reloading if the target is within half a screen
							if Dist.Largest < (FrameMan.PlayerScreenWidth * 0.5 + AI.Target.Radius + Owner.AimDistance) then
								-- select a primary if we have an empty secondary equipped
								if Owner:EquipLoadedFirearmInGroup("Weapons - Secondary", "None", true) then
									PrjDat = nil;
								elseif Owner:EquipDeviceInGroup("Weapons - Primary", true) then
									PrjDat = nil;
								end
							end
						end

						local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
						if _abrt then return true end
					end

					-- we might have a different weapon equipped now check if FirearmIsEmpty again
					if Owner.FirearmIsEmpty then
						-- TODO: check if ducking is appropriate while reloading (when we can make the actor stand up reliably)
						Owner:ReloadFirearms();

						-- increase the TargetLostTimer limit so we don't end this behavior before the reload is finished
						if Owner.EquippedItem and IsHDFirearm(Owner.EquippedItem) then
							AI.TargetLostTimer:SetSimTimeLimitMS(ToHDFirearm(Owner.EquippedItem).ReloadTime+500);
						end
					end

					distMultiplier = AI.aimSkill * math.max(math.min(0.0035*Dist.Largest, 1.0), 0.01);
					ErrorOffset = Vector(RangeRand(25, 40)*distMultiplier, 0):RadRotate(RangeRand(1, 6));
					shootDelay = RangeRand(220, 330) * AI.aimSpeed + 50;
					if Owner.FirearmActivationDelay > 0 then	-- Spin up asap
						shootDelay = math.max(50*AI.aimSpeed, shootDelay-Owner.FirearmActivationDelay);
					end
				end
			else
				AI:CreateGetWeaponBehavior(Owner);
				break; -- no firearm available
			end
		end

		-- make sure we are facing the right direction
		if Owner.HFlipped then
			if Dist.X > 0 then
				Owner.HFlipped = false;
			end
		elseif Dist.X < 0 then
			Owner.HFlipped = true;
		end

		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end
	end

	-- reload the secondary before switching to the primary weapon
	if Owner:HasObjectInGroup("Weapons - Primary") and (Owner.EquippedItem and Owner.EquippedItem:HasObjectInGroup("Weapons - Secondary")) then
		while Owner.EquippedItem and ToHeldDevice(Owner.EquippedItem):IsReloading() do
			local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
			if _abrt then return true end
		end
	end

	if AI.PlayerPreferredHD then
		Owner:EquipNamedDevice(AI.PlayerPreferredHD, true);
	elseif not Owner:EquipDeviceInGroup("Weapons - Primary", true) then
		Owner:EquipDeviceInGroup("Weapons - Secondary", true);
	end

	if Owner.FirearmIsEmpty then
		Owner:ReloadFirearms();
	end

	return true;
end

-- throw a grenade at the selected target
--TODO: This behavior should effectively have the actor close in on the target if out of range!
function HumanBehaviors.ThrowTarget(AI, Owner, Abort)
	local ThrowTimer = Timer();
	local aimTime = Owner.ThrowPrepTime;
	local scan = 0;
	local miss = 0; -- stop scanning after a few missed attempts
	local AimPoint, Dist, MO, ID, rootID, LOS, aim;

	AI.TargetLostTimer:SetSimTimeLimitMS(1500);

	while true do
		if not MovableMan:ValidMO(AI.Target) then
			break;
		end

		if LOS then	-- don't sharp aim until LOS has been confirmed
			if Owner.ThrowableIsReady then
				if not ThrowTimer:IsPastSimMS(aimTime) then
					AI.Ctrl.AnalogAim = Vector(1,0):RadRotate(aim+RangeRand(-0.04, 0.04));
					AI.fire = true;
				else
					AI.fire = false;
				end
			else	-- no grenades left, continue attack
				if not (Owner.AIMode == Actor.AIMODE_SENTRY or Owner.AIMode == Actor.AIMODE_SQUAD) then
					AI:CreateAttackBehavior(Owner);
				end
				break;
			end
		else
			if scan < 1 then
				if AI.Target.Door then
					AimPoint = AI.Target.Door.Pos;
				else
					AimPoint = AI.Target.Pos; -- look for the center
					if AI.Target.EyePos then
						AimPoint = (AimPoint + AI.Target.EyePos) / 2;
					end
				end

				ID = rte.NoMOID;
				--if Owner:IsWithinRange(Vector(AimPoint.X, AimPoint.Y)) then	-- TODO: use grenade properties to decide this
				if true then
					Dist = SceneMan:ShortestDistance(Owner.EyePos, AimPoint, false);
					ID = SceneMan:CastMORay(Owner.EyePos, Dist, Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, false, 3);
					if ID < 1 or ID == rte.NoMOID then	-- not found, look for any head or legs
						AimPoint = AI.Target.EyePos; -- the head
						if AimPoint then
							local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
							if _abrt then return true end
							if not MovableMan:ValidMO(AI.Target) then	-- must verify that the target exist after a yield
								break;
							end

							Dist = SceneMan:ShortestDistance(Owner.EyePos, AimPoint, false);
							ID = SceneMan:CastMORay(Owner.EyePos, Dist, Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, false, 3);
						end

						if ID < 1 or ID == rte.NoMOID then
							local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
							if _abrt then return true end
							if not MovableMan:ValidMO(AI.Target) then	-- must verify that the target exist after a yield
								break;
							end

							local Legs = AI.Target.FGLeg or AI.Target.BGLeg; -- the legs
							if Legs then
								AimPoint = Legs.Pos;
								Dist = SceneMan:ShortestDistance(Owner.EyePos, AimPoint, false);
								ID = SceneMan:CastMORay(Owner.EyePos, Dist, Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, false, 3);
							end
						end
					end
				else
					break; -- out of range
				end

				if ID > 0 and ID ~= rte.NoMOID then	-- MO found
					-- check what target we will hit
					rootID = MovableMan:GetRootMOID(ID);
					if rootID ~= AI.Target.ID then
						MO = MovableMan:GetMOFromID(rootID);
						if MovableMan:ValidMO(MO) then
							if MO.Team ~= Owner.Team then
								if MO.ClassName == "AHuman" then
									AI.Target = ToAHuman(MO);
									local Legs = AI.Target.FGLeg or AI.Target.BGLeg; -- the legs
									if Legs then
										AimPoint = Legs.Pos;
									end
								elseif MO.ClassName == "ACrab" then
									AI.Target = ToACrab(MO);
									local Legs = AI.Target.LeftFGLeg or AI.Target.RightFGLeg or AI.Target.LeftBGLeg or AI.Target.RightFGLeg; -- the legs
									if Legs then
										AimPoint = Legs.Pos;
									end
								elseif MO.ClassName == "ACRocket" then
									AI.Target = ToACRocket(MO);
								elseif MO.ClassName == "ACDropShip" then
									AI.Target = ToACDropShip(MO);
								elseif MO.ClassName == "ADoor" then
									AI.Target = ToADoor(MO);
								elseif MO.ClassName == "Actor" then
									AI.Target = ToActor(MO);
								else
									break;
								end
							else
								break; -- don't shoot friendlies
							end
						end
					end

					scan = 6; -- skip the LOS check the next n frames
					miss = 0;
					LOS = true; -- we have line of sight to the target

					-- first try to reach the target with an the max throw vel
					if Owner.ThrowableIsReady then
						local Grenade = ToThrownDevice(Owner.EquippedItem);
						if Grenade then
							local maxThrowVel = Grenade:GetCalculatedMaxThrowVelIncludingArmThrowStrength();
							local minThrowVel = Grenade.MinThrowVel;
							if minThrowVel == 0 then
								minThrowVel = maxThrowVel * 0.2;
							end
							aim = HumanBehaviors.GetGrenadeAngle(AimPoint, Vector(), Grenade.MuzzlePos, maxThrowVel);
							if aim then
								aim = aim - Owner.RotAngle;
								ThrowTimer:Reset();
								aimTime = Owner.ThrowPrepTime * RangeRand(0.9, 1.1);
								local maxAim = aim;

								-- try again with an average throw vel
								aim = HumanBehaviors.GetGrenadeAngle(AimPoint, Vector(), Grenade.MuzzlePos, (maxThrowVel + minThrowVel) * 0.5);
								if aim then
									aimTime = Owner.ThrowPrepTime * RangeRand(0.45, 0.55);
								else
									aim = maxAim;
								end
							else
								break; -- target out of range
							end
						else
							break;
						end
					else
						break;
					end
				else
					miss = miss + 1;
					if miss > 4 then	-- stop looking if we cannot find anything after n attempts
						break;
					else
						scan = 3; -- check LOS a little bit more often if no MO was found
					end
				end
			else
				scan = scan - 1;
			end
		end

		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end
	end

	return true;
end

-- attack the target in hand-to-hand
function HumanBehaviors.AttackTarget(AI, Owner, Abort)
	if not AI.Target or not MovableMan:ValidMO(AI.Target) then
		return true;
	end

	AI.TargetLostTimer:SetSimTimeLimitMS(5000);

	-- If we've been trying for a while and we're not getting any closer to our target, auto-fail the behaviour so we do other stuff
	local MovementFailTimer = Timer();
	MovementFailTimer:SetSimTimeLimitMS(3000);
	local closestDistance = nil;

	-- If we're attacking the target but it's not dying (likely that we're too far away), fail
	-- It may be that we are successfully damaging it, just very slowly. That's not a big issue
	-- If it's still a good target, we'll likely retarget it next frame, and move a little closer (by re-adding the GoToBehavior)
	local DamageFailTimer = Timer();
	DamageFailTimer:SetSimTimeLimitMS(6000);

	-- move back here later
	local PrevMOMoveTarget, PrevSceneWaypoint;
	if Owner.MOMoveTarget and MovableMan:ValidMO(Owner.MOMoveTarget) then
		PrevMOMoveTarget = Owner.MOMoveTarget;
	else
		Owner.MOMoveTarget = nil;
		PrevSceneWaypoint = SceneMan:MovePointToGround(Owner:GetLastAIWaypoint(), Owner.Height/5, 4);
	end

	-- move towards the target
	Owner:ClearMovePath();
	Owner:AddAIMOWaypoint(AI.Target);

	if PrevMOMoveTarget then
		Owner:AddAIMOWaypoint(PrevMOMoveTarget);
	end

	if PrevSceneWaypoint then
		Owner:AddAISceneWaypoint(PrevSceneWaypoint);
	end

	AI:CreateGoToBehavior(Owner);
	AI.proneState = AHuman.NOTPRONE;

	while true do
		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end

		if not AI.Target or not MovableMan:ValidMO(AI.Target) then
			break;
		end

		-- use following sequence to attack either with a suited melee weapon or arms
		local suitableWeapon = false;

		if AI.Target.ClassName == "ADoor" then
			-- Prefer breaching tools for attacking doors
			suitableWeapon = Owner:EquipDeviceInGroup("Tools - Breaching", true) or Owner:EquipDeviceInGroup("Tools - Diggers", true) or Owner:EquipDeviceInGroup("Weapons - Melee", true);
		else
			-- Prefer melee weapons for attacking actors
			suitableWeapon = Owner:EquipDeviceInGroup("Weapons - Melee", true) or Owner:EquipDeviceInGroup("Tools - Diggers", true) or Owner:EquipDeviceInGroup("Tools - Breaching", true);
		end

		if not suitableWeapon then
			-- We have no suitable weapon to use
			AI:CreateGetWeaponBehavior(Owner);
			break;
		end

		local startPos = Vector(Owner.EquippedItem.Pos.X, Owner.EquippedItem.Pos.Y);
		local attackPos = (AI.Target.ClassName == "ADoor" and ToADoor(AI.Target).Door and ToADoor(AI.Target).Door:IsAttached()) and ToADoor(AI.Target).Door.Pos or AI.Target.Pos;
		local distance = SceneMan:ShortestDistance(startPos, attackPos, SceneMan.SceneWrapsX);
		local meleeDist = Owner.IndividualRadius + (IsThrownDevice(Owner.EquippedItem) and 50 or 25);
		if distance:MagnitudeIsLessThan(meleeDist) then
			if DamageFailTimer:IsPastSimTimeLimit() then
				break;
			end
			AI.lateralMoveState = Actor.LAT_STILL;
			AI.Ctrl.AnalogAim = SceneMan:ShortestDistance(Owner.EyePos, attackPos, SceneMan.SceneWrapsX).Normalized;
			AI.fire = not (AI.fire and IsThrownDevice(Owner.EquippedItem) and Owner.ThrowProgress == 1);
		else
			DamageFailTimer:Reset();

			-- Ensure we're getting closer
			if not closestDistance or distance.SqrMagnitude < closestDistance.SqrMagnitude then
				closestDistance = distance;
				MovementFailTimer:Reset();
			elseif MovementFailTimer:IsPastSimTimeLimit() then
				break;
			end

			AI.fire = false;
		end
	end

	return true;
end


function HumanBehaviors.GetAngleToHit(PrjDat, Dist)
	if PrjDat.g == 0 then	-- this projectile is not affected by gravity
		return Dist.AbsRadAngle;
	else	-- compensate for gravity
		local rootSq, muzVelSq;
		local D = Dist / GetPPM(); -- convert from pixels to meters
		if PrjDat.drg < 1 then	-- compensate for air resistance
			local rng = D.Magnitude;
			local timeToTarget = math.floor((rng / math.max(PrjDat.vel*PrjDat.drg^math.floor(rng/(PrjDat.vel+1)+0.5), PrjDat.thr)) / TimerMan.DeltaTimeSecs); -- estimate time of flight in frames

			if timeToTarget > 1 then
				local muzVel = 0.9*math.max(PrjDat.vel * PrjDat.drg^timeToTarget, PrjDat.thr) + 0.1*PrjDat.vel; -- compensate for velocity reduction during flight
				muzVelSq = muzVel * muzVel;
				rootSq = muzVelSq*muzVelSq - PrjDat.g * (PrjDat.g*D.X*D.X + 2*-D.Y*muzVelSq);
			else
				muzVelSq = PrjDat.vsq;
				rootSq = PrjDat.vqu - PrjDat.g * (PrjDat.g*D.X*D.X + 2*-D.Y*muzVelSq);
			end
		else
			muzVelSq = PrjDat.vsq;
			rootSq = PrjDat.vqu - PrjDat.g * (PrjDat.g*D.X*D.X + 2*-D.Y*muzVelSq);
		end

		if rootSq >= 0 then	-- no solution exists if rootSq is below zero
			local ang1 = math.atan2(muzVelSq - math.sqrt(rootSq), PrjDat.g*D.X);
			local ang2 = math.atan2(muzVelSq + math.sqrt(rootSq), PrjDat.g*D.X);
			if ang1 + ang2 > math.pi then	-- both angles in the second or third quadrant
				if ang1 > math.pi or ang2 > math.pi then	-- one or more angle in the third quadrant
					return math.min(ang1, ang2);
				else
					return math.max(ang1, ang2);
				end
			else	-- both angles in the firs quadrant
				return math.min(ang1, ang2);
			end
		end
	end
end

-- open fire on the area around the selected target
function HumanBehaviors.ShootArea(AI, Owner, Abort)
	if not MovableMan:ValidMO(AI.UnseenTarget) or not Owner.FirearmIsReady then
		return true;
	end

	-- see if we can shoot from the prone position
	local ShootTimer = Timer();
	local aimTime = 50 + RangeRand(100, 300) * AI.aimSpeed;
	if not AI.flying and AI.UnseenTarget.Vel.Largest < 12 and HumanBehaviors.GoProne(AI, Owner, AI.UnseenTarget.Pos, AI.UnseenTarget.ID) then
		aimTime = aimTime + 500;
	end

	local StartPos = Vector(AI.UnseenTarget.Pos.X, AI.UnseenTarget.Pos.Y);

	-- aim at the target in case we can see it when sharp aiming
	Owner:SetAimAngle(SceneMan:ShortestDistance(Owner.EyePos, StartPos, false).AbsRadAngle);
	AI.deviceState = AHuman.AIMING;

	-- aim for ~160ms
	for _ = 1, 10 do
		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end
	end

	if not Owner.FirearmIsReady then
		return true;
	end

	local AimPoint;
	for _ = 1, 5 do	-- try up to five times to find a target area that is reasonably close to the target
		AimPoint = StartPos + Vector(RangeRand(-100, 100), RangeRand(-100, 50));
		if AimPoint.X >= SceneMan.SceneWidth then
			AimPoint.X = SceneMan.SceneWidth - AimPoint.X;
		elseif AimPoint.X < 0 then
			AimPoint.X = AimPoint.X + SceneMan.SceneWidth;
		end

		-- check if we can fire at the AimPoint
		local Trace = SceneMan:ShortestDistance(Owner.EyePos, AimPoint, false);
		local rayLength = SceneMan:CastObstacleRay(Owner.EyePos, Trace, Vector(), Vector(), rte.NoMOID, Owner.IgnoresWhichTeam, rte.grassID, 11);
		if Trace:MagnitudeIsLessThan(rayLength * 1.5) then
			break; -- the AimPoint is close enough to the target, start shooting
		end

		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end
	end

	if not Owner.FirearmIsReady then
		return true;
	end

	local aim;
	local PrjDat = SharedBehaviors.GetProjectileData(Owner);
	local Dist = SceneMan:ShortestDistance(Owner.EquippedItem.Pos, AimPoint, false);
	local Weapon = ToHDFirearm(Owner.EquippedItem);

	-- uncomment these to get the range of the weapon
	--ConsoleMan:PrintString(Owner.EquippedItem.PresetName .. " range = " .. PrjDat.rng .. " px");
	--ConsoleMan:PrintString("AimPoint range = " .. SceneMan:ShortestDistance(Owner.Pos, AimPoint, false).Magnitude .. " px");

	if Dist:MagnitudeIsLessThan(PrjDat.rng) then
		aim = HumanBehaviors.GetAngleToHit(PrjDat, Dist);
	else
		return true; -- target out of range
	end

	local CheckTargetTimer = Timer();
	local aimError = RangeRand(-0.25, 0.25) * AI.aimSkill;

	AI.fire = false;
	while aim do
		if Owner.FirearmIsReady then
			AI.deviceState = AHuman.AIMING;
			AI.Ctrl.AnalogAim = Vector(1,0):RadRotate(aim+aimError+RangeRand(-0.02, 0.02)*AI.aimSkill);
			if ShootTimer:IsPastRealMS(aimTime) then
				if Weapon.FullAuto then
					AI.fire = true;
				else
					ShootTimer:Reset();
					aimTime = 120 * AI.aimSkill;
					AI.fire = not AI.fire;
				end

				aimError = aimError * 0.985;
			end
		else
			AI.deviceState = AHuman.POINTING;
			AI.fire = false;

			ShootTimer:Reset();
			if Owner.FirearmIsEmpty then
				Owner:ReloadFirearms();
			end

			break; -- stop this behavior when the mag is empty
		end

		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end

		if AI.UnseenTarget and CheckTargetTimer:IsPastRealMS(400*AI.aimSkill) then
			if MovableMan:ValidMO(AI.UnseenTarget) and (AI.UnseenTarget.ClassName == "AHuman" or AI.UnseenTarget.ClassName == "ACrab") then
				CheckTargetTimer:Reset();
				if AI.UnseenTarget:GetController() and AI.UnseenTarget:GetController():IsState(Controller.WEAPON_FIRE) then
					-- compare the enemy aim angle with the angle to us
					local AimEnemy = SceneMan:ShortestDistance(AI.UnseenTarget.EyePos, AI.UnseenTarget.ViewPoint, false).Normalized;
					local DistNormal = SceneMan:ShortestDistance(AI.UnseenTarget.EyePos, Owner.Pos, false).Normalized;
					local dot = DistNormal.X * AimEnemy.X + DistNormal.Y * AimEnemy.Y;
					if dot > 0.4 then
						-- this actor is shooting in our direction
						AimPoint = AI.UnseenTarget.Pos + SceneMan:ShortestDistance(AI.UnseenTarget.Pos, AimPoint, false) / 2 + Vector(RangeRand(-40, 40)*AI.aimSkill, RangeRand(-40, 40)*AI.aimSkill);
						aimError = RangeRand(-0.15, 0.15) * AI.aimSkill;

						Dist = SceneMan:ShortestDistance(Owner.EquippedItem.Pos, AimPoint, false);
						if Dist:MagnitudeIsLessThan(PrjDat.rng) then
							aim = HumanBehaviors.GetAngleToHit(PrjDat, Dist);
						end
					end
				end
			else
				AI.UnseenTarget = nil;
			end
		end
	end
	return true
end

-- stop the user from inadvertently modifying the storage table
local Proxy = {};
local Mt = {
	__index = HumanBehaviors,
	__newindex = function(Table, k, v)
		error("The HumanBehaviors table is read-only.", 2);
	end
}
setmetatable(Proxy, Mt);
HumanBehaviors = Proxy;