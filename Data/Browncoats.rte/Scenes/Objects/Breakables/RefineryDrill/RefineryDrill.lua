function OnGlobalMessage(self, message, context)
	if message == "RefineryAssault_DrillOverloadBegin" then
		self.overloading = true;
		self.overloadSound:Play(self.Pos + Vector(0, 200));
		self.overloadFXTimer = Timer();
		self.loopSound:FadeOut(2000);
	end
end

function Create(self)
	self.Activity = ToGameActivity(ActivityMan:GetActivity());
	
	self.overloadExplodeSound = CreateSoundContainer("Yskely Refinery Drill Overload Explode", "Browncoats.rte");
	self.overloadSound = CreateSoundContainer("Yskely Refinery Drill Overload Start", "Browncoats.rte");
	-- How long the overload sound is, in MS. The drill blows up once that much sim time has gone by, instead of when the audio system is done playing the sound, since that goes by real time.
	self.overloadSoundLength = 16165;

	self.loopSound = CreateSoundContainer("Yskely Refinery Drill Idle Loop", "Browncoats.rte");
	self.loopSound:Play(self.Pos + Vector(0, 200));
end

function ThreadedUpdate(self)
	if self.overloading then
		if self.overloadFXTimer:IsPastSimMS(self.overloadSoundLength) and not self.overloadExploded then
			self.overloadExplodeSound:Play(self.Pos + Vector(0, 200));
			self.overloadExploded = true;
			self.overloadFXTimer:Reset();
			CameraMan:AddScreenShake(100, self.Pos + Vector(0, 200));
			self.Activity:SendMessage("Refinery_S3DrillExploded");
		elseif self.overloadExploded then
			-- fx soon
		else
			-- fx soon
		end
	end
end

function Destroy(self)
	self.loopSound:Stop(-1);
end
