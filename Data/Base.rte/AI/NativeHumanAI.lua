require("Constants")
require("AI/HumanBehaviors");

NativeHumanAI = {};

function NativeHumanAI:Create(Owner)
	local Members = {};

	Members.lateralMoveState = Actor.LAT_STILL;
	Members.proneState = AHuman.NOTPRONE;
	Members.jumpState = AHuman.NOTJUMPING;
	Members.deviceState = AHuman.STILL;
	Members.lastAIMode = Actor.AIMODE_NONE;
	Members.teamBlockState = Actor.NOTBLOCKED;
	Members.SentryFacing = Owner.HFlipped;
	Members.fire = false;
	Members.groundContact = 5;
	Members.flying = false;

	Members.squadShoot = false;
	Members.useMedikit = false;

	-- timers
	Members.AirTimer = Timer();
	Members.PickUpTimer = Timer();
	Members.ReloadTimer = Timer();
	Members.BlockedTimer = Timer();
	Members.SquadShootTimer = Timer();
	Members.SquadShootDelay = math.random(50,100);

	Members.AlarmTimer = Timer();
	Members.AlarmTimer:SetSimTimeLimitMS(400);

	Members.TargetLostTimer = Timer();
	Members.TargetLostTimer:SetSimTimeLimitMS(1000);
	
	-- customizable variables, trend started by pawnis on 01/09/2023 :)
	
	-- humble beginnings
	-- pause time between sweeping aim up and down when guarding
	Members.idleAimTime = Owner:NumberValueExists("AIIdleAimTime") and Owner:GetNumberValue("AIIdleAimTime") or 500;

	-- set shooting skill
	Members.aimSpeed, Members.aimSkill, Members.skill = HumanBehaviors.GetTeamShootingSkill(Owner.Team);
	
	Members.aimSpeed = Owner:NumberValueExists("AIAimSpeed") and Owner:GetNumberValue("AIAimSpeed") or Members.aimSpeed;
	Members.aimSkill = Owner:NumberValueExists("AIAimSkill") and Owner:GetNumberValue("AIAimSkill") or Members.aimSkill;
	Members.skill = Owner:NumberValueExists("AISkill") and Owner:GetNumberValue("AISkill") or Members.skill;
	
	-- default to enhanced AI if AI skill has been set high enough
	if Members.skill >= GameActivity.NUTSDIFFICULTY or Owner:HasObjectInGroup("Brains") or Owner:HasObjectInGroup("Actors - Snipers") or Owner:HasObjectInGroup("Actors - Boss") then
		Members.SpotTargets = HumanBehaviors.CheckEnemyLOS;
	else
		Members.SpotTargets = HumanBehaviors.LookForTargets;
	end

	-- check if this team is controlled by a human
	if ActivityMan:GetActivity():IsHumanTeam(Owner.Team) then
		Members.isPlayerOwned = true;
		Members.PlayerInterferedTimer = Timer();
		Members.PlayerInterferedTimer:SetSimTimeLimitMS(500);
	end

	-- the native AI assume the jetpack cannot be destroyed
	if Owner.Jetpack then
		if not Members.isPlayerOwned then
			Owner.Jetpack.JetTimeTotal = Owner.Jetpack.JetTimeTotal * 1.2;	-- increase jetpack fuel to compensate for extra fuel spend
		end

		Members.minBurstTime = math.min(Owner.Jetpack.BurstSpacing*2, Owner.Jetpack.JetTimeTotal*0.99); -- in milliseconds
	end

	setmetatable(Members, self);
	self.__index = self;
	return Members;
end

