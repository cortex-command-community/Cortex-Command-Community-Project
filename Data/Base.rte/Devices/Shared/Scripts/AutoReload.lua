function Update(self)
	if self.RoundInMagCount == 0 and IsActor(self:GetRootParent()) then
		self:Reload();
	end
end