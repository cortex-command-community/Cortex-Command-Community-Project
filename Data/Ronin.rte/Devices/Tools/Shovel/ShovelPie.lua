function RoninCreateSandbag(pieMenuOwner, pieMenu, pieSlice)
	if pieMenuOwner:GetNumberValue("RoninShovelResource") >= 10 then
		pieMenuOwner = ToAHuman(pieMenuOwner);
		pieMenuOwner:RemoveNumberValue("RoninShovelResource");
		pieMenuOwner.PieMenu:GetFirstPieSliceByPresetName("Ronin Shovel Fill Sandbag PieSlice").Enabled = false;
		pieMenuOwner:AddInventoryItem(CreateThrownDevice("Ronin.rte/Sandbag"));
		pieMenuOwner:EquipNamedDevice("Sandbag", true);
	else
		local errorSound = CreateSoundContainer("Error", "Base.rte");
		errorSound:Play(pieMenuOwner.Pos, pieMenuOwner:GetController().Player);
	end
end