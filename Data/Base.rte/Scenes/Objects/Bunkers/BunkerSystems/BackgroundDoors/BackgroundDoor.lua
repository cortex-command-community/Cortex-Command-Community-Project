function Create(self)
	self.Frame = 0;

	self.isStayingOpen = false;
	self.isClosing = false;

	self.stayOpenTimer = Timer();
	self.stayOpenTimer:SetSimTimeLimitMS(self:NumberValueExists("StayOpenDuration") and self:GetNumberValue("StayOpenDuration") or 2000);

	self.openCloseSound = CreateSoundContainer("Background Door Open Close Sound", "Base.rte");
	self.openCloseSound.Pos = self.Pos;
	self.openCloseSound:Play();
end

function Update(self)
	if self.Frame == self.FrameCount - 1 and not self.isStayingOpen and not self.isClosing then
		self.isStayingOpen = true;
		self.stayOpenTimer:Reset();
		self.SpriteAnimMode = MOSprite.NOANIM;
	elseif self.isStayingOpen and not self.isClosing and self.stayOpenTimer:IsPastSimTimeLimit() then
		self.SpriteAnimMode = MOSprite.ALWAYSPINGPONG;
		self.isStayingOpen = false;
		self.isClosing = true;
		self.openCloseSound:Play();
	elseif self.Frame == 0 and self.isClosing then
		self.ToDelete = true;
	end
end