function Create(self)
	self.muzzleTable = {self.MuzzleOffset + Vector(0, -1), self.MuzzleOffset + Vector(0, 2)};
	self.muzzleSelect = 0;
end

function ThreadedUpdate(self)
	if self.FiredFrame then
		self.muzzleSelect = math.abs(self.muzzleSelect - 1);
		self.MuzzleOffset = self.muzzleTable[self.muzzleSelect + 1];
	end
end