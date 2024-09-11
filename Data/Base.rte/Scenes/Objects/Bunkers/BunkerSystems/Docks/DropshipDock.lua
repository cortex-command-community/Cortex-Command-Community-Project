function Create(self)
	self.updateTimer = Timer();
	self.healTimer = Timer();
	self.detectionRange = 50;
end

function Update(self)
	--Settle into terrain instead of gibbing
	if self.WoundCount > (self.GibWoundLimit * 0.9) then
		self.ToSettle = true;
	end

	if self.craft and IsActor(self.craft) and self.craft.AIMode == Actor.AIMODE_STAY then
		--Disable collisions with the ship
		self.craft:SetWhichMOToNotHit(self, 100);
		--Pin the ship and pull it nicely into the docking unit.
		local dist = SceneMan:ShortestDistance(self.craft.Pos, self.Pos + Vector(0, ToMOSprite(self.craft):GetSpriteHeight() * 0.5), SceneMan.SceneWrapsX);
		self.craft.Vel = self.craft.Vel * 0.9 + dist/(3 + self.craft.Vel.Magnitude);
		self.craft.AngularVel = self.craft.AngularVel * 0.9 - self.craft.RotAngle * 3;

		if self.craft.Status < Actor.DYING then
			self.craft.Status = Actor.UNSTABLE;	--Deactivated
		end
		
		--Heal the craft.
		if self.healTimer:IsPastSimMS(self.craft.Mass) then
			self.healTimer:Reset();
			if self.craft.WoundCount > 0 then
				self.craft:RemoveWounds(1);
			elseif self.craft.Health < self.craft.MaxHealth then
				self.craft.Health = math.min(self.craft.Health + 1, self.craft.MaxHealth);
			end
		end
	elseif self.updateTimer:IsPastSimMS(200) then
		self.craft = nil;
		for actor in MovableMan.Actors do
			--See if a live dropship is within 50 pixel range of the docking unit and if it has the AI mode set to "Stay".
			if (actor.ClassName == "ACDropShip") and (actor.AIMode == Actor.AIMODE_STAY) and actor.Health > 0 and (math.abs(actor.Pos.X - self.Pos.X) < self.detectionRange) and (math.abs(actor.Pos.Y - self.Pos.Y) < self.detectionRange) then
				self.craft = ToACDropShip(actor);
			end
		end

		self.updateTimer:Reset();
	end
end