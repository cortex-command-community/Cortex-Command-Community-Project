SharedBehaviors = {};

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

		AverageVel = HumanBehaviors.UpdateAverageVel(Owner, AverageVel);
		
		local stuckThreshold = 2.5; -- pixels per second of movement we need to be considered not stuck

		-- Cap AverageVel, so if we have a spike in velocity it doesn't take too long to come back down
		AverageVel:CapMagnitude(stuckThreshold * 5)

		-- Reset our stuck timer if we're moving
		if AverageVel:MagnitudeIsGreaterThan(stuckThreshold) then
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

								if Owner.Jetpack and Owner.Head and Owner.Head:IsAttached() then
									if Owner.Jetpack.JetTimeLeft < AI.minBurstTime then
										AI.jump = false; -- not enough fuel left, no point in jumping yet
										if not AI.flying or Owner.Vel.Y > 4 then
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
											local jumpHeight = Owner.JumpHeight;

											-- when jumping (check four directions)
											for k, Face in pairs(Facings) do
												local JetAccel = Vector(-jumpHeight, 0):RadRotate(Owner.RotAngle+1.375*math.pi+Face.facing*0.25);
												local JumpPos = Owner.Head.Pos + JetAccel;

												-- check for obstacles from the head
												Trace = SceneMan:ShortestDistance(Owner.Head.Pos, JumpPos, false);
												local obstDist = SceneMan:CastObstacleRay(Owner.Head.Pos, Trace, JumpPos, Vector(), Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, 3);
												if obstDist < 0 then	-- no obstacles: calculate the distance from the future pos to the wpt
													Facings[k].range = SceneMan:ShortestDistance(Waypoint.Pos, JumpPos, false).Magnitude;
												else -- the ray hit terrain or start inside terrain: avoid
													Facings[k].range = SceneMan:ShortestDistance(Waypoint.Pos, JumpPos, false).Largest * 2;
												end
											end

											-- when falling or walking
											local FallPos = Owner.Head.Pos + PixelVel;
											if AI.flying then
												FallPos = FallPos + Accel * (t*t*0.5);
											end

											-- check for obstacles when falling/walking
											local Trace = SceneMan:ShortestDistance(Owner.Head.Pos, FallPos, false);
											SceneMan:CastObstacleRay(Owner.Head.Pos, Trace, FallPos, Vector(), Owner.ID, Owner.IgnoresWhichTeam, rte.grassID, 3);

											local deltaToJump = 15;
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

	return true;
end