function Create(self)
	self.baseWalkSpeed = self:GetLimbPathTravelSpeed(Actor.WALK);
end

function ThreadedUpdate(self)
	self:SetLimbPathTravelSpeed(Actor.WALK, self.baseWalkSpeed);
	local controller = self:GetController();
	if self.Health > 0 and not controller:IsState(Controller.BODY_WALKCROUCH) then
		local walkSpeedScalar = 2.3;
		local legs = {self.FGLeg, self.BGLeg};
		for _, leg in pairs(legs) do
			walkSpeedScalar = walkSpeedScalar - (leg and leg.Frame/leg.FrameCount or 1.1);
		end
		self:SetLimbPathTravelSpeed(Actor.WALK, self.baseWalkSpeed * walkSpeedScalar);
		--[[Display health as relative to physical damage? (to-do this in source instead)
		if self.Head and (self.FGArm or self.BGArm or self.FGLeg or self.BGLeg) then
			local limbs = {self.Head, self.FGArm, self.BGArm, self.FGLeg, self.BGLeg};
			self.Health = self.MaxHealth * (1 - self.WoundCount/self:GetGibWoundLimit(true, false, false)) * #limbs/5;
		end
		]]--
	end
end