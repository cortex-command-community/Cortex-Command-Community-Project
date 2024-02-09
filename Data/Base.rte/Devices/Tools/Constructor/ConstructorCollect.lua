function Create(self)
	self.speed = self.Vel.Magnitude;
	if self.Sharpness ~= rte.NoMOID then
		local mo = MovableMan:GetMOFromID(self.Sharpness);
		if mo then
			self.target = mo;
			self.Sharpness = 0;
		end
	else
		self.ToDelete = true;
	end
end

function Update(self)
	if self.target and self.target.ID ~= rte.NoMOID then
		self:NotResting();
		targetPos = IsHDFirearm(self.target) and ToHDFirearm(self.target).MuzzlePos or self.target.Pos;
		local dist = SceneMan:ShortestDistance(self.Pos, targetPos, SceneMan.SceneWrapsX);
		if dist:MagnitudeIsGreaterThan(self.speed) then
			self.Vel = Vector(dist.X, dist.Y):SetMagnitude(self.speed) - (SceneMan.GlobalAcc * TimerMan.DeltaTimeSecs) * self.GlobalAccScalar;
		else
			self.ToDelete = true;
		end
	else
		self.target = nil;
		self.PinStrength = 0;
	end
	
	if self.PinStrength > 0 then
		self.Pos = self.Pos + self.Vel * rte.PxTravelledPerFrame;
	end
end