function Create(self)
	self.lastRoundCount = self.RoundCount;
	self.loopFrames = 3;
end

function Update(self)
	if self.RoundCount ~= self.lastRoundCount then
		if self.RoundCount < self.FrameCount - self.loopFrames then
			self.Frame = self.FrameCount - self.RoundCount - 1;
		else
			self.Frame = (self.Frame + 1) % self.loopFrames;
		end
	end
	
	self.lastRoundCount = self.RoundCount;
end