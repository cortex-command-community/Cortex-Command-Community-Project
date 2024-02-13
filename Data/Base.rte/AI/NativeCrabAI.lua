require("Constants")
require("AI/HumanBehaviors");
require("AI/CrabBehaviors");

NativeCrabAI = {};

function NativeCrabAI:Create(Owner)
	local Members = {};

	Members.lateralMoveState = Actor.LAT_STILL;
	Members.deviceState = ACrab.STILL;
	Members.lastAIMode = Actor.AIMODE_NONE;
	Members.teamBlockState = Actor.NOTBLOCKED;
	Members.SentryFacing = Owner.HFlipped;
	Members.fire = false;

	Members.ReloadTimer = Timer();
	Members.BlockedTimer = Timer();
	Members.TargetLostTimer = Timer();
	Members.SquadShootTimer = Timer();

	Members.AlarmTimer = Timer();
	Members.AlarmTimer:SetSimTimeLimitMS(500);

	-- check if this team is controlled by a human
	if ActivityMan:GetActivity():IsHumanTeam(Owner.Team) then
		Members.isPlayerOwned = true;
		Members.PlayerInterferedTimer = Timer();
		Members.PlayerInterferedTimer:SetSimTimeLimitMS(500);
	end
	
	-- customizable variables, trend started by pawnis on 01/09/2023 :)
	
	-- humble beginnings
	-- pause time between sweeping aim up and down when guarding
	Members.idleAimTime = Owner:NumberValueExists("AIIdleAimTime") and Owner:GetNumberValue("AIIdleAimTime") or 500;

	-- set shooting skill
	Members.aimSpeed, Members.aimSkill = HumanBehaviors.GetTeamShootingSkill(Owner.Team);

	setmetatable(Members, self);
	self.__index = self;
	return Members;
end

