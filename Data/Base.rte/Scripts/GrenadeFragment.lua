function Create(self)
	self.skipFrames = math.random(2);
end

function Update(self)
	if not self.HitsMOs and math.floor(self.Age/TimerMan.DeltaTimeMS + 0.5) >= self.skipFrames then
		self.HitsMOs = true;
		self:DisableScript();
	end
end