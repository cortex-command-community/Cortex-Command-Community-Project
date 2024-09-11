--TODO: Add cylinder sounds!
function Create(self)
	self.fireTimer = Timer();
end

function Update(self)
	if self.FiredFrame or self:IsReloading() then
		self.fireTimer:Reset();
	end
	
	self.Frame = math.floor(self.FrameCount * (1 - math.min(self.fireTimer.ElapsedSimTimeMS/(60000/math.max(self.RateOfFire, 1)), 1)) + 0.5);
end