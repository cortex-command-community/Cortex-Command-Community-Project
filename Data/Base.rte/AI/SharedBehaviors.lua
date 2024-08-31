SharedBehaviors = {};

function SharedBehaviors.GetTeamShootingSkill(team)
	local skill = 80;
	local Activ = ActivityMan:GetActivity();
	if Activ then
		-- i am fancy mathematician, doing fancy mathematics
		-- this weigh actor skill heavily towards 100
		local num = (Activ:GetTeamAISkill(team)/100);
		skill = (1 - math.pow(1 - num, 3)) * 100;
	end

	local aimSpeed, aimSkill;
	if skill >= Activity.UNFAIRSKILL then
		aimSpeed = 0.04;
		aimSkill = 0.04;
	else
		-- the AI shoot sooner and with slightly better precision
		aimSpeed = 1/(0.65/(2.9-math.exp(skill*0.01)));
		aimSkill = 1/(0.75/(3.0-math.exp(skill*0.01)));
	end
	return aimSpeed, aimSkill, skill;
end

function SharedBehaviors.ProcessAlarmEvent(AI, Owner)
	AI.AlarmPos = nil;

	local loudness, AlarmVec;
	local canSupress = not AI.flying and Owner.FirearmIsReady and Owner.EquippedItem:HasObjectInGroup("Weapons - Explosive");
	for Event in MovableMan.AlarmEvents do
		if Event.Team ~= Owner.Team then	-- caused by some other team's activities - alarming!
			loudness = Owner.AimDistance + Owner.Perceptiveness * Event.Range;
			AlarmVec = SceneMan:ShortestDistance(Owner.EyePos, Event.ScenePos, false); -- see how far away the alarm situation is
			if AlarmVec.Largest < loudness then	-- only react if the alarm is within hearing range
				-- if our relative position to the alarm location is the same, don't repeat the signal
				-- check if we have line of sight to the alarm point
				if (not AI.LastAlarmVec or SceneMan:ShortestDistance(AI.LastAlarmVec, AlarmVec, false):MagnitudeIsGreaterThan(25)) then
					AI.LastAlarmVec = AlarmVec;

					if AlarmVec.Largest < 100 then
						-- check more carfully at close range, and allow hearing of partially blocked alarm events
						if SceneMan:CastStrengthSumRay(Owner.EyePos, Event.ScenePos, 4, rte.grassID) < 100 then
							AI.AlarmPos = Vector(Event.ScenePos.X, Event.ScenePos.Y);
						end
					elseif not SceneMan:CastStrengthRay(Owner.EyePos, AlarmVec, 6, Vector(), 8, rte.grassID, true) then
						AI.AlarmPos = Vector(Event.ScenePos.X, Event.ScenePos.Y);
					end

					if AI.AlarmPos then
						Owner:SetAlarmPoint(AI.AlarmPos);
						AI:CreateFaceAlarmBehavior(Owner);
						return true;
					end
				end
			-- sometimes try to shoot back at enemies outside our view range (0.5 is the range of the brain alarm)
			elseif canSupress and Event.Range > 0.5 and PosRand() > (0.3/AI.aimSkill) and
				AlarmVec.Largest < FrameMan.PlayerScreenWidth * 1.8 and
				(not AI.LastAlarmVec or SceneMan:ShortestDistance(AI.LastAlarmVec, AlarmVec, false).Largest > 30)
			then
				-- only do this if we are facing the shortest distance to the alarm event
				local AimOwner = SceneMan:ShortestDistance(Owner.EyePos, Owner.ViewPoint, false).Normalized;
				local AlarmNormal = AlarmVec.Normalized;
				local dot = AlarmNormal.X * AimOwner.X + AlarmNormal.Y * AimOwner.Y;
				if dot > 0.2 then
					-- check LOS
					local ID = SceneMan:CastMORay(Owner.EyePos, AlarmVec, Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, false, 11);
					if ID ~= rte.NoMOID then
						local FoundMO = MovableMan:GetMOFromID(ID);
						if FoundMO then
							FoundMO = FoundMO:GetRootParent();

							if not FoundMO.EquippedItem or not FoundMO.EquippedItem:HasObjectInGroup("Weapons - Explosive") then
								FoundMO = nil; -- don't shoot at without weapons or actors using tools
							end

							if FoundMO and FoundMO:GetController() and FoundMO:GetController():IsState(Controller.WEAPON_FIRE) and FoundMO.Vel.Largest < 20 then
								-- compare the enemy aim angle with the angle of the alarm vector
								local AimEnemy = SceneMan:ShortestDistance(FoundMO.EyePos, FoundMO.ViewPoint, false).Normalized;
								local dot = AlarmNormal.X * AimEnemy.X + AlarmNormal.Y * AimEnemy.Y;
								if dot < -0.5 then
									-- this actor is shooting in our direction
									AI.ReloadTimer:Reset();
									AI.TargetLostTimer:Reset();

									-- try to shoot back
									AI.UnseenTarget = FoundMO;
									AI:CreateSuppressBehavior(Owner);

									AI.AlarmPos = Event.ScenePos;
									return true;
								end
							end
						end
					else
						AI.LastAlarmVec = AlarmVec; -- don't look here again if the raycast failed
						AI.LastAlarmVec = nil;
					end
				end
			end
		end
	end
end

-- look at the alarm event
function SharedBehaviors.FaceAlarm(AI, Owner, Abort)
	if AI.AlarmPos then
		local AlarmDist = SceneMan:ShortestDistance(Owner.EyePos, AI.AlarmPos, false);
		AI.AlarmPos = nil;
		for _ = 1, math.ceil(200/TimerMan.AIDeltaTimeMS) do
			AI.deviceState = AHuman.AIMING;
			if not Owner.aggressive then
				AI.lateralMoveState = Actor.LAT_STILL;
			end
			AI.Ctrl.AnalogAim = AlarmDist.Normalized;
			local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
			if _abrt then return true end
		end
	end
	return true;
end

