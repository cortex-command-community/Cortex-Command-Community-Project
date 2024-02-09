function Create(self)
	self.shieldWalkSound = CreateSoundContainer("Metal Shield Walk", "Base.rte");
	self.shieldWalkTimer = Timer();
end

function ThreadedUpdate(self)
	self.shieldWalkSound.Pos = self.Pos;
	self.parent = self:GetRootParent();
	
	if IsAHuman(self.parent) then
		if ToAHuman(self.parent).StrideFrame and self.shieldWalkTimer:IsPastSimMS(400) then
			self.shieldWalkTimer:Reset();
			self.shieldWalkSound:Play(self.Pos);
		end
	end
end