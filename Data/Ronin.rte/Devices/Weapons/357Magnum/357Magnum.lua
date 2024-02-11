function Create(self)
	self.rofBoost = 3;
	self.drawGunSpeed = 1;

	self.prevAngle = self.RotAngle;
	self.drawGunAngle = 0;
	self.drawGun = false;
end

function Update(self)
	--Read RateOfFire on Update() to take Global Scripts to account
	if self.rof == nil then
		self.rof = self.RateOfFire;
	end
	-- Special high-RoF revolver fanning mode
	if self:NumberValueExists("CowboyMode") then
		self:SetOneHanded(false);
		self:SetDualWieldable(false);
		if MovableMan:IsOfActor(self.ID) then
			actor = ToActor(MovableMan:GetMOFromID(self.RootID));

			ToActor(actor):GetController():SetState(Controller.AIM_SHARP, false);
			ToActor(actor):GetController():SetState(Controller.BODY_CROUCH, false);
			if ToActor(actor):GetController():IsState(Controller.WEAPON_FIRE) then

				if self:GetNumberValue("CowboyMode") < 3 then
					self:Deactivate();
					self.triggerPulled = true;

					self.StanceOffset = Vector(8, 5);
					self:SetNumberValue("CowboyMode", 3); --Begin draw phase
				end
			else
				self.triggerPulled = false;
				self:Deactivate();
			end

			if self:GetNumberValue("CowboyMode") >= 3 then
				--In the draw phase, you can still fire - but the gun may be pointed downwards and result in friendly fire
				if self.drawGunAngle > 0 then
					self.drawGunAngle = self.drawGunAngle - (0.1 * self.drawGunSpeed);
					self.Team = -1;
				elseif self.drawGunAngle < 0 then
					self.drawGunAngle = 0;
					if self.triggerPulled == false then
						self:SetNumberValue("CowboyMode", 4); --Final state: gun is drawn
					end
				end
			end

			self.RotAngle = self.RotAngle - (self.drawGunAngle * self.FlipFactor);

			if self:GetNumberValue("CowboyMode") == 1 then --Setup phase
				self.RateOfFire = self.rof * self.rofBoost;
				self.StanceOffset = Vector(3, 10);
				self:SetNumberValue("CowboyMode", 2); --Setup done

				self.drawGunAngle = 1.4;
				self:Deactivate();
			elseif self:GetNumberValue("CowboyMode") == 2 then --Ready to draw
				self:Deactivate();
			elseif self:GetNumberValue("CowboyMode") == 3 then --Drawing gun
				if self.triggerPulled == true then
					self:Deactivate();
				else
					self:Deactivate();
					self:SetNumberValue("CowboyMode", 4); --Soon can fire
				end
			elseif self:GetNumberValue("CowboyMode") == 6 then --Revert without firing
				self:RemoveNumberValue("CowboyMode");
			end
			
			if self:IsReloading() then
				self.RotAngle = self.prevAngle + (self.FlipFactor * 0.42 * self.drawGunSpeed);
				self.JointOffset = Vector(2, 2);
				local jointOffset = Vector(self.JointOffset.X * self.FlipFactor, self.JointOffset.Y):RadRotate(self.RotAngle);
				self.Pos = self.Pos - jointOffset + Vector(jointOffset.X, jointOffset.Y):RadRotate(-self.RotAngle);
				self.prevAngle = self.RotAngle;
				self:SetNumberValue("CowboyMode", 5);
			else
				self.prevAngle = self.RotAngle;
				if self:GetNumberValue("CowboyMode") > 4 then
					self:RemoveNumberValue("CowboyMode");
				end
			end
		else
			self:RemoveNumberValue("CowboyMode");
		end
	else --Revert values if dropped or no longer controlled by player
		self:SetOneHanded(true);
		self:SetDualWieldable(true);
		self.RateOfFire = self.rof;
		self.JointOffset = Vector(-2, 3);
		self.StanceOffset = Vector(12, 0);
		self.drawGunAngle = 0;
	end
end