function NativeHumanAI:Update(Owner)
	self.Ctrl = Owner:GetController();

	-- Our jetpack might have thrust balancing enabled, so update for our current mass
	if Owner.Jetpack then		
		self.jetImpulseFactor = Owner.Jetpack:EstimateImpulse(false) * GetPPM() / TimerMan.DeltaTimeSecs;
		self.jetBurstFactor = (Owner.Jetpack:EstimateImpulse(true) * GetPPM() / TimerMan.DeltaTimeSecs - self.jetImpulseFactor) * math.pow(TimerMan.DeltaTimeSecs, 2) * 0.5;
	end

	if self.isPlayerOwned then
		if self.PlayerInterferedTimer:IsPastSimTimeLimit() then
			-- Tell the coroutines to abort to avoid memory leaks
			if self.Behavior then
				local msg, done = coroutine.resume(self.Behavior, self, Owner, true);
			end

			self.Behavior = nil; -- remove the current behavior
			self.BehaviorName = nil;
			if self.BehaviorCleanup then
				self.BehaviorCleanup(self); -- clean up after the current behavior
				self.BehaviorCleanup = nil;
			end

			-- Tell the coroutines to abort to avoid memory leaks
			if self.GoToBehavior then
				local msg, done = coroutine.resume(self.GoToBehavior, self, Owner, true);
			end

			self.GoToBehavior = nil;
			self.GoToName = nil;
			if self.GoToCleanup then
				self.GoToCleanup(self);
				self.GoToCleanup = nil;
			end

			self.Target = nil;
			self.UnseenTarget = nil;
			self.OldTargetPos = nil;
			self.PickupHD = nil;

			self.fire = false;
			self.canHitTarget = false;
			self.jump = false;

			self.squadShoot = false;
			self.useMedikit = false;

			self.proneState = AHuman.NOTPRONE;
			self.SentryFacing = Owner.HFlipped;
			self.deviceState = AHuman.STILL;
			self.lastAIMode = Actor.AIMODE_NONE;
			self.teamBlockState = Actor.NOTBLOCKED;

			if Owner.EquippedItem then
				self.PlayerPreferredHD = Owner.EquippedItem:GetModuleAndPresetName();
			else
				self.PlayerPreferredHD = nil;
			end
		end

		self.PlayerInterferedTimer:Reset();
	end

	if self.Target and not MovableMan:ValidMO(self.Target) then
		self.Target = nil;
	end

	if self.UnseenTarget and not MovableMan:ValidMO(self.UnseenTarget) then
		self.UnseenTarget = nil;
	end

	-- switch to the next behavior, if available
	if self.NextBehavior then
		if self.BehaviorCleanup then
			self.BehaviorCleanup(self);
		end

		-- Tell the coroutines to abort to avoid memory leaks
		if self.Behavior then
			local msg, done = coroutine.resume(self.Behavior, self, Owner, true);
		end

		self.Behavior = self.NextBehavior;
		self.BehaviorCleanup = self.NextCleanup;
		self.BehaviorName = self.NextBehaviorName;

		self.NextBehavior = nil;
		self.NextCleanup = nil;
		self.NextBehaviorName = nil;
	end

	-- switch to the next GoTo behavior, if available
	if self.NextGoTo then
		if self.GoToCleanup then
			self.GoToCleanup(self);
		end

		-- Tell the coroutines to abort to avoid memory leaks
		if self.GoToBehavior then
			local msg, done = coroutine.resume(self.GoToBehavior, self, Owner, true);
		end

		self.GoToBehavior = self.NextGoTo;
		self.GoToCleanup = self.NextGoToCleanup;
		self.GoToName = self.NextGoToName;

		self.NextGoTo = nil;
		self.NextGoToCleanup = nil;
		self.NextGoToName = nil;
	end

	-- check if the AI mode has changed or if we need a new behavior
	if Owner.AIMode ~= self.lastAIMode or not(self.Behavior or self.GoToBehavior) then
		-- Tell the coroutines to abort to avoid memory leaks
		if self.Behavior then
			local msg, done = coroutine.resume(self.Behavior, self, Owner, true);
		end

		self.Behavior = nil;
		if self.BehaviorCleanup then
			self.BehaviorCleanup(self); -- stop the current behavior
			self.BehaviorCleanup = nil;
		end

		-- Tell the coroutines to abort to avoid memory leaks
		if self.GoToBehavior then
			local msg, done = coroutine.resume(self.GoToBehavior, self, Owner, true);
		end

		self.GoToBehavior = nil;
		if self.GoToCleanup then
			self.GoToCleanup(self);
			self.GoToCleanup = nil;
		end

		-- select a new behavior based on AI mode
		if Owner.AIMode == Actor.AIMODE_GOTO or Owner.AIMode == Actor.AIMODE_SQUAD then
			self:CreateGoToBehavior(Owner);
		elseif Owner.AIMode == Actor.AIMODE_BRAINHUNT then
			self:CreateBrainSearchBehavior(Owner);
		elseif Owner.AIMode == Actor.AIMODE_GOLDDIG then
			self:CreateGoldDigBehavior(Owner);
		elseif Owner.AIMode == Actor.AIMODE_PATROL then
			self:CreatePatrolBehavior(Owner);
		else
			if Owner.AIMode ~= self.lastAIMode and Owner.AIMode == Actor.AIMODE_SENTRY then
				self.SentryFacing = Owner.HFlipped; -- store the direction in which we should be looking
				self.SentryPos = Vector(Owner.Pos.X, Owner.Pos.Y); -- store the pos on which we should be standing
			end

			self:CreateSentryBehavior(Owner);
		end

		self.lastAIMode = Owner.AIMode;
	end


	-- check if the feet reach the ground
	if self.AirTimer:IsPastSimMS(250) then
		self.AirTimer:Reset();

		local Origin = {};
		if Owner.FGFoot then
			table.insert(Origin, Vector(Owner.FGFoot.Pos.X, Owner.FGFoot.Pos.Y) + Vector(0, 4));
		end
		if Owner.BGFoot then
			table.insert(Origin, Vector(Owner.BGFoot.Pos.X, Owner.BGFoot.Pos.Y) + Vector(0, 4));
		end
		if #Origin == 0 then
			table.insert(Origin, Vector(Owner.Pos.X, Owner.Pos.Y) + Vector(0, 4 + ToMOSprite(Owner):GetSpriteHeight() + Owner.SpriteOffset.Y));
		end
		for i = 1, #Origin do
			if SceneMan:GetTerrMatter(Origin[i].X, Origin[i].Y) ~= rte.airID then
				self.groundContact = 3;
				break;
			else
				self.groundContact = self.groundContact - 1;
			end
		end

		local newFlying = false;
		if not (Owner.FGLeg and Owner.BGLeg) then
			newFlying = true;
		end

		if self.groundContact < 0 then
			newFlying = true;
		end

		if self.flying ~= newFlying then
			Owner:SendMessage("AI_IsFlying", newFlying);
			self.flying = newFlying;
		end

		Owner:EquipShieldInBGArm(); -- try to equip a shield
	end

	-- look for targets
	local FoundMO, HitPoint = self.SpotTargets(self, Owner, self.skill);
	if FoundMO then
		--TODO: decide whether to attack based on the material strength of found MO
		if self.Behavior ~= nil and self.Target and MovableMan:ValidMO(self.Target) and FoundMO.ID == self.Target.ID then	-- found the same target
			self.OldTargetPos = Vector(self.Target.Pos.X, self.Target.Pos.Y);
			self.TargetOffset = SceneMan:ShortestDistance(self.Target.Pos, HitPoint, false);
			self.TargetLostTimer:Reset();
			self.ReloadTimer:Reset();
		elseif FoundMO.Team ~= Owner.Team then	-- found an enemy
			if FoundMO.ClassName == "AHuman" then
				FoundMO = ToAHuman(FoundMO);
			elseif FoundMO.ClassName == "ACrab" then
				FoundMO = ToACrab(FoundMO);
			elseif FoundMO.ClassName == "ACRocket" then
				FoundMO = ToACRocket(FoundMO);
			elseif FoundMO.ClassName == "ACDropShip" then
				FoundMO = ToACDropShip(FoundMO);
			elseif FoundMO.ClassName == "ADoor" and FoundMO.Team ~= Activity.NOTEAM and Owner.AIMode ~= Actor.AIMODE_SENTRY and ToADoor(FoundMO).Door and ToADoor(FoundMO).Door:IsAttached() then
				FoundMO = ToADoor(FoundMO);
			elseif FoundMO.ClassName == "Actor" then
				FoundMO = ToActor(FoundMO);
			else
				FoundMO = nil;
			end

			if FoundMO and FoundMO.Status < Actor.INACTIVE then
				if self.Target then
					-- check if this MO should be targeted instead
					if HumanBehaviors.CalculateThreatLevel(FoundMO, Owner) > HumanBehaviors.CalculateThreatLevel(self.Target, Owner) + 0.5 then
						self.OldTargetPos = Vector(self.Target.Pos.X, self.Target.Pos.Y);
						self.Target = FoundMO;
						self.TargetOffset = SceneMan:ShortestDistance(self.Target.Pos, HitPoint, false); -- this is the distance vector from the target center to the point we hit with our ray
						if self.NextBehaviorName ~= "ShootTarget" then
							self:CreateAttackBehavior(Owner);
						end
					end
				else
					self.OldTargetPos = nil;
					self.Target = FoundMO;
					self.TargetOffset = SceneMan:ShortestDistance(self.Target.Pos, HitPoint, false); -- this is the distance vector from the target center to the point we hit with our ray
					self:CreateAttackBehavior(Owner);
				end
			end
		end
	else -- no target found this frame
		if self.Target and self.TargetLostTimer:IsPastSimTimeLimit() then
			self.Target = nil; -- the target has been out of sight for too long, ignore it
			self:CreatePinBehavior(Owner); -- keep aiming in the direction of the target for a short time
		end

		if self.ReloadTimer:IsPastSimMS(8000) then	-- check if we need to reload
			if Owner.FirearmNeedsReload then
				Owner:ReloadFirearms();
				if not Owner.FirearmNeedsReload then -- account for BG weapon needing to reload separately
					self.ReloadTimer:Reset();
				end
			elseif not HumanBehaviors.EquipPreferredWeapon(self, Owner) then	-- make sure we equip a preferred or a primary weapon if we have one
				self.ReloadTimer:Reset();
			end
		end
	end

	self.squadShoot = false;
	if Owner.MOMoveTarget then
		-- make the last waypoint marker stick to the MO we are following
		if MovableMan:ValidMO(Owner.MOMoveTarget) then
			Owner:RemoveMovePathEnd();
			Owner:AddToMovePathEnd(Owner.MOMoveTarget.Pos);

			if Owner.AIMode == Actor.AIMODE_SQUAD then
				-- look where the SL looks, if not moving
				if not self.jump and self.lateralMoveState == Actor.LAT_STILL then
					local Leader = MovableMan:GetMOFromID(Owner:GetAIMOWaypointID());
					if Leader then
						if IsAHuman(Leader) then
							Leader = ToAHuman(Leader);
						elseif IsACrab(Leader) then
							Leader = ToACrab(Leader);
						else
							Leader = nil;
						end
					end

					if Leader then
						local dist = SceneMan:ShortestDistance(Owner.Pos, Leader.Pos, false).Largest;
						local radius = (Leader.Height + Owner.Height) * 0.5;
						if dist < radius then
							local copyControls = {Controller.MOVE_LEFT, Controller.MOVE_RIGHT, Controller.BODY_JUMPSTART, Controller.BODY_JUMP, Controller.BODY_CROUCH};
							for _, control in pairs(copyControls) do
								local state = Leader:GetController():IsState(control);
								self.Ctrl:SetState(control, state);
							end
							if Leader.EquippedItem then
								local aimDelta = SceneMan:ShortestDistance(Leader.Pos, Leader.ViewPoint, false);
								
								if IsHDFirearm(Leader.EquippedItem) then
									local aimCorrectionRatio = Leader.SharpAimProgress * (dist/radius);
									self.Ctrl.AnalogAim = (aimDelta * (1 - aimCorrectionRatio) + SceneMan:ShortestDistance(Owner.Pos, Leader.ViewPoint + aimDelta, false) * aimCorrectionRatio).Normalized;
									
									local LeaderWeapon = ToHDFirearm(Leader.EquippedItem);
									if LeaderWeapon:IsWeapon() then
										self.deviceState = AHuman.POINTING;

										-- check if the SL is shooting and if we have a similar weapon
										if Owner.FirearmIsReady then
											self.deviceState = AHuman.AIMING;

											if IsHDFirearm(Owner.EquippedItem) and Leader:GetController():IsState(Controller.WEAPON_FIRE) then
												local OwnerWeapon = ToHDFirearm(Owner.EquippedItem);
												if OwnerWeapon:IsTool() then
													-- try equipping a weapon
													if Owner.InventorySize > 0 and not Owner:EquipDeviceInGroup("Weapons - Primary", true) then
														Owner:EquipFirearm(true);
													end
												elseif LeaderWeapon:GetAIBlastRadius() >= OwnerWeapon:GetAIBlastRadius() * 0.5 and OwnerWeapon:CompareTrajectories(LeaderWeapon) < math.max(100, OwnerWeapon:GetAIBlastRadius()) then
													-- slightly displace full-auto shots to diminish stacking sounds and create a more dense fire rate
													if OwnerWeapon.FullAuto then
														if math.random() < 0.3 then
															self.Target = nil;
															self.squadShoot = true;
														end
													else
														self.Target = nil;
														self.squadShoot = true;
													end
												end
											else
												self.squadShoot = false;
											end
										else
											if Owner.FirearmIsEmpty then
												Owner:ReloadFirearms();
											elseif Owner.InventorySize > 0 and not Owner:EquipDeviceInGroup("Weapons - Primary", true) then
												Owner:EquipFirearm(true);
											end
										end
									end
								elseif IsThrownDevice(Leader.EquippedItem) and Leader:IsPlayerControlled() and ToThrownDevice(Leader.EquippedItem):HasObjectInGroup("Bombs - Grenades") and Owner:HasObjectInGroup("Bombs - Grenades") then
									-- throw grenades in unison with squad
									self.Ctrl.AnalogAim = aimDelta.Normalized;
									self.deviceState = AHuman.POINTING;

									if Leader:GetController():IsState(Controller.WEAPON_FIRE) then

										Owner:EquipDeviceInGroup("Bombs - Grenades", true);

										self.Target = nil;
										self.squadShoot = true;
									else
										self.squadShoot = false;
									end
								end
							end
						end
						if Leader.AIMode == Actor.AIMODE_GOTO then
							Owner.leaderWaypoint = Leader:GetLastAIWaypoint();
						end
					end
				end
			end
		else
			if self.GoToName == "GoToWpt" then
				self:CreateGoToBehavior(Owner);
			end

			-- if we are in AIMODE_SQUAD the leader just got killed
			if Owner.AIMode == Actor.AIMODE_SQUAD then
				Owner:ClearMovePath();
				if Owner.leaderWaypoint then
					Owner.AIMode = Actor.AIMODE_GOTO;
					Owner:AddAISceneWaypoint(Owner.leaderWaypoint);
					Owner.leaderWaypoint = nil;
				else
					Owner.AIMode = Actor.AIMODE_SENTRY;
				end
			end
		end
	elseif Owner.AIMode == Actor.AIMODE_SQUAD then	-- if we are in AIMODE_SQUAD the leader just got killed
		Owner.AIMode = Actor.AIMODE_SENTRY;
		if self.GoToName == "GoToWpt" then
			self:CreateGoToBehavior(Owner);
		end
	end

	if self.squadShoot then
		-- cycle semi-auto weapons on and off so the AI will shoot even if the player only press and hold the trigger
		if Owner.FirearmIsSemiAuto and self.SquadShootTimer:IsPastSimMS(Owner.FirearmActivationDelay+self.SquadShootDelay) then
			self.SquadShootTimer:Reset();
			self.squadShoot = false;
			self.squadShootDelay = math.random(50,100);
		end
	else
		-- run the move behavior and delete it if it returns true
		if self.GoToBehavior then
			local msg, done = coroutine.resume(self.GoToBehavior, self, Owner, false);
			if not msg then
				ConsoleMan:PrintString(Owner.PresetName .. " " .. self.GoToName .. " error:\n" .. done  .. debug.traceback(self.GoToBehavior)); -- print the error message
				done = true;
			end

			if done then
				self.GoToBehavior = nil;
				self.GoToName = nil;
				if self.GoToCleanup then
					self.GoToCleanup(self);
					self.GoToCleanup = nil;
				end
			end
		elseif self.flying then	-- avoid falling damage
			local jumpThreshold = 9;
			if Owner.Jetpack and Owner.Jetpack.JetpackType == AEJetpack.JumpPack then
				jumpThreshold = jumpThreshold * 3;
			end

			if (not self.jump and Owner.Vel.Y > jumpThreshold) or (self.jump and Owner.Vel.Y > jumpThreshold * 0.66) then
				self.jump = true;

				-- try falling straight down
				if not self.Target then
					if Owner.Vel.X > 2 then
						self.lateralMoveState = Actor.LAT_LEFT;
					elseif Owner.Vel.X < -2 then
						self.lateralMoveState = Actor.LAT_RIGHT;
					else
						self.lateralMoveState = Actor.LAT_STILL;
					end
				end
			else
				self.jump = false;
				self.lateralMoveState = Actor.LAT_STILL;
			end
		else
			self.jump = false;
		end

		-- run the selected behavior and delete it if it returns true
		if self.Behavior then
			local msg, done = coroutine.resume(self.Behavior, self, Owner, false);
			if not msg then
				ConsoleMan:PrintString(Owner.PresetName .. " behavior " .. self.BehaviorName .. " error:\n" .. done); -- print the error message
				done = true;
			end

			if done then
				self.Behavior = nil;
				self.BehaviorName = nil;
				if self.BehaviorCleanup then
					self.BehaviorCleanup(self);
					self.BehaviorCleanup = nil;
				end

				if not self.NextBehavior and not self.PickupHD and self.PickUpTimer:IsPastSimMS(10000) then
					self.PickUpTimer:Reset();

					if not Owner:EquipFirearm(false) then
						self:CreateGetWeaponBehavior(Owner);
					elseif Owner.AIMode ~= Actor.AIMODE_SENTRY and not Owner:EquipDiggingTool(false) then
						self:CreateGetToolBehavior(Owner);
					end
				end
			end
		end

		-- there is a HeldDevice we want to pick up
		if self.PickupHD then
			if not MovableMan:IsDevice(self.PickupHD) or self.PickupHD.ID ~= self.PickupHD.RootID then
				self.PickupHD = nil; -- the HeldDevice has been destroyed or picked up
			elseif SceneMan:ShortestDistance(Owner.Pos, self.PickupHD.Pos, false):MagnitudeIsLessThan(Owner.Height) then
				self.Ctrl:SetState(Controller.WEAPON_PICKUP, true);
			end
		end

		-- listen and react to AlarmEvents and AlarmPoints
		local AlarmPoint = Owner:GetAlarmPoint();
		if AlarmPoint.Largest > 0 then
			if not self.Target and not self.UnseenTarget then
				self.AlarmPos = Vector(AlarmPoint.X, AlarmPoint.Y);
				self:CreateFaceAlarmBehavior(Owner);
			else
				-- is the alarm generated from behind us?
				local AlarmVector = SceneMan:ShortestDistance(Owner.Pos, AlarmPoint, false);
				if (Owner.HFlipped and AlarmVector.X > 0) or (not Owner.HFlipped and AlarmVector.X < 0) then
					self.AlarmPos = Vector(AlarmPoint.X, AlarmPoint.Y);
					self:CreateFaceAlarmBehavior(Owner);
				end
			end
		elseif not (self.Target or self.UnseenTarget) then
			-- use medikit if not engaging enemy
			if Owner.Health < (Owner.MaxHealth * 0.5) then
				if Owner:HasObject("Medikit") then

					self.useMedikit = Owner:EquipNamedDevice("Medikit", true);
				else
					self.useMedikit = false;
					if not self.isPlayerOwned and Owner.AIMode == Actor.AIMODE_SENTRY then
						Owner.AIMode = Actor.AIMODE_PATROL;
					end
				end
			else
				if self.useMedikit == true then
					self.useMedikit = false;
					Owner:EquipFirearm(true);
				end
				if self.AlarmTimer:IsPastSimTimeLimit() and HumanBehaviors.ProcessAlarmEvent(self, Owner) then
					self.AlarmTimer:Reset();
				end
			end
		end
	end

	if self.teamBlockState == Actor.IGNORINGBLOCK then
		if self.BlockedTimer:IsPastSimMS(10000) then
			self.teamBlockState = Actor.NOTBLOCKED;
		end
	elseif self.teamBlockState == Actor.BLOCKED then	-- we are blocked by a team-mate, stop
		self.lateralMoveState = Actor.LAT_STILL;
		self.jump = false;
		if self.BlockedTimer:IsPastSimMS(20000) then
			self.BlockedTimer:Reset();
			self.teamBlockState = Actor.IGNORINGBLOCK;
		end
	else
		self.BlockedTimer:Reset();
	end

	-- controller states
	if self.squadShoot then
		self.Ctrl:SetState(Controller.WEAPON_FIRE, (self.fire or self.squadShoot));
	else
		self.Ctrl:SetState(Controller.WEAPON_FIRE, (self.fire or self.useMedikit));
	end

	if self.deviceState == AHuman.AIMING then
		self.Ctrl:SetState(Controller.AIM_SHARP, true);
	end
	-- force jetpack at detrimental downwards velocity
	if (not self.jump and Owner.Vel.Y > 18) then
		self.jump = true;
	end
	if self.jump and Owner.Jetpack and Owner.Jetpack.JetTimeLeft > TimerMan.AIDeltaTimeMS then
		if self.jumpState == AHuman.PREJUMP then
			self.jumpState = AHuman.UPJUMP;
		elseif self.jumpState ~= AHuman.UPJUMP then	-- the jetpack is off
			self.jumpState = AHuman.PREJUMP;
		end
	else
		self.jumpState = AHuman.NOTJUMPING;
	end

	if Owner.Jetpack then
		if self.jumpState == AHuman.PREJUMP then
			self.Ctrl:SetState(Controller.BODY_JUMPSTART, true); -- try to trigger a burst
		elseif self.jumpState == AHuman.UPJUMP then
			self.Ctrl:SetState(Controller.BODY_JUMP, true); -- trigger normal jetpack emission
		end
	end

	if self.proneState == AHuman.GOPRONE then
		self.proneState = AHuman.PRONE;
	elseif self.proneState == AHuman.PRONE then
		self.Ctrl:SetState(Controller.BODY_CROUCH, true);
	end

	if self.lateralMoveState == Actor.LAT_LEFT then
		self.Ctrl:SetState(Controller.MOVE_LEFT, true);
	elseif self.lateralMoveState == Actor.LAT_RIGHT then
		self.Ctrl:SetState(Controller.MOVE_RIGHT, true);
	end
