function Create(self)
	self.unpinThreshold = self.GibWoundLimit * 0.5;
	self.plummetSound = CreateSoundContainer("Browncoat Refinery Fuel Tank Large Plummet", "Browncoats.rte");
end

function Update(self)
	self.plummetSound.Pos = self.Pos;
	if self.WoundCount > self.unpinThreshold and self.PinStrength > 0 then
		self.PinStrength = 0;
		
		self.plummetSound:Play(self.Pos);
		self.AngularVel = math.random(-1, 1);
		self.Vel = self.Vel + Vector(math.random(-1, 1), 0);
	end
end

function OnCollideWithTerrain(self)
	self:GibThis();
end

function OnCollideWithMO(self, MO, rootMO)
	if self.PinStrength == 0 and MO.UniqueID == rootMO.UniqueID and IsMOSRotating(MO) then
		ToMOSRotating(MO):GibThis();
	end
end

function OnDestroy(self)
	self.plummetSound:FadeOut(200);
end