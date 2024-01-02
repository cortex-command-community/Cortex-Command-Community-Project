function Update(self)
	local parent = ToTDExplosive(self:GetParent());
	if parent:IsActivated() then
		self.Scale = 1;
		self:EnableEmission(true);
	else
		self.Scale = 0;
		self:EnableEmission(false);
	end
end