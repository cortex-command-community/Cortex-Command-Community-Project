function Update(self)
	if self.FiredFrame then
		self.Frame = self.FrameCount - 1;
		if self.Magazine then
			self.Magazine.ToDelete = true;
		end
	elseif self:IsReloading() then
		local inverseProgress = 1 - self.ReloadProgress;
		self.Frame = math.min(math.ceil((self.FrameCount) * inverseProgress), self.Frame);
		local parent = self:GetRootParent();
		if IsAHuman(parent) then
			parent = ToAHuman(parent);
			if parent.FGArm and parent.BGArm then
				pullArm = self:GetParent().ID == parent.BGArm.ID and parent.FGArm or parent.BGArm;
				pullArm:ClearHandTargets();
				pullArm.HandPos = self.Pos + Vector(6 * self.FlipFactor * math.sin(inverseProgress * math.pi), -2):RadRotate(self.RotAngle);
			end
		end
	elseif self:DoneReloading() then
		self.Frame = 0;
	elseif self.Frame ~= 0 and self.Frame ~= self.FrameCount - 1 then
		self.Frame = self.FrameCount - 1;
	end
	
	if self.Magazine and not self:GetParent() and self.TravelImpulse:MagnitudeIsGreaterThan(self.Mass) then
		if math.random() < 0.5 then
			self:Activate();
		else
			self:RemoveAttachable(self.Magazine, true, true);
		end
	end
end