function Create(self)
	self.detonationDelay = self.Lifetime - 20;

	self.lifeTimer = Timer();
	self.emitTimerA = Timer();
	self.emitTimerB = Timer();
	self.emitSpread = math.pi * 2;

	self.smallAngle = math.pi/6;

	self.angleList = {};
end

function Update(self)
	if MovableMan:IsParticle(self) and self.lifeTimer:IsPastSimMS(self.detonationDelay) then
		self:GibThis();
	else
		if self.emitTimerA:IsPastSimMS(40) then
			self.emitTimerA:Reset();

			self.angleList = {};

			for i = 1, 12 do
				local angleCheck = self.smallAngle * i;

				for i = 1, 5 do
					local checkPos = self.Pos + Vector(i, 0):RadRotate(angleCheck);
					if SceneMan.SceneWrapsX == true then
						if checkPos.X > SceneMan.SceneWidth then
							checkPos = Vector(checkPos.X - SceneMan.SceneWidth, checkPos.Y);
						elseif checkPos.X < 0 then
							checkPos = Vector(SceneMan.SceneWidth + checkPos.X, checkPos.Y);
						end
					end
					
					local terrCheck = SceneMan:GetTerrMatter(checkPos.X,checkPos.Y);
					if terrCheck ~= rte.airID then
						break;
					end

					if i == 5 then
						self.angleList[#self.angleList + 1] = angleCheck;
					end
				end
			end

			if #self.angleList > 0 then
				local listNum = #self.angleList;

				local randomAngleA = self.angleList[math.random(listNum)] + (math.random() * 0.4) - 0.2;
				local randomAngleB = self.angleList[math.random(listNum)] + (math.random() * 0.4) - 0.2;
				local randomAngleC = self.angleList[math.random(listNum)] + (math.random() * 0.4) - 0.2;

				local fireA = CreateMOPixel("Particle Browncoat Incendiary Flame 1");
				fireA.Pos = self.Pos;
				fireA.Vel = self.Vel * 0.5 + Vector(math.random(10, 15), 0):RadRotate(randomAngleA);
				MovableMan:AddParticle(fireA);

				local fireB = CreateMOPixel("Particle Browncoat Incendiary Flame 2");
				fireB.Pos = self.Pos;
				fireB.Vel = self.Vel * 0.5 + Vector(math.random(5, 10), 0):RadRotate(randomAngleB);
				MovableMan:AddParticle(fireB);

				local fireC = CreateMOSParticle("Tiny Smoke Ball Incendiary Flamer 1");
				fireC.Pos = self.Pos;
				fireC.Vel = self.Vel * 0.5 + Vector(math.random(2, 5), 0):RadRotate(randomAngleC);
				MovableMan:AddParticle(fireC);
			else
				local fireA = CreateMOPixel("Particle Browncoat Incendiary Flame 1");
				fireA.Pos = self.Pos;
				fireA.Vel = self.Vel * 0.5 + Vector(math.random(10, 15), 0):RadRotate(math.random() * self.emitSpread);
				MovableMan:AddParticle(fireA);

				local fireB = CreateMOPixel("Particle Browncoat Incendiary Flame 2");
				fireB.Pos = self.Pos;
				fireB.Vel = self.Vel * 0.5 + Vector(math.random(5, 10), 0):RadRotate(math.random() * self.emitSpread);
				MovableMan:AddParticle(fireB);

				local fireC = CreateMOSParticle("Tiny Smoke Ball Incendiary Flamer 1");
				fireC.Pos = self.Pos;
				fireC.Vel = self.Vel * 0.5 + Vector(math.random(2, 5), 0):RadRotate(math.random() * self.emitSpread);
				MovableMan:AddParticle(fireC);
			end

		end

		if self.emitTimerB:IsPastSimMS(60) then
			self.emitTimerB:Reset();

			local listNum = #self.angleList;

			if #self.angleList > 0 then
				local randomAngleA = self.angleList[math.random(listNum)] + (math.random() * 0.4) - 0.2;
				local randomAngleB = self.angleList[math.random(listNum)] + (math.random() * 0.4) - 0.2;

				local fireD = CreateMOSParticle("Tiny Smoke Ball Incendiary Flamer 2");
				fireD.Pos = self.Pos;
				fireD.Vel = self.Vel * 0.5 + Vector(math.random(10, 15), 0):RadRotate(randomAngleA);
				MovableMan:AddParticle(fireD);

				local fireE = CreateMOSParticle("Tiny Smoke Ball Incendiary Flamer 3");
				fireE.Pos = self.Pos;
				fireE.Vel = self.Vel * 0.5 + Vector(math.random(5, 10), 0):RadRotate(randomAngleB);
				MovableMan:AddParticle(fireE);

				local flame = CreatePEmitter("Flame Hurt Short Float");
				flame.Pos = self.Pos;
				flame.Vel = self.Vel * 0.5 + Vector(math.random(3, 7), 0):RadRotate(math.random() * self.emitSpread);
				flame.Lifetime = flame.Lifetime * RangeRand(0.5, 1.0);
				MovableMan:AddParticle(flame);
			else
				local fireD = CreateMOSParticle("Tiny Smoke Ball Incendiary Flamer 2");
				fireD.Pos = self.Pos;
				fireD.Vel = self.Vel * 0.5 + Vector(math.random(10, 15), 0):RadRotate(math.random() * self.emitSpread);
				MovableMan:AddParticle(fireD);

				local fireE = CreateMOSParticle("Tiny Smoke Ball Incendiary Flamer 3");
				fireE.Pos = self.Pos;
				fireE.Vel = self.Vel * 0.5 + Vector(math.random(5, 10), 0):RadRotate(math.random() * self.emitSpread);
				MovableMan:AddParticle(fireE);
			end
		end
	end
end