function NativeCrabAI:Update(Owner)
	self.Ctrl = Owner:GetController();

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

			self.Target = nil;
			self.UnseenTarget = nil;
			self.OldTargetPos = nil;
			self.fire = false;
			self.SentryFacing = Owner.HFlipped;
			self.lastAIMode = Actor.AIMODE_NONE;
		end

		self.PlayerInterferedTimer:Reset();
	end

	if self.Target and not MovableMan:ValidMO(self.Target) then
		self.Target = nil;
	end

	if self.UnseenTarget and not MovableMan:ValidMO(self.UnseenTarget) then
		self.UnseenTarget = nil;
	end

	-- switch to the next behavior, if avaliable
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
	end

	-- check if the AI mode has changed or if we need a new behavior
	if Owner.AIMode ~= self.lastAIMode or not self.Behavior then
		-- Tell the coroutines to abort to avoid memory leaks
		if self.Behavior then
			local msg, done = coroutine.resume(self.Behavior, self, Owner, true);
		end

		self.Behavior = nil;
		self.BehaviorName = nil;
		if self.BehaviorCleanup then
			self.BehaviorCleanup(self); -- stop the current behavior
			self.BehaviorCleanup = nil;
		end

		-- select a new behavior based on AI mode
		if Owner.AIMode == Actor.AIMODE_GOTO or Owner.AIMode == Actor.AIMODE_SQUAD then
			self:CreateGoToBehavior();
		elseif Owner.AIMode == Actor.AIMODE_PATROL then
			self:CreatePatrolBehavior(Owner);
		elseif Owner.AIMode == Actor.AIMODE_BRAINHUNT then
			self:CreateBrainSearchBehavior(Owner);
		else
			if Owner.AIMode ~= self.lastAIMode and Owner.AIMode == Actor.AIMODE_SENTRY then
				self.SentryFacing = Owner.HFlipped; -- store the direction in which we should be looking
			end

			self:CreateSentryBehavior(Owner);
		end

		self.lastAIMode = Owner.AIMode;
	end

	-- cast a ray to find targets
	CrabBehaviors.LookForTargets(self, Owner);

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

					if Leader and Leader.EquippedItem and IsHDFirearm(Leader.EquippedItem) and SceneMan:ShortestDistance(Owner.Pos, Leader.Pos, false).Largest < (Leader.Height + Owner.Height) * 0.5 then
						local LeaderWeapon = ToHDFirearm(Leader.EquippedItem);
						if LeaderWeapon:IsWeapon() then
							local AimDelta = SceneMan:ShortestDistance(Leader.Pos, Leader.ViewPoint, false);
							self.Ctrl.AnalogAim = SceneMan:ShortestDistance(Owner.Pos, Leader.ViewPoint+AimDelta, false).Normalized;
							self.deviceState = AHuman.POINTING;

							-- check if the SL is shooting and if we have a similar weapon
							if Owner.FirearmIsReady then
								self.deviceState = AHuman.AIMING;

								if IsHDFirearm(Owner.EquippedItem) and Leader:GetController():IsState(Controller.WEAPON_FIRE) then
									local OwnerWeapon = ToHDFirearm(Owner.EquippedItem);
									if not OwnerWeapon:IsTool() and LeaderWeapon:GetAIBlastRadius() >= OwnerWeapon:GetAIBlastRadius() * 0.5 and OwnerWeapon:CompareTrajectories(LeaderWeapon) < math.max(100, OwnerWeapon:GetAIBlastRadius()) then
										self.Target = nil;
										self.squadShoot = true;
									end
								end
							else
								if Owner.FirearmIsEmpty then
									Owner:ReloadFirearms();
								end
							end
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
				Owner.AIMode = Actor.AIMODE_SENTRY;
				Owner:ClearMovePath();
			end
		end
	elseif Owner.AIMode == Actor.AIMODE_SQUAD then	-- if we are in AIMODE_SQUAD the leader just got killed
		Owner.AIMode = Actor.AIMODE_SENTRY;
		if self.GoToName == "GoToWpt" then
			self:CreateGoToBehavior(Owner);
		end
	end
	--[[if Owner.MOMoveTarget then
		-- make the last waypoint marker stick to the MO we are following
		if MovableMan:ValidMO(Owner.MOMoveTarget) then
			Owner:RemoveMovePathEnd();
			Owner:AddToMovePathEnd(Owner.MOMoveTarget.Pos);

			if Owner.AIMode == Actor.AIMODE_SQUAD then
				-- look where the SL looks, if not moving
				if not self.Target and self.lateralMoveState == Actor.LAT_STILL then
					local Leader = ToActor(Owner.MOMoveTarget);
					if Leader and SceneMan:ShortestDistance(Owner.Pos, Leader.Pos, false).Largest < Owner.Height then
						self.deviceState = AHuman.AIMING;
						self.Ctrl.AnalogAim = SceneMan:ShortestDistance(Owner.EyePos, Leader.ViewPoint, false).Normalized;
					end
				end
			end
		else
			Owner.MOMoveTarget = nil;

			-- if we are in AIMODE_SQUAD the leader just got killed
			if Owner.AIMode == Actor.AIMODE_SQUAD then
				Owner.AIMode = Actor.AIMODE_SENTRY;
				Owner:ClearMovePath();
			end
		end
	elseif Owner.AIMode == Actor.AIMODE_SQUAD then	-- if we are in AIMODE_SQUAD the leader just got killed
		Owner.AIMode = Actor.AIMODE_SENTRY;
		Owner:ClearMovePath();
	end]]

	if self.squadShoot then
		-- cycle semi-auto weapons on and off so the AI will shoot even if the player only press and hold the trigger
		if Owner.FirearmIsSemiAuto and self.SquadShootTimer:IsPastSimMS(Owner.FirearmActivationDelay+50) then
			self.SquadShootTimer:Reset();
			self.squadShoot = false;
		end
	else
		-- run the selected behavior and delete it if it returns true
		if self.Behavior then
			local msg, done = coroutine.resume(self.Behavior, self, Owner, false);
			if not msg then
				ConsoleMan:PrintString(Owner.PresetName .. " behavior " .. self.BehaviorName .. " error:\n" .. done .. debug.traceback(self.Behavior)); -- print the error message
				done = true;
			end

			if done then
				self.Behavior = nil;
				self.BehaviorName = nil;
				if self.BehaviorCleanup then
					self.BehaviorCleanup(self);
					self.BehaviorCleanup = nil;
				end
			end
		end

		-- listen and react to relevant AlarmEvents
		if not self.Target and not self.UnseenTarget then
			if self.AlarmTimer:IsPastSimTimeLimit() and HumanBehaviors.ProcessAlarmEvent(self, Owner) then
				self.AlarmTimer:Reset();
			end
		end

		if self.teamBlockState == Actor.IGNORINGBLOCK then
			if self.BlockedTimer:IsPastSimMS(20000) then
				self.teamBlockState = Actor.NOTBLOCKED;
			end
		elseif self.teamBlockState == Actor.BLOCKED then	-- we are blocked by a teammate, stop
			self.lateralMoveState = Actor.LAT_STILL;
			if self.BlockedTimer:IsPastSimMS(10000) then
				self.BlockedTimer:Reset();
				self.teamBlockState = Actor.IGNORINGBLOCK;
			end
		else
			self.BlockedTimer:Reset();
		end
	end

	-- controller states
	self.Ctrl:SetState(Controller.WEAPON_FIRE, self.fire or self.squadShoot);

	if self.deviceState == ACrab.AIMING then
		self.Ctrl:SetState(Controller.AIM_SHARP, true);
	end

	if self.lateralMoveState == Actor.LAT_LEFT then
		self.Ctrl:SetState(Controller.MOVE_LEFT, true);
	elseif self.lateralMoveState == Actor.LAT_RIGHT then
		self.Ctrl:SetState(Controller.MOVE_RIGHT, true);
	end
