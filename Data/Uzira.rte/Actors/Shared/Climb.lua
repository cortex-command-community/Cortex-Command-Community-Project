function Create(self)
	self.width = self:GetSpriteWidth();
	self.grabbedMO = nil;
	self.climbingWall = false;
end

function ThreadedUpdate(self)
	self.grabbedMO = nil;
	self.climbingWall = false;

	local controller = self:GetController();
	if self.Status == Actor.STABLE and not (controller:IsState(Controller.BODY_JUMP) or controller:IsState(Controller.BODY_CROUCH)) then	
		for _, foot in pairs ({self.FGFoot, self.BGFoot}) do
			if foot then
				local lowerCheck = SceneMan:CastMORay(foot.Pos, Vector(0, 1), self.ID, -2, rte.airID, false, 1);
				if lowerCheck ~= rte.NoMOID then
					local mo = MovableMan:GetMOFromID(lowerCheck):GetRootParent();
					if mo and mo.Team == self.Team and IsActor(mo) then
						self.grabbedMO = ToActor(mo);
						self:RequestSyncedUpdate();
						break;
					end
				end
			end
		end

		local controller = self:GetController();
		local dir = 0;
		if controller:IsState(Controller.MOVE_RIGHT) then
			dir = 1;
		elseif controller:IsState(Controller.MOVE_LEFT) then
			dir = -1
		end

		if self.grabbedMO == nil and dir ~= 0 and SceneMan:CastStrengthRay(self.Pos, Vector(self.width * self.FlipFactor, 0), 10, Vector(), 1, rte.airID, SceneMan.SceneWrapsX) then
			self.climbingWall = true;
			self:RequestSyncedUpdate();
		end
	end
end

function SyncedUpdate(self)
	if self.grabbedMO then
		self.grabbedMO:AddForce(self.Vel * self.Mass, Vector());
		--If the ID of the grabbed MO is lower than this actor's, it will have its forces applied to it before this, so halve the anti-gravitational force
		if self.grabbedMO.ID < self.ID then
			self.Vel = (self.Vel + self.grabbedMO.Vel - SceneMan.GlobalAcc * TimerMan.DeltaTimeSecs) * 0.5;
		else
			self.Vel = (self.Vel + self.grabbedMO.Vel) * 0.5 - SceneMan.GlobalAcc * TimerMan.DeltaTimeSecs;
		end
		self.AngularVel = (self.AngularVel - self.grabbedMO.AngularVel) * 0.5;
	end
	
	local controller = self:GetController();
	local dir = 0;
	if controller:IsState(Controller.MOVE_RIGHT) then
		dir = 1;
	elseif controller:IsState(Controller.MOVE_LEFT) then
		dir = -1
	end

	if dir ~= 0 then
		if self.grabbedMO then
			self.Vel = self.Vel * 0.5 + Vector(dir, -1);
		elseif self.climbingWall then
			self.Vel = Vector(self.Vel.X * 0.75, self.Vel.X * 0.5 - 1);
		end
	end
end