-- find the closest enemy brain
function SharedBehaviors.BrainSearch(AI, Owner, Abort)
	if AI.PlayerPreferredHD then
		Owner:EquipNamedDevice(AI.PlayerPreferredHD, true);
	end

	local Brains = {};
	for Act in MovableMan.Actors do
		if Act.Team ~= Owner.Team and Act:HasObjectInGroup("Brains") then
			table.insert(Brains, Act);
		end
	end

	if #Brains < 1 then	-- no brain actors found, check if some other actor is the brain
		local GmActiv = ActivityMan:GetActivity();
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if GmActiv:PlayerActive(player) and GmActiv:GetTeamOfPlayer(player) ~= Owner.Team then
				local Act = GmActiv:GetPlayerBrain(player);
				if Act and MovableMan:IsActor(Act) then
					table.insert(Brains, Act);
				end
			end
		end
	end

	if #Brains > 0 then
		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end

		if #Brains == 1 then
			if MovableMan:IsActor(Brains[1]) then
				Owner:ClearAIWaypoints();
				Owner:AddAIMOWaypoint(Brains[1]);
				AI:CreateGoToBehavior(Owner);
			end
		else	-- lobotomy test
			local ClosestBrain;
			local minDist = math.huge;
			for _, Act in pairs(Brains) do
				-- measure how easy the path to the destination is to traverse
				if MovableMan:IsActor(Act) then
					Owner:ClearAIWaypoints();
					Owner:AddAISceneWaypoint(Act.Pos);
					Owner:UpdateMovePath();

					-- wait until movepath is updated
					while Owner.IsWaitingOnNewMovePath do
						local _ai, _ownr, _abrt = coroutine.yield();
						if _abrt then return true end
					end

					local OldWpt, deltaY;
					local index = 0;
					local height = 0;
					local pathLength = 0;
					local pathObstMaxHeight = 0;

					local PathDump = {}
					-- copy the MovePath to a temporary table so we can yield safely while working on the path
					for WptPos in Owner.MovePath do
						table.insert(PathDump, WptPos);
					end

					for _, Wpt in pairs(PathDump) do
						pathLength = pathLength + 1;
						if OldWpt then
							deltaY = OldWpt.Y - Wpt.Y;
							if deltaY > 20 then	-- Wpt is more than n pixels above OldWpt in the scene
								if deltaY / math.abs(SceneMan:ShortestDistance(OldWpt, Wpt, false).X) > 1 then	-- the slope is more than 45 degrees
									height = height + (OldWpt.Y - Wpt.Y);
									pathObstMaxHeight = math.max(pathObstMaxHeight, height);
								else
									height = 0;
								end
							else
								height = 0;
							end
						end

						OldWpt = Wpt;

						if index > 20 then
							index = 0;
							local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
							if _abrt then return true end
						else
							index = index + 1;
						end
					end

					local score = pathLength * 0.55 + math.floor(pathObstMaxHeight/27) * 8;
					if score < minDist then
						minDist = score;
						ClosestBrain = Act;
					end

					local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
					if _abrt then return true end
				end
			end

			--Owner:ClearAIWaypoints(); -- this part freezes the script when facing the opposing brain

			if MovableMan:IsActor(ClosestBrain) then
				Owner:ClearAIWaypoints(); -- moving the function here fixes it (4zK)
				Owner:AddAIMOWaypoint(ClosestBrain);
				AI:CreateGoToBehavior(Owner);
			else
				return true; -- the brain we found died while we where searching, restart this behavior next frame
			end
		end
	else	-- no enemy brains left
		AI:CreateSentryBehavior(Owner);
	end

	return true;
end

function SharedBehaviors.Patrol(AI, Owner, Abort)
	while AI.flying or Owner.Vel:MagnitudeIsGreaterThan(4) do	-- wait until we are stationary
		return true;
	end

	if Owner.ClassName == "AHuman" then
		if AI.PlayerPreferredHD then
			Owner:EquipNamedDevice(AI.PlayerPreferredHD, true);
		elseif not Owner:EquipDeviceInGroup("Weapons - Primary", true) then
			Owner:EquipDeviceInGroup("Weapons - Secondary", true);
		end
	end

	local Free = Vector();
	local WptA, WptB;

	-- look for a path to the right
	SceneMan:CastObstacleRay(Owner.Pos, Vector(512, 0), Vector(), Free, Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, 4);
	local Dist = SceneMan:ShortestDistance(Owner.Pos, Free, false);

	if Dist:MagnitudeIsGreaterThan(20) then
		Owner:ClearAIWaypoints();
		Owner:AddAISceneWaypoint(Free);	
		Owner:UpdateMovePath();

		-- wait until movepath is updated
		while Owner.IsWaitingOnNewMovePath do
			local _ai, _ownr, _abrt = coroutine.yield();
			if _abrt then return true end
		end

		local PrevPos = Vector(Owner.Pos.X, Owner.Pos.Y);
		for WptPos in Owner.MovePath do
			if math.abs(PrevPos.Y - WptPos.Y) > 14 then
				break;
			end

			WptA = Vector(PrevPos.X, PrevPos.Y);
			PrevPos:SetXY(WptPos.X, WptPos.Y);
		end
	end

	-- look for a path to the left
	SceneMan:CastObstacleRay(Owner.Pos, Vector(-512, 0), Vector(), Free, Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, 4);
	Dist = SceneMan:ShortestDistance(Owner.Pos, Free, false);

	if Dist:MagnitudeIsGreaterThan(20) then
		Owner:ClearAIWaypoints();
		Owner:AddAISceneWaypoint(Free);
		Owner:UpdateMovePath();

		-- wait until movepath is updated
		while Owner.IsWaitingOnNewMovePath do
			local _ai, _ownr, _abrt = coroutine.yield();
			if _abrt then return true end
		end

		local PrevPos = Vector(Owner.Pos.X, Owner.Pos.Y);
		for WptPos in Owner.MovePath do
			if math.abs(PrevPos.Y - WptPos.Y) > 14 then
				break;
			end

			WptB = Vector(PrevPos.X, PrevPos.Y);
			PrevPos:SetXY(WptPos.X, WptPos.Y);
		end
	end

	Owner:ClearAIWaypoints();
	local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
	if _abrt then return true end

	if WptA then
		Dist = SceneMan:ShortestDistance(Owner.Pos, WptA, false);
		if Dist:MagnitudeIsGreaterThan(20) then
			Owner:AddAISceneWaypoint(WptA);
		else
			WptA = nil;
		end
	end

	if WptB then
		Dist = SceneMan:ShortestDistance(Owner.Pos, WptB, false);
		if Dist:MagnitudeIsGreaterThan(20) then
			Owner:AddAISceneWaypoint(WptB);
		else
			WptB = nil;
		end
	end

	if WptA or WptB then
		AI:CreateGoToBehavior(Owner);
	else	-- no path was found
		local FlipTimer = Timer();
		FlipTimer:SetSimTimeLimitMS(3000);
		while true do
			local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
			if _abrt then return true end
			if FlipTimer:IsPastSimTimeLimit() then
				FlipTimer:Reset();
				FlipTimer:SetSimTimeLimitMS(RangeRand(2000, 5000));
				Owner.HFlipped = not Owner.HFlipped	-- turn around and try the other direction sometimes
				if PosRand() < 0.3 then
					break; -- end the behavior
				end
			end
		end
	end
	return true;
