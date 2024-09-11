function Create(self)
	self.doorParent = ToADoor(self:GetRootParent());
	self.lastFrame = 0;
end

function ThreadedUpdate(self)
	if IsADoor(self.doorParent) then
		-- handle frame wrapping
		if self.doorParent:GetDoorState() == ADoor.OPENING then
			if self.lastFrame == self.doorParent.FrameCount - 1 and self.doorParent.Frame == 0 then
				self.lastFrame = -1;
			end
		else
			if self.lastFrame == 0 and self.doorParent.Frame == self.doorParent.FrameCount - 1 then
				self.lastFrame = self.doorParent.FrameCount;
			end			
		end	
	
		-- if our parent increased a frame, increase a frame, if our parent decreased, etc etc...
		self.Frame = (self.doorParent.Frame - self.lastFrame < 0 and self.Frame - 1 or self.doorParent.Frame - self.lastFrame > 0 and self.Frame + 1 or self.Frame) % self.FrameCount;
		self.lastFrame = self.doorParent.Frame ~= self.lastFrame and self.doorParent.Frame or self.lastFrame;
	end
end