end

function NativeHumanAI:Destroy(Owner)
	-- Tell the coroutines to abort to avoid memory leaks
	if self.GoToBehavior then
		local msg, done = coroutine.resume(self.GoToBehavior, self, Owner, true);
	end

	if self.Behavior then
		local msg, done = coroutine.resume(self.Behavior, self, Owner, true);
	end
end

-- functions that create behaviors. the default behaviors are stored in the HumanBehaviors table. store your custom behaviors in a table to avoid name conflicts between mods.
function NativeHumanAI:CreateQuickthrowBehavior(Owner)
	if self.Target and MovableMan:ValidMO(self.Target) then
		if Owner:EquipThrowable(true) and Owner.ThrowableIsReady then
			self.NextBehavior = coroutine.create(HumanBehaviors.ThrowTarget);
			self.NextBehaviorName = "ThrowTarget";
			return true;
		end
	end
	return false;
end

function NativeHumanAI:CreateSentryBehavior(Owner)
	if self.Target then
		self:CreateAttackBehavior(Owner);
	else
		if not Owner:EquipFirearm(true) then
			if self.PickUpTimer:IsPastSimMS(2000) then
				self.PickUpTimer:Reset();
				self:CreateGetWeaponBehavior(Owner);
			end

			return;
		end

		self.NextBehavior = coroutine.create(HumanBehaviors.Sentry); -- replace "HumanBehaviors.Sentry" with the function name of your own sentry behavior
		self.NextCleanup = nil;
		self.NextBehaviorName = "Sentry";
	end