end

-- sharp aim at an area where we expect the enemy to be
function SharedBehaviors.PinArea(AI, Owner, Abort)
	if AI.OldTargetPos then
		local AlarmDist = SceneMan:ShortestDistance(Owner.EyePos, AI.OldTargetPos, false);
		for _ = 1, math.ceil(math.random(1000, 3000)/TimerMan.AIDeltaTimeMS) do
			AI.deviceState = AHuman.AIMING;
			AI.lateralMoveState = Actor.LAT_STILL;
			AlarmDist:SetXY(AlarmDist.X+RangeRand(-5,5), AlarmDist.Y+RangeRand(-5,5));
			AI.Ctrl.AnalogAim = AlarmDist.Normalized;
			local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
			if _abrt then return true end
		end
	end
	return true;
end

function SharedBehaviors.GetRealVelocity(Owner)
	-- Calculate a velocity based on our actual movement. This is because otherwise gravity falsely reports that we have a downward velocity, even if our net movement is zero.
	-- Note - we use normal delta time, not AI delta time, because PrevPos is updated per-tick (not per-AI-tick)
	return (Owner.Pos - Owner.PrevPos) / TimerMan.DeltaTimeSecs;
end

function SharedBehaviors.UpdateAverageVel(Owner, AverageVel)
	-- Store an exponential moving average of our speed over the past seconds
	local timeInSeconds = 1;

	local ticksPerTime = timeInSeconds / TimerMan.AIDeltaTimeSecs;
	AverageVel = AverageVel - (AverageVel / ticksPerTime);
	AverageVel = AverageVel + (SharedBehaviors.GetRealVelocity(Owner) / ticksPerTime);

	return AverageVel;
end

