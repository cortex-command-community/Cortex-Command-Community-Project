function Create(self)
	self.healTimer = Timer();
	self.baseHealDelay = 150;
	self.healIncrementPerTarget = 100;
	self.healIncrementPerWound = 50;
	self.healTimer:SetSimTimeLimitMS(self.baseHealDelay);
	self.crossTimer = Timer();
	self.crossTimer:SetSimTimeLimitMS(800);

	self.visual = {};
	self.visual.Colors = {135, 133, 149, 148, 145, 148, 149, 133};
	self.visual.CurrentColor = 0;
	self.visual.Rotation = 0;
	self.visual.RPM = 60;
	self.visual.ArcCount = 2;

	self.maxHealRange = 100 + self.IndividualRadius;
	self.healStrength = 1;
	self.healTargets = {};
end

function Update(self)
	local parent = self:GetParent();
	if parent and IsActor(parent) then
		parent = ToActor(parent);
		--Visualize heal range
		local healRange = self.maxHealRange * (1 - (self.WoundCount/self.GibWoundLimit));
		if (parent:IsPlayerControlled() and parent:GetController():IsState(Controller.WEAPON_FIRE)) then
			local screen = ActivityMan:GetActivity():ScreenOfPlayer(parent:GetController().Player);
			if screen ~= -1 then
				self.visual.Rotation = self.visual.Rotation - self.visual.RPM/(TimerMan.DeltaTimeMS * 0.5);
				local color = self.visual.Colors[self.visual.CurrentColor];
				local angleSize = 180/self.visual.ArcCount;
				for i = 0, self.visual.ArcCount - 1 do
					local arcThin = i * 360/self.visual.ArcCount + self.visual.Rotation;
					local arcThick = arcThin + angleSize * 0.1;
					PrimitiveMan:DrawArcPrimitive(self.Pos, arcThick, arcThick + angleSize * 0.8, healRange, color, 2);
					PrimitiveMan:DrawArcPrimitive(self.Pos, arcThin, arcThin + angleSize, healRange, color, 1);
				end
			end
		end
		if self.healTimer:IsPastSimTimeLimit() then
			self.visual.CurrentColor = self.visual.CurrentColor % #self.visual.Colors + 1;
			self.healTimer:Reset();
			for _, healTarget in pairs(self.healTargets) do
				if healTarget and IsActor(healTarget) and (healTarget.Health < healTarget.MaxHealth or healTarget.WoundCount > 0) and healTarget.Vel.Largest < 10 then
					local trace = SceneMan:ShortestDistance(self.Pos, healTarget.Pos, false);
					if trace:MagnitudeIsLessThan(healRange + healTarget.Radius) and SceneMan:CastObstacleRay(self.Pos, trace, Vector(), Vector(), parent.ID, parent.IgnoresWhichTeam, rte.grassID, 5) < 0 then
						healTarget.Health = math.min(healTarget.Health + self.healStrength, healTarget.MaxHealth);
						if self.crossTimer:IsPastSimTimeLimit() then
							local cross = CreateMOSParticle("Particle Heal Effect", "Base.rte");
							if cross then
								cross.Pos = healTarget.AboveHUDPos + Vector(0, 4);
								MovableMan:AddParticle(cross);
							end
							healTarget:RemoveWounds(self.healStrength);
						end
					end
				end
			end
			if self.crossTimer:IsPastSimTimeLimit() then
				self.crossTimer:Reset();
			end
			self.healTargets = {};
			for actor in MovableMan.Actors do
				if actor.Team == parent.Team and actor.ID ~= parent.ID and (actor.Health < actor.MaxHealth or actor.WoundCount > 0) and actor.Vel.Largest < 5 then
					local trace = SceneMan:ShortestDistance(self.Pos, actor.Pos, false);
					if trace:MagnitudeIsLessThan(healRange * 0.9 + actor.Radius) then
						if SceneMan:CastObstacleRay(self.Pos, trace, Vector(), Vector(), parent.ID, parent.IgnoresWhichTeam, rte.airID, 3) < 0 then
							table.insert(self.healTargets, actor);
						end
					end
				end
			end
			self.healTimer:SetSimTimeLimitMS(self.baseHealDelay + (self.healIncrementPerWound * self.WoundCount) + (#self.healTargets * self.healIncrementPerTarget));
		end
	end
end