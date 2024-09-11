function Update(self)
	if self:DoneReloading() then
		self:SetNextMagazineName("Magazine Ronin RPG-7");
	end
	
	if self.RoundInMagCount == 0 and self.Magazine then
		self.Magazine.ToDelete = true;
	end
end

function WhilePieMenuOpen(self, pieMenu)
	if pieMenu.Owner:HasObject("Shovel") and (not self.Magazine or self.Magazine.PresetName ~= "Magazine Ronin Shovel Shot") then
		if pieMenu:GetFirstPieSliceByPresetName("RPG7 Insert Ammo") == nil then
			pieMenu:AddPieSlice(CreatePieSlice("RPG7 Insert Ammo", "Ronin.rte"), self, false);
		end
	else
		pieMenu:RemovePieSlicesByPresetName("RPG7 Insert Ammo");
	end
end