end

function NativeCrabAI:Destroy(Owner)
	-- Tell the coroutines to abort to avoid memory leaks
	if self.Behavior then
		local msg, done = coroutine.resume(self.Behavior, self, Owner, true);
	end
end

function NativeCrabAI:CreatePatrolBehavior(Owner)
	self.NextBehavior = coroutine.create(HumanBehaviors.Patrol);
	self.NextCleanup = nil;
	self.NextBehaviorName = "Patrol";
end

function NativeCrabAI:CreateSentryBehavior(Owner)
	if not Owner.FirearmIsReady then
		return;
	end

	self.NextBehavior = coroutine.create(CrabBehaviors.Sentry);
	self.NextCleanup = nil;
	self.NextBehaviorName = "Sentry";
end

function NativeCrabAI:CreateAttackBehavior(Owner)
	if Owner.FirearmIsReady then
		self.NextBehavior = coroutine.create(CrabBehaviors.ShootTarget);
		self.NextBehaviorName = "ShootTarget";
	else
		if Owner.FirearmIsEmpty then
			Owner:ReloadFirearms();
		end
		return;
	end

	self.NextCleanup = function(AI)
		AI.fire = false;
		AI.Target = nil;
		AI.deviceState = ACrab.STILL;
	end
end

function NativeCrabAI:CreateSuppressBehavior(Owner)
	if Owner.FirearmIsReady then
		self.NextBehavior = coroutine.create(CrabBehaviors.ShootArea);
		self.NextBehaviorName = "ShootArea";
	else
		if self.FirearmIsEmpty then
			self:ReloadFirearms();
		end
		return;
	end

	self.NextCleanup = function(AI)
		AI.fire = false;
		AI.UnseenTarget = nil;
		AI.deviceState = ACrab.STILL;
	end
end

function NativeCrabAI:CreateGoToBehavior(Owner)
	self.NextBehavior = coroutine.create(CrabBehaviors.GoToWpt);
	self.NextBehaviorName = "GoToWpt";

	self.NextCleanup = function(AI)
		AI.lateralMoveState = Actor.LAT_STILL;
	end
end

function NativeCrabAI:CreateBrainSearchBehavior(Owner)
	self.NextBehavior = coroutine.create(HumanBehaviors.BrainSearch);
	self.NextCleanup = nil;
	self.NextBehaviorName = "BrainSearch";
end

function NativeCrabAI:CreateFaceAlarmBehavior(Owner)
	self.NextBehavior = coroutine.create(HumanBehaviors.FaceAlarm);
	self.NextBehaviorName = "FaceAlarm";
	self.NextCleanup = nil;
end

function NativeCrabAI:CreatePinBehavior(Owner)
	if self.OldTargetPos and Owner.FirearmIsReady then
		self.NextBehavior = coroutine.create(HumanBehaviors.PinArea);
		self.NextBehaviorName = "PinArea";
	else
		return;
	end

	self.NextCleanup = function(AI)
		self.OldTargetPos = nil;
	end
end
