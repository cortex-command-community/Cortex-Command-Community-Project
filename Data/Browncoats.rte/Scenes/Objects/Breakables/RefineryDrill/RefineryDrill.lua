function OnGlobalMessage(self, message, context)

	if message == "RefineryAssault_DrillOverloadBegin" then
		self.overloading = true;
		self.overloadSound:Play(self.Pos + Vector(0, 200));
		self.overloadFXTimer = Timer();
		self.loopSound:FadeOut(2000);
	end

end

function Create(self)
	
	self.overloadExplodeSound = CreateSoundContainer("Yskely Refinery Drill Overload Explode", "Browncoats.rte");
	self.overloadSound = CreateSoundContainer("Yskely Refinery Drill Overload Start", "Browncoats.rte");

	self.loopSound = CreateSoundContainer("Yskely Refinery Drill Idle Loop", "Browncoats.rte");
	self.loopSound:Play(self.Pos + Vector(0, 200));
	
end

function ThreadedUpdate(self)

	if self.overloading then
		if not self.overloadSound:IsBeingPlayed() and not self.overloadExploded then
			self.overloadExplodeSound:Play(self.Pos + Vector(0, 200));
			self.overloadExploded = true;
			self.overloadFXTimer:Reset();
			CameraMan:AddScreenShake(100, self.Pos + Vector(0, 200));
		elseif self.overloadExploded then
			-- fx soon
		else
			-- fx soon
		end
	end
	
end

function OnDestroy(self)

	self.loopSound:Stop(-1);

end