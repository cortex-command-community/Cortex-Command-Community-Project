function Create(self)
	self.lastVel = Vector(self.Vel.X, self.Vel.Y);
end

function Update(self)
	if math.abs(self.lastVel.AbsRadAngle - self.Vel.AbsRadAngle) > 0.25 then
		self.Vel = Vector(self.lastVel.Magnitude, 0):RadRotate(self.Vel.AbsRadAngle + (math.random() * 0.4) - 0.2);
	end
	
	self.lastVel = Vector(self.Vel.X, self.Vel.Y);
end