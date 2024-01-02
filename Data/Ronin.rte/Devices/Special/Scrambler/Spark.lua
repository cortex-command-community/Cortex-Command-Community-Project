function Create(self)
	self.checkTimer = Timer();
	self.checkDelay = math.random(30);
end

function Update(self)
	if self.checkTimer:IsPastSimMS(self.checkDelay) then
		self.checkTimer:Reset();
		self.checkDelay = self.checkDelay * 1.1;
		self.Vel = Vector(self.Vel.X, self.Vel.Y):RadRotate(RangeRand(-0.5, 0.5));
	end
end