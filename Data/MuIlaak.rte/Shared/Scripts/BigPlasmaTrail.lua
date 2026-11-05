function Update(self)
	local velNum = math.ceil(math.sqrt(self.Vel.Magnitude));
	for i = 1, velNum do
		local trail = CreateMOPixel("Plasma Bolt Trail Glow");
		trail.Pos = self.Pos - Vector(self.Vel.X, self.Vel.Y):SetMagnitude(i);
		MovableMan:AddParticle(trail);
	end
	
	local glow = CreateMOPixel("Plasma Bolt Trail Glow 2");
	glow.Pos = self.Pos;
	MovableMan:AddParticle(glow);
end