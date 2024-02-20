function Create(self)
	self.delayTimer = Timer();
	self.actionPhase = 0;
	self.fireOn = false;
	self.alliedTeam = -1;

	self.detonateDelay = 60000/self.RateOfFire;

	self.detonateSound = CreateSoundContainer("Explosive Device Detonate", "Base.rte");
end

function ThreadedUpdate(self)
	if self.ID ~= self.RootID then
		local actor = MovableMan:GetMOFromID(self.RootID);
		if MovableMan:IsActor(actor) then
			self.alliedTeam = ToActor(actor).Team;
		end
	end
	if self.Magazine then
		if self:IsActivated() then
			if self.delayTimer:IsPastSimMS(self.detonateDelay) then
				self.delayTimer:Reset();
				self.actionPhase = self.actionPhase + 1;
				self.blink = false;
			else
				self.Magazine.RoundCount = math.ceil(100 * (1 - 1 * self.delayTimer.ElapsedSimTimeMS/self.detonateDelay));
			end
			if self.actionPhase >= 1 then
			
				self.explosiveTable = {};

				for particle in MovableMan.Particles do
					if particle.PresetName == "Remote Explosive Active" then
						table.insert(self.explosiveTable, particle);
					end
				end
				
				self:RequestSyncedUpdate();
				
				self.detonateSound:Play(self.Pos);
				self:Reload();
			end
		else
			self.delayTimer:Reset();
			self.Magazine.RoundCount = 100;
			self.fireOn = false;
			self.actionPhase = 0;
		end
	end
end

function SyncedUpdate(self)
	for k, explosive in pairs(self.explosiveTable) do
		explosive:SendMessage("RemoteExplosive_Detonate", self.alliedTeam);
	end
	
	self.explosiveTable = nil;
end