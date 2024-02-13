function Create(self)
	self.fuzeDelay = 3000;
end

function Update(self)
	self.AngularVel = (self.AngularVel - self.Vel.X * 0.1) * 0.9;
	if self.fuze then
		local velNum = math.ceil(math.sqrt(self.Vel.Magnitude));
		for i = 1, velNum do
			local trail = CreateMOPixel("Impulse Grenade Trail Glow");
			trail.Pos = self.Pos - Vector(self.Vel.X, self.Vel.Y):SetMagnitude(i);
			MovableMan:AddParticle(trail);
		end
		
		local glow = CreateMOPixel("Impulse Grenade Trail Glow 2");
		glow.Pos = self.Pos;
		MovableMan:AddParticle(glow);
		if self.fuze:IsPastSimMS(self.fuzeDelay) then
			self:MoveOutOfTerrain(20);
			local payload = CreateMOSRotating("Impulse Grenade Payload", "Dummy.rte");
			if payload then
				payload.Pos = self.Pos;
				MovableMan:AddParticle(payload);
				payload:GibThis();
			end
			self:GibThis();
		end
	elseif self:IsActivated() then
		self.fuze = Timer();
		self.GibWoundLimit = self.GibWoundLimit * 2;
	end

	if self.deadMansSwitch and self.ID ~= rte.NoMOID then
		if self.RootID == self.ID then
			self:Activate();
		end
	end

	if self:GetParent() or self.ID == rte.NoMOID or self.WoundCount > 0 then
		self.deadMansSwitch = true;
	end
end