end

function NativeHumanAI:CreatePatrolBehavior(Owner)
	self.NextBehavior = coroutine.create(HumanBehaviors.Patrol);
	self.NextCleanup = nil;
	self.NextBehaviorName = "Patrol";
end

function NativeHumanAI:CreateGoldDigBehavior(Owner)
	if not Owner:EquipDiggingTool(false) then
		if self.PickUpTimer:IsPastSimMS(1000) then
			self.PickUpTimer:Reset();
			self:CreateGetToolBehavior(Owner);
		end

		return;
	end

	self.NextBehavior = coroutine.create(HumanBehaviors.GoldDig);
	self.NextCleanup = nil;
	self.NextBehaviorName = "GoldDig";
end

function NativeHumanAI:CreateBrainSearchBehavior(Owner)
	self.NextBehavior = coroutine.create(HumanBehaviors.BrainSearch);
	self.NextCleanup = nil;
	self.NextBehaviorName = "BrainSearch";
end

function NativeHumanAI:CreateGetToolBehavior(Owner)
	if Owner.AIMode ~= Actor.AIMODE_SQUAD then
		self.NextBehavior = coroutine.create(HumanBehaviors.ToolSearch);
		self.NextCleanup = nil;
		self.NextBehaviorName = "ToolSearch";
	end