-- move to the next waypoint
function SharedBehaviors.GoToWpt(AI, Owner, Abort)
	-- check if we have arrived
	if not (Owner.AIMode == Actor.AIMODE_SQUAD or Owner:GetWaypointListSize() > 0) then
		if not Owner.MOMoveTarget then
			if SceneMan:ShortestDistance(Owner:GetLastAIWaypoint(), Owner.Pos, false).Largest < Owner.Height * 0.15 then
				Owner:ClearAIWaypoints();
				Owner:ClearMovePath();
				Owner:DrawWaypoints(false);
				AI:CreateSentryBehavior(Owner);

				if Owner.AIMode == Actor.AIMODE_GOTO then
					AI.SentryFacing = Owner.HFlipped; -- guard this direction
					AI.SentryPos = Vector(Owner.Pos.X, Owner.Pos.Y); -- guard this point
				end

				return true;
			end
		end
	end

	-- is Y1 lower down in the scene, compared to Y2?
	local Lower = function(Y1, Y2, margin)
		return Y1.Pos and Y2.Pos and (Y1.Pos.Y - margin > Y2.Pos.Y);
	end

	local ArrivedTimer = Timer();
	local UpdatePathTimer = Timer();

	if Owner.MOMoveTarget then
		UpdatePathTimer:SetSimTimeLimitMS(RangeRand(7000, 8000));
	else
		UpdatePathTimer:SetSimTimeLimitMS(RangeRand(12000, 14000));
	end

	local NoLOSTimer = Timer();
	NoLOSTimer:SetSimTimeLimitMS(1000);

	local StuckTimer = Timer();
	StuckTimer:SetSimTimeLimitMS(1000);
	local AverageVel = Owner.Vel;

	local nextLatMove = AI.lateralMoveState;
	local nextAimAngle = Owner:GetAimAngle(false) * 0.95;
	local scanAng = 0; -- for obstacle detection
	local Obstacles = {};
	local PrevWptPos = Vector(Owner.Pos.X, Owner.Pos.Y);
	local sweepCW = true;
	local sweepRange = 0;
	local digState = AHuman.NOTDIGGING;
	local obstacleState = Actor.PROCEEDING;
	local Obst = {R_LOW = 1, R_FRONT = 2, R_HIGH = 3, R_UP = 5, L_UP = 6, L_HIGH = 8, L_FRONT = 9, L_LOW = 10};
	local Facings = {{aim=0, facing=0}, {aim=1.4, facing=1.4}, {aim=1.4, facing=math.pi-1.4}, {aim=0, facing=math.pi}};
	
	local NeedsNewPath, Waypoint, HasMovePath, Dist, CurrDist;
	NeedsNewPath = true;

	Owner:RemoveNumberValue("AI_StuckForTime");

	while true do
		Waypoint = nil;
		HasMovePath = false;

		-- ugh
		for pos in Owner.MovePath do
			HasMovePath = true;
			Waypoint = {};
			Waypoint.Pos = pos;
			Waypoint.Type = nil;
			if Owner.MovePathSize == 1 then
				Waypoint.Type = "last";
			end
			break;
		end

		if Waypoint ~= nil and Waypoint.Type ~= "air" then
			local Free = Vector();

			-- only if we have a digging tool
			if Waypoint.Type ~= "drop" and Owner:HasObjectInGroup("Tools - Diggers") then
				local PathSegRay = SceneMan:ShortestDistance(PrevWptPos, Waypoint.Pos, false); -- detect material blocking the path and start digging through it
				if AI.teamBlockState ~= Actor.BLOCKED and SceneMan:CastStrengthRay(PrevWptPos, PathSegRay, 4, Free, 2, rte.doorID, true) then
					if SceneMan:ShortestDistance(Owner.Pos, Free, false):MagnitudeIsLessThan(Owner.Height*0.4) then	-- check that we're close enough to start digging
						digState = AHuman.STARTDIG;
						AI.deviceState = AHuman.DIGGING;
						obstacleState = Actor.DIGPAUSING;
						nextLatMove = Actor.LAT_STILL;
						sweepRange = math.min(math.pi*0.2, Owner.AimRange);
						StuckTimer:SetSimTimeLimitMS(6000);
						AI.Ctrl.AnalogAim = SceneMan:ShortestDistance(Owner.Pos, Waypoint.Pos, false).Normalized; -- aim in the direction of the next waypoint
					else
						digState = AHuman.NOTDIGGING;
						obstacleState = Actor.PROCEEDING;
					end
				else
					digState = AHuman.NOTDIGGING;
					obstacleState = Actor.PROCEEDING;
					StuckTimer:SetSimTimeLimitMS(1000);
				end
			end

			if digState == AHuman.NOTDIGGING and AI.deviceState ~= AHuman.DIGGING then
				-- if our path isn't blocked enough to dig, but the headroom is too little, start crawling to get through
				local heading = SceneMan:ShortestDistance(Owner.Pos, Waypoint.Pos, false):SetMagnitude(Owner.Height*0.5);

				-- This gets the angle of the heading vector relative to flat (i.e, straight along the X axis)
				-- This gives a range of [0, 90]
				-- 0 is pointing straight left/right, and 90 is pointing straight up/down.
				local angleRadians = math.abs(math.atan2(-(heading.X * heading.Y), heading.X * heading.X));
				local angleDegrees = angleRadians * (180 / math.pi);

				-- We only crawl it it's quite flat, otherwise climb
				local crawlThresholdDegrees = 30;
				if angleDegrees <= crawlThresholdDegrees and Owner.Head and Owner.Head:IsAttached() then
					local topHeadPos = Owner.Head.Pos - Vector(0, Owner.Head.Radius*0.7);

					-- first check up to the top of the head, and then from there forward
					if SceneMan:CastStrengthRay(Owner.Pos, topHeadPos - Owner.Pos, 5, Free, 4, rte.doorID, true) or SceneMan:CastStrengthRay(topHeadPos, heading, 5, Free, 4, rte.doorID, true) then
						AI.proneState = AHuman.PRONE;
					else
						AI.proneState = AHuman.NOTPRONE;
					end
				else
					AI.proneState = AHuman.NOTPRONE;
				end
			end
		end

		if Waypoint == nil or not Waypoint.Type then
			ArrivedTimer:SetSimTimeLimitMS(100);
		elseif Waypoint.Type == "last" then
			ArrivedTimer:SetSimTimeLimitMS(300);
		else	-- air or corner wpt
			ArrivedTimer:SetSimTimeLimitMS(0);
		end

		AverageVel = SharedBehaviors.UpdateAverageVel(Owner, AverageVel);
		
		local stuckThreshold = 2.5; -- pixels per second of movement we need to be considered not stuck

		-- Cap AverageVel, so if we have a spike in velocity it doesn't take too long to come back down
		AverageVel:CapMagnitude(stuckThreshold * 5)

		-- Reset our stuck timer if we're moving
		if AverageVel:MagnitudeIsGreaterThan(stuckThreshold) then
			if StuckTimer:IsPastSimTimeLimit() then
				Owner:RemoveNumberValue("AI_StuckForTime");
			end
			StuckTimer:Reset();
		end

		if AI.refuel and Owner.Jetpack then
			-- if jetpack is full or we are falling we can stop refuelling
			if Owner.Jetpack.JetTimeLeft > Owner.Jetpack.JetTimeTotal * 0.98 or (AI.flying and Owner.Vel.Y < -3 and Owner.Jetpack.JetTimeLeft > AI.minBurstTime*2) then
				AI.refuel = false;
			elseif not AI.flying then
				AI.jump = false;
				AI.lateralMoveState = Actor.LAT_STILL;
			end
		elseif UpdatePathTimer:IsPastSimTimeLimit() then
			UpdatePathTimer:Reset();

			AI.deviceState = AHuman.STILL;
			AI.proneState = AHuman.NOTPRONE;
			AI.jump = false;
			nextLatMove = Actor.LAT_STILL;
			digState = AHuman.NOTDIGGING;
			Waypoint = nil;
			NeedsNewPath = true; -- update the path
		elseif StuckTimer:IsPastSimTimeLimit() then	-- dislodge
			Owner:SetNumberValue("AI_StuckForTime", StuckTimer.ElapsedSimTimeMS);
			if AI.jump then
				if Owner.Jetpack and Owner.Jetpack.JetTimeLeft < AI.minBurstTime then	-- out of fuel
					AI.jump = false;
					AI.refuel = true;
					nextLatMove = Actor.LAT_STILL;
				else
					local chance = PosRand();
					if chance < 0.1 then
						nextLatMove = Actor.LAT_LEFT;
					elseif chance > 0.9 then
						nextLatMove = Actor.LAT_RIGHT;
					else
						nextLatMove = Actor.LAT_STILL;
					end
				end
			else
				local updateInterval = SettingsMan.AIUpdateInterval;

				-- Try swapping direction, with a 15% random chance per tick while we're stuck
				if PosRand() > (1 - 0.15) / updateInterval then
					nextLatMove = AI.lateralMoveState == Actor.LAT_LEFT and Actor.LAT_RIGHT or Actor.LAT_LEFT;
				end

				-- Try swapping prone/unprone, with a 0.5% random chance per tick while we're stuck
				if PosRand() > (1 - 0.005) / updateInterval then
					AI.proneState = AI.proneState == AHuman.PRONE and AHuman.NOTPRONE or AHuman.PRONE;
				end

				-- refuelling done
				if AI.refuel and Owner.Jetpack and Owner.Jetpack.JetpackType == AEJetpack.Standard and Owner.Jetpack.JetTimeLeft >= Owner.Jetpack.JetTimeTotal * 0.99 then
					AI.jump = true;
				end
			end
		elseif not NeedsNewPath then	-- we have a list of waypoints, follow it
			if Owner.MovePathSize == 0 then	-- arrived
				if Owner.MOMoveTarget then -- following actor
					if Owner.MOMoveTarget:IsActor() then
						local Trace = SceneMan:ShortestDistance(Owner.Pos, Owner.MOMoveTarget.Pos, false);
						if Trace.Largest < Owner.Height * 0.5 + (Owner.MOMoveTarget.Height or 100) * 0.5 and
							SceneMan:CastStrengthRay(Owner.Pos, Trace, 5, Vector(), 4, rte.grassID, true)
						then -- add a waypoint if the MOMoveTarget is close and in LOS
							Waypoint = {Pos=SceneMan:MovePointToGround(Owner.MOMoveTarget.Pos, Owner.Height*0.2, 4)};
						else
							NeedsNewPath = true; -- update the path
						end
					end
				else	-- moving towards a scene point
					--local GroundPos = Owner:GetLastAIWaypoint()
					local GroundPos = SceneMan:MovePointToGround(Owner:GetLastAIWaypoint(), Owner.Height*0.2, 4);
					if SceneMan:ShortestDistance(GroundPos, Owner.Pos, false).Largest < Owner.Height * 0.4 then
						if Owner.AIMode == Actor.AIMODE_GOTO then
							AI.SentryFacing = Owner.HFlipped; -- guard this direction
							AI.SentryPos = Vector(Owner.Pos.X, Owner.Pos.Y); -- guard this point
							AI:CreateSentryBehavior(Owner);
						end

						Owner:ClearAIWaypoints();
						Owner:ClearMovePath();
						Owner:DrawWaypoints(false);
						return true;
					end
				end
			else
				if Waypoint then
					if Owner.MOMoveTarget and MovableMan:ValidMO(Owner.MOMoveTarget) then
						local Trace = SceneMan:ShortestDistance(Owner.Pos, Owner.MOMoveTarget.Pos, false);

						if Owner.MOMoveTarget.Team == Owner.Team then
							if Trace.Largest > Owner.Height * 0.3 + (Owner.MOMoveTarget.Height or 100) * 0.3 then
								Waypoint.Pos = Owner.MOMoveTarget.Pos;
							else	-- arrived
								if not AI.flying then
									while true do
										StuckTimer:Reset();
										UpdatePathTimer:Reset();
										AI.lateralMoveState = Actor.LAT_STILL;
										AI.jump = false;

										if Owner.MOMoveTarget and MovableMan:ValidMO(Owner.MOMoveTarget) then
											Trace = SceneMan:ShortestDistance(Owner.Pos, Owner.MOMoveTarget.Pos, false);
											if Trace.Largest > Owner.Height * 0.4 + (Owner.MOMoveTarget.Height or 100) * 0.4 or
												SceneMan:CastStrengthRay(Owner.Pos, Trace, 5, Vector(), 4, rte.doorID, true)
											then
												Waypoint = nil;
												NeedsNewPath = true; -- update the path
												break;
											end
										else -- MOMoveTarget gone
											return true;
										end
									end
								end
							end
						elseif Trace.Largest < Owner.Height * 0.33 + (Owner.MOMoveTarget.Height or 100) * 0.33 then -- enemy MO
							Waypoint.Pos = Owner.MOMoveTarget.Pos;
						end
					end

					if Waypoint then
						CurrDist = SceneMan:ShortestDistance(Owner.Pos, Waypoint.Pos, false);

						-- digging
						if digState ~= AHuman.NOTDIGGING then
							if not AI.Target and Owner:EquipDiggingTool(true) then	-- switch to the digger if we have one
								if Owner.FirearmIsEmpty then	-- reload if it's empty
									AI.fire = false;
									AI.Ctrl:SetState(Controller.WEAPON_RELOAD, true);
								else
									if AI.teamBlockState == Actor.BLOCKED then
										AI.fire = false;
										nextLatMove = Actor.LAT_STILL;
									else
										if obstacleState == Actor.PROCEEDING then
											if CurrDist.X < -1 then
												nextLatMove = Actor.LAT_LEFT;
											elseif CurrDist.X > 1 then
												nextLatMove = Actor.LAT_RIGHT;
											end
										else
											nextLatMove = Actor.LAT_STILL;
										end

										-- check if we are close enough to dig
										if SceneMan:ShortestDistance(PrevWptPos, Owner.Pos, false):MagnitudeIsGreaterThan(Owner.Height*0.5) and
											 SceneMan:ShortestDistance(Owner.Pos, Waypoint.Pos, false):MagnitudeIsGreaterThan(Owner.Height*0.5)
										then
											digState = AHuman.NOTDIGGING;
											obstacleState = Actor.PROCEEDING;
											AI.deviceState = AHuman.STILL;
											AI.fire = false;
											Owner:EquipFirearm(true);
										else
											-- see if we have dug out all that we can in the sweep area without moving closer
											local centerAngle = CurrDist.AbsRadAngle;
											local Ray = Vector(Owner.Height*0.3, 0):RadRotate(centerAngle); -- center
											if SceneMan:CastNotMaterialRay(Owner.Pos, Ray, 0, 3, false) < 0 then
												-- now check the tunnel's thickness
												Ray = Vector(Owner.Height*0.3, 0):RadRotate(centerAngle + sweepRange); -- up
												if SceneMan:CastNotMaterialRay(Owner.Pos, Ray, rte.airID, 3, false) < 0 then
													Ray = Vector(Owner.Height*0.3, 0):RadRotate(centerAngle - sweepRange); -- down
													if SceneMan:CastNotMaterialRay(Owner.Pos, Ray, rte.airID, 3, false) < 0 then
														obstacleState = Actor.PROCEEDING; -- ok the tunnel section is clear, so start walking forward while still digging
													else
														obstacleState = Actor.DIGPAUSING; -- tunnel cavity not clear yet, so stay put and dig some more
													end
												end
											else
												obstacleState = Actor.DIGPAUSING; -- tunnel cavity not clear yet, so stay put and dig some more
											end

											local aimAngle = Owner:GetAimAngle(true);
											local AimVec = Vector(1, 0):RadRotate(aimAngle);

											local angDiff = math.asin(AimVec:Cross(CurrDist.Normalized)); -- the angle between CurrDist and AimVec
											if math.abs(angDiff) < sweepRange then
												AI.fire = true; -- only fire the digger at the obstacle
											else
												AI.fire = false;
											end

											-- sweep the digger between the two endpoints of the obstacle
											local DigTarget;
											if sweepCW then
												DigTarget = Vector(Owner.Height*0.4, 0):RadRotate(centerAngle + sweepRange);
											else
												DigTarget = Vector(Owner.Height*0.4, 0):RadRotate(centerAngle - sweepRange);
											end

											angDiff = math.asin(AimVec:Cross(DigTarget.Normalized)); -- The angle between DigTarget and AimVec
											if math.abs(angDiff) < 0.1 then
												sweepCW = not sweepCW; -- this is close enough, go in the other direction next frame
											else
												AI.Ctrl.AnalogAim = (Vector(AimVec.X, AimVec.Y):RadRotate(-angDiff*0.15)).Normalized;
											end

											-- check if we are done when we get close enough to the waypoint
											if Owner.AIMode == Actor.AIMODE_GOLDDIG then
												Waypoint.Pos = SceneMan:MovePointToGround(Waypoint.Pos, Owner.Height*0.2, 4);
											end
										end
									end
								end
							else
								digState = AHuman.NOTDIGGING;
								obstacleState = Actor.PROCEEDING;
								AI.deviceState = AHuman.STILL;
								AI.fire = false;
								Owner:EquipFirearm(true);
							end
						else	-- not digging
							if not AI.Target then
								AI.fire = false;
							end

							-- Scan for obstacles
							local Trace = Vector(Owner.Radius*0.75, 0):RadRotate(scanAng);
							local Free = Vector();
							local index = math.floor(scanAng*2.5+2.01);
							if SceneMan:CastObstacleRay(Owner.Pos, Trace, Vector(), Free, Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, 3) > -1 then
								Obstacles[index] = true;
							else
								Obstacles[index] = false;
							end

							if scanAng < 1.57 then	-- pi/2
								if scanAng > 1.2 then
									scanAng = 1.89;
								else
									scanAng = scanAng + 0.55;
								end
							else
								if scanAng > 3.5 then
									scanAng = -0.4;
								else
									scanAng = scanAng + 0.55;
								end
							end

							local tolerance = Owner.MoveProximityLimit;
							if AI.jump then
								tolerance = tolerance * 2;
							end

							if CurrDist:MagnitudeIsGreaterThan(tolerance) then	-- not close enough to the waypoint
								ArrivedTimer:Reset();

								-- check if we have LOS to the waypoint
								if SceneMan:CastObstacleRay(Owner.Pos, CurrDist, Vector(), Vector(), Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, 9) < 0 then
									NoLOSTimer:Reset();
								elseif NoLOSTimer:IsPastSimTimeLimit() then	-- calculate new path
									Waypoint = nil;
									NeedsNewPath = true; -- update the path
									nextLatMove = Actor.LAT_STILL;

									if Owner.AIMode == Actor.AIMODE_GOLDDIG and digState == AHuman.NOTDIGGING and math.random() < 0.5 then
										return true; -- end this behavior and look for gold again
									end
								end
							elseif ArrivedTimer:IsPastSimTimeLimit() then	-- only remove a waypoint if we have been close to it for a while
								if Waypoint.Type == "last" then
									if not AI.flying and Owner.Vel.Largest < 5 then
										if not Owner.MOMoveTarget then
											local ProxyWpt = SceneMan:MovePointToGround(Owner:GetLastAIWaypoint(), Owner.Height*0.2, 4);
											if SceneMan:ShortestDistance(Owner.Pos, ProxyWpt, false).Largest < Owner.Height*0.4 then
												Owner:ClearAIWaypoints();
												Owner:ClearMovePath();
												Owner:DrawWaypoints(false);
												break;
											end
										end

										PrevWptPos = Waypoint.Pos;
										Owner:RemoveMovePathBeginning();
										Waypoint = nil;
										NeedsNewPath = true; -- update the path
									end
								else
									PrevWptPos = Waypoint.Pos;
									Owner:RemoveMovePathBeginning();
									Waypoint = nil;
								end
							end

							if Waypoint then	-- move towards the waypoint
								-- control horizontal movement
								if not AI.flying then
									if CurrDist.X < -3 then
										nextLatMove = Actor.LAT_LEFT;
									elseif CurrDist.X > 3 then
										nextLatMove = Actor.LAT_RIGHT;
									else
										nextLatMove = Actor.LAT_STILL;
									end
								end

								if Waypoint.Type == "right" then
									if CurrDist.X > -3 then
										nextLatMove = Actor.LAT_RIGHT;
									end
								elseif Waypoint.Type == "left" then
									if CurrDist.X < 3 then
										nextLatMove = Actor.LAT_LEFT;
									end
								end

								if Owner.Jetpack then
									if Owner.Jetpack.JetTimeLeft < AI.minBurstTime then
										if not AI.flying or Owner.Vel.Y > 4 then
											AI.jump = false; -- not enough fuel left, no point in jumping yet
											AI.refuel = true;
										end
									else
										-- do we have a target we want to shoot at?
										if (AI.Target and AI.canHitTarget and AI.BehaviorName ~= "AttackTarget") then
											-- are we also flying
											if AI.flying and Owner.Jetpack.JetpackType == AEJetpack.Standard then
												-- predict jetpack movement when jumping and there is a target (check one direction)
												local jetStrength = AI.jetImpulseFactor / Owner.Mass;
												local t = math.min(0.4, Owner.Jetpack.JetTimeLeft*0.001);
												local PixelVel = Owner.Vel * (GetPPM() * t);
												local Accel = SceneMan.GlobalAcc * GetPPM();

												-- a burst use 10x more fuel
												if Owner.Jetpack:CanTriggerBurst() then
													t = math.max(math.min(0.4, Owner.Jetpack.JetTimeLeft*0.001-TimerMan.AIDeltaTimeSecs*10), TimerMan.AIDeltaTimeSecs);
												end

												-- test jumping
												local JetAccel = Accel + Vector(-jetStrength, 0):RadRotate(Owner.RotAngle+1.375*math.pi+Owner:GetAimAngle(false)*0.25);
												local JumpPos = Owner.Head.Pos + PixelVel + JetAccel * (t*t*0.5);

												-- a burst add a one time boost to acceleration
												if Owner.Jetpack:CanTriggerBurst() then
													JumpPos = JumpPos + Vector(-AI.jetBurstFactor, 0):AbsRotateTo(JetAccel);
												end

												-- check for obstacles from the head
												Trace = SceneMan:ShortestDistance(Owner.Head.Pos, JumpPos, false);
												local jumpScore = SceneMan:CastObstacleRay(Owner.Head.Pos, Trace, JumpPos, Vector(), Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, 3);
												if jumpScore < 0 then	-- no obstacles: calculate the distance from the future pos to the wpt
													jumpScore = SceneMan:ShortestDistance(Waypoint.Pos, JumpPos, false).Magnitude;
												else -- the ray hit terrain or start inside terrain: avoid
													jumpScore = SceneMan:ShortestDistance(Waypoint.Pos, JumpPos, false).Largest * 2;
												end

												-- test falling
												local FallPos = Owner.Head.Pos + PixelVel + Accel * (t*t*0.5);

												-- check for obstacles when falling/walking
												local Trace = SceneMan:ShortestDistance(Owner.Head.Pos, FallPos, false);
												SceneMan:CastObstacleRay(Owner.Head.Pos, Trace, FallPos, Vector(), Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, 3);

												if SceneMan:ShortestDistance(Waypoint.Pos, FallPos, false):MagnitudeIsLessThan(jumpScore) then
													AI.jump = false;
												else
													AI.jump = true;
												end
											else
												AI.jump = false;
											end
										else
											if Waypoint.Type ~= "drop" and not Lower(Waypoint, Owner, 20) and Owner.Jetpack.JetpackType == AEJetpack.Standard then
												-- jump over low obstacles unless we want to jump off a ledge
												if nextLatMove == Actor.LAT_RIGHT and (Obstacles[Obst.R_LOW] or Obstacles[Obst.R_FRONT]) and not Obstacles[Obst.R_UP] then
													AI.jump = true;
													if Obstacles[Obst.R_HIGH] then
														nextLatMove = Actor.LAT_LEFT; -- TODO: only when too close to the obstacle?
													end
												elseif nextLatMove == Actor.LAT_LEFT and (Obstacles[Obst.L_LOW] or Obstacles[Obst.L_FRONT]) and not Obstacles[Obst.L_UP] then
													AI.jump = true;
													if Obstacles[Obst.L_HIGH] then
														nextLatMove = Actor.LAT_RIGHT; -- TODO: only when too close to the obstacle?
													end
												end
											end

											-- predict jetpack movement...
											local jetStrength = (AI.jetImpulseFactor / Owner.Mass);
											local t = math.min(0.4, Owner.Jetpack.JetTimeLeft*0.001);
											local PixelVel = Owner.Vel * (GetPPM() * t);
											local Accel = SceneMan.GlobalAcc * GetPPM();

											-- a burst use 10x more fuel
											if Owner.Jetpack:CanTriggerBurst() then
												t = math.max(math.min(0.4, Owner.Jetpack.JetTimeLeft*0.001-TimerMan.AIDeltaTimeSecs*10), TimerMan.AIDeltaTimeSecs);
											end

											-- when jumping (check four directions)
											for k, Face in pairs(Facings) do
												local JetAccel = Vector(-jetStrength, 0):RadRotate(Owner.RotAngle+1.375*math.pi+Face.facing*0.25);
												local JumpPos = Owner.Pos + PixelVel + (Accel + JetAccel) * (t*t*0.5);

												-- a burst add a one time boost to acceleration
												if Owner.Jetpack:CanTriggerBurst() then
													JumpPos = JumpPos + Vector(-AI.jetBurstFactor, 0):AbsRotateTo(JetAccel);
												end

												-- check for obstacles from the head
												Trace = SceneMan:ShortestDistance(Owner.Pos, JumpPos, false);
												Facings[k].range = SceneMan:ShortestDistance(Waypoint.Pos, JumpPos, false).Magnitude;
											end

											-- when falling or walking
											local FallPos = Owner.Pos + PixelVel;
											if AI.flying then
												FallPos = FallPos + Accel * (t*t*0.5);
											end

											-- check for obstacles when falling/walking
											local Trace = SceneMan:ShortestDistance(Owner.Pos, FallPos, false);
											SceneMan:CastObstacleRay(Owner.Pos, Trace, FallPos, Vector(), Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, 3);

											local deltaToJump = 5;
											if Owner.Jetpack.JetpackType == AEJetpack.JumpPack then
												deltaToJump = deltaToJump * 1.4;
											end

											table.sort(Facings, function(A, B) return A.range < B.range end);
											local delta = SceneMan:ShortestDistance(Waypoint.Pos, FallPos, false).Magnitude - Facings[1].range;
											if delta < 1 then
												AI.jump = false;
											elseif delta > deltaToJump or (AI.flying and Owner.Jetpack.JetpackType == AEJetpack.Standard) then
												AI.jump = true;
												nextAimAngle = Owner:GetAimAngle(false) * 0.5 + Facings[1].aim * 0.5; -- adjust jetpack nozzle direction
												nextLatMove = Actor.LAT_STILL;

												if Facings[1].facing > 1.4 then
													if not Owner.HFlipped then
														nextLatMove = Actor.LAT_LEFT;
													end
												elseif Owner.HFlipped then
													nextLatMove = Actor.LAT_RIGHT;
												end
											end
										end
									end
								end
							end
						end
					end
				end
			end
		else	-- no waypoint list
			NeedsNewPath = false;

			local Trace = SceneMan:ShortestDistance(Owner.Pos, Owner:GetLastAIWaypoint(), false);
			Owner:UpdateMovePath();

			-- wait until movepath is updated
			while Owner.IsWaitingOnNewMovePath do
				local _ai, _ownr, _abrt = coroutine.yield();
				if _abrt then return true end
			end

			-- have we arrived?
			if not Owner.MOMoveTarget then
				local ProxyWpt = SceneMan:MovePointToGround(Owner:GetLastAIWaypoint(), Owner.Height*0.2, 5);
				local Trace = SceneMan:ShortestDistance(Owner.Pos, ProxyWpt, false);
				if Trace.Largest < Owner.Height*0.25 and not SceneMan:CastStrengthRay(Owner.Pos, Trace, 6, Vector(), 3, rte.grassID, true) then
					if Owner.AIMode == Actor.AIMODE_GOTO then
						AI.SentryPos = Vector(Owner.Pos.X, Owner.Pos.Y);
						AI:CreateSentryBehavior(Owner);
					end

					Owner:ClearAIWaypoints();
					Owner:ClearMovePath();
					Owner:DrawWaypoints(false);

					break;
				end
			end

			Owner:DrawWaypoints(true);
			NoLOSTimer:Reset();
		end

		-- movement commands
		if (AI.Target and AI.BehaviorName ~= "AttackTarget") or (Owner.AIMode ~= Actor.AIMODE_SQUAD and (AI.BehaviorName == "ShootArea" or AI.BehaviorName == "FaceAlarm")) then
			if Owner.aggressive then	-- the aggressive behavior setting makes the AI pursue waypoint at all times
				AI.lateralMoveState = nextLatMove;
			else
				AI.lateralMoveState = Actor.LAT_STILL;
			end
			if not AI.flying then
				AI.jump = false;
			end
		else
			AI.lateralMoveState = nextLatMove;
			if digState == AHuman.NOTDIGGING then
				Owner:SetAimAngle(nextAimAngle);
				nextAimAngle = Owner:GetAimAngle(false) * 0.95; -- look straight ahead
			end
		end

		local _ai, _ownr, _abrt = coroutine.yield(); -- wait until next frame
		if _abrt then return true end
	end

	Owner:RemoveNumberValue("AI_StuckForTime");
	return true;
