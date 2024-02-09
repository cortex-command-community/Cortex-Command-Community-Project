
function Update(self)

	if self:IsAttached() then
		local parent = self:GetRootParent();
		self.Scale = parent.Scale;
		
		for wound in self.Wounds do
			wound.Scale = parent.Scale;
		end			
		
		if self.Magazine then
			self.Magazine.Scale = parent.Scale
		end
	else
		self.Scale = 1;
		
		if self.Magazine then
			self.Magazine.Scale = 1
		end		

		for wound in self.Wounds do
			wound.Scale = 1;
		end	
		
		self:DisableScript("Techion.rte/Actors/Infantry/ChromeGhost/StealthItem.lua")
		
	end

end
