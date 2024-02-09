require("Constants")
require("AI/HumanBehaviors");
require("AI/CrabBehaviors");
require("AI/TurretBehaviors");

NativeTurretAI = {};

function NativeTurretAI:Create(Owner)
	local Members = {};

	Members.lateralMoveState = Actor.LAT_STILL;
	Members.deviceState = ACrab.STILL;
	Members.lastAIMode = Actor.AIMODE_NONE;
	Members.SentryFacing = Owner.HFlipped;
	Members.fire = false;

	Members.ReloadTimer = Timer();
	Members.TargetLostTimer = Timer();

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

function NativeTurretAI:Update(Owner)
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
			self.fire = false;
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
		if Owner.AIMode == Actor.AIMODE_PATROL then
			self:CreatePatrolBehavior(Owner);
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

	if Owner.AIMode == Actor.AIMODE_SQUAD then
		if Owner.MOMoveTarget then
			-- make the last waypoint marker stick to the MO we are following
			if MovableMan:ValidMO(Owner.MOMoveTarget) then
				Owner:RemoveMovePathEnd();
				Owner:AddToMovePathEnd(Owner.MOMoveTarget.Pos);

				-- look where the SL looks, if not moving
				if not self.Target and self.lateralMoveState == Actor.LAT_STILL then
					local Leader = ToActor(Owner.MOMoveTarget);
					if SceneMan:ShortestDistance(Owner.Pos, Leader.Pos, false).Largest < Owner.Height then
						self.deviceState = ACrab.POINTING;
						--self.deviceState = ACrab.AIMING;
						self.Ctrl.AnalogAim = SceneMan:ShortestDistance(Owner.EyePos, Leader.ViewPoint, false).Normalized;
					end
				end
			else	-- the leader just got killed
				Owner.MOMoveTarget = nil;
				Owner.AIMode = Actor.AIMODE_SENTRY;
				Owner:ClearMovePath();
			end
		else	-- the leader just got killed
			Owner.AIMode = Actor.AIMODE_SENTRY;
			Owner:ClearMovePath();
		end
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

	-- controller states
	if self.fire then
		self.Ctrl:SetState(Controller.WEAPON_FIRE, true);
	end

	if self.deviceState == ACrab.AIMING then
		self.Ctrl:SetState(Controller.AIM_SHARP, true);
	end
end

function NativeTurretAI:Destroy(Owner)
	-- Tell the coroutines to abort to avoid memory leaks
	if self.Behavior then
		local msg, done = coroutine.resume(self.Behavior, self, Owner, true);
	end
end


function NativeTurretAI:CreatePatrolBehavior(Owner)
	self.NextBehavior = coroutine.create(TurretBehaviors.Patrol);
	self.NextCleanup = nil;
	self.NextBehaviorName = "Patrol";
end

function NativeTurretAI:CreateSentryBehavior(Owner)
	if not Owner.FirearmIsReady and not Owner.ThrowableIsReady then
		return;
	end

	self.NextBehavior = coroutine.create(CrabBehaviors.Sentry);
	self.NextCleanup = nil;
	self.NextBehaviorName = "Sentry";
end

function NativeTurretAI:CreateAttackBehavior(Owner)
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

function NativeTurretAI:CreateSuppressBehavior(Owner)
	if Owner.FirearmIsReady then
		self.NextBehavior = coroutine.create(CrabBehaviors.ShootArea);
		self.NextBehaviorName = "ShootArea";
	else
		if Owner.FirearmIsEmpty then
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

function NativeTurretAI:CreateFaceAlarmBehavior(Owner)
	self.NextBehavior = coroutine.create(HumanBehaviors.FaceAlarm);
	self.NextBehaviorName = "FaceAlarm";
	self.NextCleanup = nil;
end

function NativeTurretAI:CreatePinBehavior(Owner)
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
