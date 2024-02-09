function Create(self)
	if self.PinStrength > 0 then
		self.rising = true;
		self.RotAngle = RangeRand(3, 4) * (math.random() < 0.5 and -1 or 1);
		self.Status = Actor.UNSTABLE;
	end
end

function Update(self)
	if self.rising then
		self.Pos = self.Pos + Vector(RangeRand(-1, 1), -math.random());
		self.RotAngle = self.RotAngle * 0.96;
		if SceneMan:GetTerrMatter(self.Pos.X, self.Pos.Y) == rte.airID then
			self.Vel = Vector(-SceneMan.GlobalAcc.X, -SceneMan.GlobalAcc.Y) * 0.2;
			self.PinStrength = 0;
			self:MoveOutOfTerrain(70);
			self.rising = false;
			self:UpdateMovePath();
		end
	else
		self:DisableScript();
	end
end