function Create(self)
	self.emitTimer = Timer();
end

function ThreadedUpdate(self)
	if self.Magazine and self.Magazine.RoundCount > 0 then
		if self.FiredFrame then
			if self.emitTimer:IsPastSimMS(1) then
				self.emitTimer:Reset();
	
				self.sparkPar = CreateMOPixel("Hyperion Emission Particle 2");
				self.sparkPar.Pos = self.MuzzlePos;
				self.sparkPar.Vel = self.Vel * 0.5 + Vector(math.random(5), 0):RadRotate(6.28 * math.random());
				self.sparkPar.Team = self.Team;
				self.sparkPar.IgnoresTeamHits = true;
				self.sparkPar.Lifetime = 100;

				self:RequestSyncedUpdate();
			end
		end
	end
end

function SyncedUpdate(self)
	if self.sparkPar then
		MovableMan:AddParticle(self.sparkPar);
		self.sparkPar = nil;
	end
end