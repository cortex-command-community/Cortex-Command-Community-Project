function Update(self)
	if self.Magazine then
		local parent = self:GetRootParent();
		if parent and IsActor(parent) then
			parent = ToActor(parent);
			local parentDimensions = Vector(ToMOSprite(parent):GetSpriteWidth(), ToMOSprite(parent):GetSpriteHeight());
			local magDimensions = Vector(self.Magazine:GetSpriteWidth(), self.Magazine:GetSpriteHeight());
			self.Magazine.Pos = parent.Pos + Vector(-(parentDimensions.X + magDimensions.X) * 0.4 * parent.FlipFactor, 1 - (parentDimensions.Y + magDimensions.Y) * 0.2):RadRotate(parent.RotAngle);
			self.Magazine.RotAngle = parent.RotAngle;
		end
	end
end