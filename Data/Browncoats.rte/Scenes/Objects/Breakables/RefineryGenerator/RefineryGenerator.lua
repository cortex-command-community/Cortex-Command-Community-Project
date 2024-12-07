function Create(self)
	self.Activity = ToGameActivity(ActivityMan:GetActivity());
	
	self.loopSound = CreateSoundContainer("Yskely Refinery Refinery Generator Loop", "Browncoats.rte");
	
	-- It's one loop for many generators, so avoid direct overlap
	-- This will potentially sound bad if loading a game near the generators, but not as bad as having them overlap
	self.randomLoopStartTimer = Timer();
	self.randomLoopStartTime = math.random(500, 10000);
end

function ThreadedUpdate(self)
	if (not self.loopSound:IsBeingPlayed()) and self.randomLoopStartTimer:IsPastSimMS(self.randomLoopStartTime) then
		self.loopSound:Play(self.Pos);
	end
end

function OnDestroy(self)
	self.Activity:SendMessage("RefineryAssault_RefineryGeneratorBroken");
end