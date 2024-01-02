function Create(self)
	self.fuzeDelay = 4000;
	self.payload = CreateMOSRotating("Frag Grenade Payload", "Coalition.rte");
end

function Update(self)
	if self.fuze then
		if self.fuze:IsPastSimMS(self.fuzeDelay) then
			self.ToDelete = true;
		end
	elseif self:IsActivated() then
		self.fuze = Timer();
	end
end

function Destroy(self)
	if self.fuze and self.payload then
		self.payload.Pos = Vector(self.Pos.X, self.Pos.Y);
		MovableMan:AddParticle(self.payload);
		self.payload:GibThis();
	end
end