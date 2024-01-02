function Create(self)
	self.lifeTimer = Timer();
	self.emitTimerA = Timer();
	self.emitTimerB = Timer();
end

function Update(self)
	if MovableMan:IsParticle(self) and self.lifeTimer:IsPastSimMS(3000) then
		self.ToDelete = true;
	else
		--self.ToDelete = false;
		--self.ToSettle = false;

		if self.emitTimerA:IsPastSimMS(40) then
			self.emitTimerA:Reset();

			local fireA = CreateMOPixel("Particle Browncoat Incendiary Flame 1");
			fireA.Pos = self.Pos;
			fireA.Vel = Vector((math.random()*5)+10,0):RadRotate(math.random()*6.28318);
			fireA.Team = self.Team;
			fireA.IgnoresTeamHits = true;
			MovableMan:AddParticle(fireA);

			local fireB = CreateMOPixel("Particle Browncoat Incendiary Flame 2");
			fireB.Pos = self.Pos;
			fireB.Vel = Vector((math.random()*5)+5,0):RadRotate(math.random()*6.28318);
			fireB.Team = self.Team;
			fireB.IgnoresTeamHits = true;
			MovableMan:AddParticle(fireB);

			local fireC = CreateMOSParticle("Tiny Smoke Ball Incendiary Flamer 1");
			fireC.Pos = self.Pos;
			fireC.Vel = Vector((math.random()*3)+2,0):RadRotate(math.random()*6.28318);
			fireC.Team = self.Team;
			fireC.IgnoresTeamHits = true;
			MovableMan:AddParticle(fireC);
		end

		if self.emitTimerB:IsPastSimMS(60) then
			self.emitTimerB:Reset();

			local fireD = CreateMOSParticle("Tiny Smoke Ball Incendiary Flamer 2");
			fireD.Pos = self.Pos;
			fireD.Vel = Vector((math.random()*5)+10,0):RadRotate(math.random()*6.28318);
			fireD.Team = self.Team;
			fireD.IgnoresTeamHits = true;
			MovableMan:AddParticle(fireD);

			local fireE = CreateMOSParticle("Tiny Smoke Ball Incendiary Flamer 3");
			fireE.Pos = self.Pos;
			fireE.Vel = Vector((math.random()*5)+5,0):RadRotate(math.random()*6.28318);
			fireE.Team = self.Team;
			fireE.IgnoresTeamHits = true;
			MovableMan:AddParticle(fireE);
		end
	end
end