end

function NativeHumanAI:CreateGetWeaponBehavior(Owner)
	if Owner.AIMode ~= Actor.AIMODE_SQUAD then
		self.NextBehavior = coroutine.create(HumanBehaviors.WeaponSearch);
		self.NextCleanup = nil;
		self.NextBehaviorName = "WeaponSearch";
	end
end

function NativeHumanAI:CreateGoToBehavior(Owner)
	self.NextGoTo = coroutine.create(HumanBehaviors.GoToWpt);
	self.NextGoToCleanup = function(AI)
		AI.lateralMoveState = Actor.LAT_STILL;
		AI.deviceState = AHuman.STILL;
		AI.proneState = AHuman.NOTPRONE;
		AI.jump = false;
		AI.fire = false;
	end
	self.NextGoToName = "GoToWpt";
end

function NativeHumanAI:CreateAttackBehavior(Owner)
	self.ReloadTimer:Reset();
	self.TargetLostTimer:Reset();

	local dist = SceneMan:ShortestDistance(Owner.Pos, self.Target.Pos, false);

	if IsADoor(self.Target) and Owner.AIMode ~= Actor.AIMODE_SQUAD then
		--TODO: Include other explosive weapons with varying effective ranges!
		if Owner:EquipDeviceInGroup("Tools - Breaching", true) then
			self.NextBehavior = coroutine.create(HumanBehaviors.AttackTarget);
			self.NextBehaviorName = "AttackTarget";
		elseif Owner.FirearmIsReady and HumanBehaviors.GetProjectileData(Owner).pen * 0.9 > (self.Target.Door or self.Target).Material.StructuralIntegrity then
			self.NextBehavior = coroutine.create(HumanBehaviors.ShootTarget);
			self.NextBehaviorName = "ShootTarget";
		else	--Cannot harm this door!
			self.Target = nil;
			return;
		end
	-- favor grenades as the initiator to a sneak attack
	elseif Owner.AIMode ~= Actor.AIMODE_SQUAD and Owner.AIMode ~= Actor.AIMODE_SENTRY and self.Target.HFlipped == Owner.HFlipped and Owner:EquipDeviceInGroup("Bombs - Grenades", true)
	and dist:MagnitudeIsGreaterThan(100) and dist:MagnitudeIsLessThan(ToThrownDevice(Owner.EquippedItem):GetCalculatedMaxThrowVelIncludingArmThrowStrength() * GetPPM()) and (self.Target.Pos.Y + 20) > Owner.Pos.Y then
		self.NextBehavior = coroutine.create(HumanBehaviors.ThrowTarget);
		self.NextBehaviorName = "ThrowTarget";
	elseif Owner:EquipFirearm(true) then
		if Owner.EquippedItem:HasObjectInGroup("Weapons - Melee") then
			self.NextBehavior = coroutine.create(HumanBehaviors.AttackTarget);
			self.NextBehaviorName = "AttackTarget";
		else
			self.NextBehavior = coroutine.create(HumanBehaviors.ShootTarget);
			self.NextBehaviorName = "ShootTarget";
		end
	elseif Owner.AIMode ~= Actor.AIMODE_SQUAD and Owner:EquipThrowable(true) and dist:MagnitudeIsLessThan(ToThrownDevice(Owner.EquippedItem):GetCalculatedMaxThrowVelIncludingArmThrowStrength() * GetPPM()) then
		self.NextBehavior = coroutine.create(HumanBehaviors.ThrowTarget);
		self.NextBehaviorName = "ThrowTarget";
	elseif Owner.AIMode ~= Actor.AIMODE_SQUAD and Owner:EquipDiggingTool(true) and dist:MagnitudeIsLessThan(250) then
		self.NextBehavior = coroutine.create(HumanBehaviors.AttackTarget);
		self.NextBehaviorName = "AttackTarget";
	else	-- unarmed or far away
		if self.PickUpTimer:IsPastSimMS(2500) then
			self.PickUpTimer:Reset();
			self.NextBehavior = coroutine.create(HumanBehaviors.WeaponSearch);
			self.NextBehaviorName = "WeaponSearch";
			self.NextCleanup = nil;

			return;
		else -- there are probably no weapons around here (in the vicinity of an area adjacent to a location)
			if not (self.isPlayerOwned and Owner.AIMode == Actor.AIMODE_SENTRY) and (self.Target.ClassName == "AHuman" or self.Target.ClassName == "ACrab") then
				self.NextBehavior = coroutine.create(HumanBehaviors.AttackTarget);
				self.NextBehaviorName = "AttackTarget";
			else
				self.Target = nil;
				return;
			end
		end
	end

	self.NextCleanup = function(AI)
		AI.fire = false;
		AI.canHitTarget = false;
		AI.deviceState = AHuman.STILL;
		AI.proneState = AHuman.NOTPRONE;
		AI.TargetLostTimer:SetSimTimeLimitMS(2000);
	end
