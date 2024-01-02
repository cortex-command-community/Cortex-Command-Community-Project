function RoninRPGSwitchAmmo(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;
	if gun and pieMenuOwner:HasObject("Shovel") then
		local gun = ToHDFirearm(gun);
		local magSwitchName = "Magazine Ronin Shovel Shot";
		if gun.Magazine == nil or gun.Magazine.PresetName ~= magSwitchName then
			pieMenuOwner:RemoveInventoryItem("Shovel");
			gun:SetNextMagazineName(magSwitchName);
			gun:Reload();
		end
	end
end