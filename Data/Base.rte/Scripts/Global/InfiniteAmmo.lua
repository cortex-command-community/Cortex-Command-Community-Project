function InfiniteAmmo:UpdateScript()
	local items = {};
	for actor in MovableMan.Actors do
		local weapon, item;
		if IsAHuman(actor) then
			item = ToAHuman(actor).EquippedItem;
			if item then
				if IsHDFirearm(item) then
					table.insert(items, ToHDFirearm(item));
				elseif IsTDExplosive(item) then
					--Add a new grenade to the inventory every time one is thrown
					local grenade = ToTDExplosive(item);
					if actor:GetController():IsState(Controller.WEAPON_FIRE) then
						local count = 0;
						for i = 1, actor.InventorySize do
							local potentialWep = actor:Inventory();
							if potentialWep.PresetName == grenade.PresetName then
								count = count + 1;
							end
							actor:SwapNextInventory(potentialWep, true);
						end
						if count == 0 then
							actor:AddInventoryItem(CreateTDExplosive(grenade:GetModuleAndPresetName()));
						end
					end
				end
			end
			local itemBG = ToAHuman(actor).EquippedBGItem;
			if itemBG and IsHDFirearm(itemBG)then
				table.insert(items, ToHDFirearm(itemBG));
			end
		elseif IsACrab(actor) then
			item = ToACrab(actor).EquippedItem;
			if item and IsHDFirearm(item) then
				table.insert(items, ToHDFirearm(item));
			end
		end
	end
	--Run this script for rogue weapons as well
	for item in MovableMan.Items do
		if IsHDFirearm(item) then
			table.insert(items, ToHDFirearm(item));
		end
	end
	for i = 1, #items do
		local weapon = items[i];
		if weapon and weapon.Magazine then
			--Stop weapons like Dihelical Cannon from misbehaving
			if weapon.ActivationDelay > 0 and weapon:IsActivated() and weapon.Magazine.Capacity == 1 and weapon.Magazine.RoundCount == 0 then
				weapon:Deactivate();
			end
			if weapon.Magazine.RoundCount == 0 or not weapon:IsActivated() then
				weapon.Magazine.RoundCount = weapon.Magazine.Capacity;
			end
		end
	end
end