end

-- force the use of a digger when attacking
function NativeHumanAI:CreateHtHBehavior(Owner)					-- has to be digger, not just "tool"
	if Owner.AIMode ~= Actor.AIMODE_SQUAD and self.Target and (Owner:HasObjectInGroup("Tools - Diggers") or Owner:HasObjectInGroup("Weapons - Melee")) then
		self.NextBehavior = coroutine.create(HumanBehaviors.AttackTarget);
		self.NextBehaviorName = "AttackTarget";
		self.NextCleanup = function(AI)
			AI.fire = false;
			AI.Target = nil;
			AI.deviceState = AHuman.STILL;
			AI.proneState = AHuman.NOTPRONE;
		end
	end
end

function NativeHumanAI:CreateSuppressBehavior(Owner)
	if Owner:EquipFirearm(true) then
		self.NextBehavior = coroutine.create(HumanBehaviors.ShootArea);
		self.NextBehaviorName = "ShootArea";
	else
		if Owner.FirearmIsEmpty then
			Owner:ReloadFirearms();
		end
		return;
	end

	self.NextCleanup = function(AI)
		AI.fire = false;
		AI.UnseenTarget = nil;
		AI.deviceState = AHuman.STILL;
		AI.proneState = AHuman.NOTPRONE;
	end
end

function NativeHumanAI:CreateFaceAlarmBehavior(Owner)
	self.NextBehavior = coroutine.create(HumanBehaviors.FaceAlarm);
	self.NextBehaviorName = "FaceAlarm";
	self.NextCleanup = nil;
end

function NativeHumanAI:CreatePinBehavior(Owner)
	if self.OldTargetPos and Owner:EquipFirearm(true) then
		self.NextBehavior = coroutine.create(HumanBehaviors.PinArea);
		self.NextBehaviorName = "PinArea";
	else
		return;
	end

	self.NextCleanup = function(AI)
		self.OldTargetPos = nil;
	end
end
