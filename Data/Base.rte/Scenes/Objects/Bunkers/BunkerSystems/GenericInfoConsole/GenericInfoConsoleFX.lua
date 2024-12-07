function Create(self)
	self.FXTimer = Timer();
	self.FXDelay = 1000;
end

function ThreadedUpdate(self)
	if self.shouldDisplayMessage then
		self.Frame = 1;
	else
		if self.FXTimer:IsPastSimMS(self.FXDelay) then
			self.FXTimer:Reset();
			self.Frame = (self.Frame + 1) % 2;
		end
	end
end