end

function SharedBehaviors.CalculateThreatLevel(MO, Owner)
	-- prioritize closer targets
	local priority = -SceneMan:ShortestDistance(Owner.Pos, MO.Pos, false).Largest / FrameMan.PlayerScreenWidth;

	-- prioritize the weaker humans over crabs
	if MO.ClassName == "AHuman" then
		if MO.FirearmIsReady then	-- prioritize armed targets
			priority = priority + 1.0;
		else
			priority = priority + 0.5;
		end
	elseif MO.ClassName == "ACrab" then
		if MO.FirearmIsReady then	-- prioritize armed targets
			priority = priority + 0.7;
		else
			priority = priority + 0.3;
		end
	elseif MO.ClassName == "ADoor" then
		priority = priority * 0.3;
	end

	return priority - MO.Health / 500; -- prioritize damaged targets
end

-- get the projectile properties from the magazine
function SharedBehaviors.GetProjectileData(Owner)
	local PrjDat = {MagazineName = ""};
	local Weapon, Round, Projectile;
	if Owner.EquippedItem and IsHDFirearm(Owner.EquippedItem) then
		Weapon = ToHDFirearm(Owner.EquippedItem);
		if Weapon.Magazine then
			Round = Weapon.Magazine.NextRound;
			Projectile = Round.NextParticle;
			PrjDat.MagazineName = Weapon.Magazine.PresetName;
		end
	end
	if Round == nil or Round.IsEmpty then	-- set default values if there is no particle
		PrjDat.g = 0;
		PrjDat.vel = 100;
		PrjDat.rng = math.huge;
		PrjDat.pen = math.huge;
		PrjDat.blast = 0;
	else
		-- find muzzle velocity
		PrjDat.vel = Weapon:GetAIFireVel();
		-- half of the theoretical upper limit for the total amount of material strength this weapon can destroy in 250ms

		PrjDat.g = SceneMan.GlobalAcc.Y * 0.67 * Weapon:GetBulletAccScalar(); -- underestimate gravity
		PrjDat.vsq = PrjDat.vel^2; -- muzzle velocity squared
		PrjDat.vqu = PrjDat.vsq^2; -- muzzle velocity quad
		PrjDat.drg = 1 - Projectile.AirResistance * TimerMan.DeltaTimeSecs; -- AirResistance is stored as the ini-value times 60
		PrjDat.thr = math.min(Projectile.AirThreshold, PrjDat.vel);
		PrjDat.pen = (Projectile.Mass * Projectile.Sharpness * PrjDat.vel) * PrjDat.drg;

		PrjDat.blast = Weapon:GetAIBlastRadius();
		if PrjDat.blast > 0 or Weapon:IsInGroup("Weapons - Explosive") then
			PrjDat.exp = true; -- set this for legacy reasons
			PrjDat.pen = PrjDat.pen + 100;
		end

		-- estimate theoretical max range with ...
		local lifeTime = Weapon:GetAIBulletLifeTime();
		if lifeTime < 1 then	-- infinite life time
			PrjDat.rng = math.huge;
		elseif PrjDat.drg < 1 then	-- AirResistance
			PrjDat.rng = 0;
			local threshold = PrjDat.thr * rte.PxTravelledPerFrame; -- AirThreshold in pixels/frame
			local vel = PrjDat.vel * rte.PxTravelledPerFrame; -- muzzle velocity in pixels/frame

			for _ = 0, math.ceil(lifeTime/TimerMan.DeltaTimeMS) do
				PrjDat.rng = PrjDat.rng + vel;
				if vel > threshold then
					vel = vel * PrjDat.drg;
				end
			end
		else	-- no AirResistance
			PrjDat.rng = PrjDat.vel * rte.PxTravelledPerFrame * (lifeTime / TimerMan.DeltaTimeMS);
		end

		-- Artificially decrease reported range to make sure AI
		-- is close enough to reach target with current weapon
		-- even if it the range is calculated incorrectly
		PrjDat.rng = PrjDat.rng * 0.9;
	end

	return PrjDat;
end

-- stop the user from inadvertently modifying the storage table
local Proxy = {};
local Mt = {
	__index = SharedBehaviors,
	__newindex = function(Table, k, v)
		error("The SharedBehaviors table is read-only.", 2);
	end
}
setmetatable(Proxy, Mt);
SharedBehaviors = Proxy;