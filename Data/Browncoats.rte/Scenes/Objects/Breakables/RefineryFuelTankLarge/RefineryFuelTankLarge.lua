function Create(self)
	self.unpinThreshold = self.GibWoundLimit * 0.5;
end

function Update(self)
	if self.WoundCount > self.unpinThreshold and self.PinStrength > 0 then
		self.PinStrength = 0;
		
		self.AngularVel = math.random(-1, 1);
		self.Vel = self.Vel + Vector(math.random(-1, 1), 0);
	end
end

function OnCollideWithTerrain(self)
	self:GibThis();
end