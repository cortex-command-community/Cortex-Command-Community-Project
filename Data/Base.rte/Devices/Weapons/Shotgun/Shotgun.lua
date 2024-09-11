function Create(self)
	self.pullTimer = Timer();
	self.loaded = false;
	self.rotFactor = math.pi;

	self.cockSound = CreateSoundContainer(self:StringValueExists("CockSound") and self:GetStringValue("CockSound") or "Base.rte/Chamber Round");
end

function ThreadedUpdate(self)
	local parent;
	local actor = self:GetRootParent();
	if actor and IsAHuman(actor) then
		parent = ToAHuman(actor);
	end

	if self.FiredFrame then
		self.shell = CreateMOSParticle("Base.rte/Shell");
		self.loaded = false;
		self.playedSound = false;
		self.rotFactor = math.pi;
	end

	if parent and not self.loaded and self.RoundInMagCount > 0 and not self.reloadCycle then
		if self.pullTimer:IsPastSimMS(15000/self.RateOfFire) then
			if not self.playedSound then
				self.cockSound:Play(self.Pos);
				self.playedSound = true;
			end

			if self.shell then
				self.shell.Pos = self.Pos;
				self.shell.Vel = self.Vel + Vector(-6 * self.FlipFactor, -4):RadRotate(self.RotAngle);
				self.shell.Team = self.Team;
				self:RequestSyncedUpdate();
			end

			self.Frame = 1;
			self.SupportOffset = Vector(1, 2);
			local rotTotal = math.sin(self.rotFactor)/5;
			self.InheritedRotAngleOffset = rotTotal;
			self.rotFactor = self.rotFactor - math.pi * 0.0005 * self.RateOfFire;
		end
		
		if self.rotFactor <= 0 then
			self.loaded = true;
			self.Frame = 0;
			self.SupportOffset = Vector(4, 1);
			self.rotFactor = 0;
		end
	else
		self.pullTimer:Reset();
	end
end

function SyncedUpdate(self)
	if self.shell then
		MovableMan:AddParticle(self.shell);
		self.shell = nil;
	end
end