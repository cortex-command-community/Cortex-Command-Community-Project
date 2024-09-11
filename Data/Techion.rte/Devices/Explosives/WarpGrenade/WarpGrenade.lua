function Create(self)
	self.fuzeDelay = 4000;
	self.fuzeDecreaseIncrement = 50;
end

function OnAttach(self, parent)
	if not self.fuze then
		self.guideRadius = self:GetRootParent().Radius;
	end
end

function Update(self)
	if self.fuze then
		--Trail effect
		local part = CreateMOPixel("Warp Flicker");
		part.Pos = self.Pos;
		part.Vel = self.Vel;
		part.Lifetime = 10;
		MovableMan:AddParticle(part);
		--Diminish fuze length on impact
		if self.TravelImpulse:MagnitudeIsGreaterThan(1) then
			self.fuzeDelay = self.fuzeDelay - self.TravelImpulse.Magnitude * self.fuzeDecreaseIncrement;
		end
		if self.fuze:IsPastSimMS(self.fuzeDelay) then

			local effect = CreateMOSRotating("Warp Grenade Effect", "Techion.rte");
			effect.Pos = self.Pos;
			MovableMan:AddParticle(effect);
			effect:GibThis();

			if MovableMan:IsActor(self.holder) then

				local effect = CreateMOSRotating("Warp Grenade Effect", "Techion.rte");
				effect.Pos = self.holder.Pos;
				MovableMan:AddParticle(effect);
				effect:GibThis();

				self.holder.Pos = self.Pos + Vector(0, -self.holder.Radius * 0.5);
				self.holder:FlashWhite(200);
				--Telefrag! Kill actors if you teleport directly on top of them
				for actor in MovableMan.Actors do
					if actor.Team ~= self.holder.Team then
						local dist = SceneMan:ShortestDistance(self.holder.Pos, actor.Pos, SceneMan.SceneWrapsX);
						if dist:MagnitudeIsLessThan(5 + (self.holder.Radius * 0.5)) then
							if (self.holder.Mass * 2) > actor.Mass then
								self.holder.Vel = self.holder.Vel + Vector(dist.X, dist.Y):SetMagnitude(1) + Vector(0, -1);
								actor:GibThis();
							else
								self.holder:GibThis();
							end
						end
					end
				end

				local parent = self:GetParent();
				if parent then
					parent:RemoveAttachable(self, true, true);
				end

				if not self.holder:HasObject(self.PresetName) then
					self.holder:AddInventoryItem(CreateTDExplosive(self.PresetName));
				end

				self.ToDelete = true;
			else
				self:GibThis();
			end
		elseif self.ToDelete then
			if MovableMan:IsActor(self.holder) then
				local effect = CreateMOSRotating("Warp Grenade Effect", "Techion.rte");
				effect.Pos = self.holder.Pos;
				MovableMan:AddParticle(effect);
				effect:GibThis();

				self.holder.ToDelete = true;
			end
		end
	elseif self:IsActivated() then
		--Get the holder
		if self.holder == nil or self.holder.ID == rte.NoMOID then
			local newHolder = self:GetRootParent();
			if MovableMan:IsActor(newHolder) then
				self.holder = ToActor(newHolder);
			end
		end
		
		self.fuze = Timer();
	end
end