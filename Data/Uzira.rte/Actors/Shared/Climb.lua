function Create(self)
	self.width = self:GetSpriteWidth();
end

function Update(self)
	local controller = self:GetController();
	if self.Status == Actor.STABLE and not (controller:IsState(Controller.BODY_JUMP) or controller:IsState(Controller.BODY_CROUCH)) then
		local grabbedMO;
		
		for _, foot in pairs ({self.FGFoot, self.BGFoot}) do
			if foot then
				local lowerCheck = SceneMan:CastMORay(foot.Pos, Vector(0, 1), self.ID, -2, rte.airID, false, 1);
				if lowerCheck ~= rte.NoMOID then
					local mo = MovableMan:GetMOFromID(lowerCheck):GetRootParent();
					if mo and mo.Team == self.Team and IsActor(mo) then
						grabbedMO = ToActor(mo);
						break;
					end
				end
			end
		end

		if grabbedMO then
			grabbedMO:AddForce(self.Vel * self.Mass, Vector());
			--If the ID of the grabbed MO is lower than this actor's, it will have its forces applied to it before this, so halve the anti-gravitational force
			if grabbedMO.ID < self.ID then
				self.Vel = (self.Vel + grabbedMO.Vel - SceneMan.GlobalAcc * TimerMan.DeltaTimeSecs) * 0.5;
			else
				self.Vel = (self.Vel + grabbedMO.Vel) * 0.5 - SceneMan.GlobalAcc * TimerMan.DeltaTimeSecs;
			end
			self.AngularVel = (self.AngularVel - grabbedMO.AngularVel) * 0.5;
		end
		
		local dir = 0;
		if controller:IsState(Controller.MOVE_RIGHT) then
			dir = 1;
		elseif controller:IsState(Controller.MOVE_LEFT) then
			dir = -1
		end
		if dir ~= 0 then
			if SceneMan:CastStrengthRay(self.Pos, Vector(self.width * self.FlipFactor, 0), 10, Vector(), 1, rte.airID, SceneMan.SceneWrapsX) then
				self.Vel = Vector(self.Vel.X * 0.75, self.Vel.X * 0.5 - 1);
			elseif grabbedMO then
				self.Vel = self.Vel * 0.5 + Vector(dir, -1);
			end
		end
	end
end