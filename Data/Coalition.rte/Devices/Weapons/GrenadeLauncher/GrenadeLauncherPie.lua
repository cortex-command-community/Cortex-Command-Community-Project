function GrenadeLauncherImpact(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;
	if gun then
		ToMOSRotating(gun):SetStringValue("GrenadeMode", "Impact");

		pieSlice.Enabled = false;
		local pieSlicePresetNamesToEnable = {"Coalition Grenade Launcher Bounce Mode", "Coalition Grenade Launcher Remote Mode"};
		for _, pieSlicePresetNameToEnable in pairs(pieSlicePresetNamesToEnable) do
			local pieSliceToEnable = pieMenu:GetFirstPieSliceByPresetName(pieSlicePresetNameToEnable);
			if pieSliceToEnable ~= nil then
				pieSliceToEnable.Enabled = true;
			end
		end
	end
end

function GrenadeLauncherBounce(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;
	if gun then
		ToMOSRotating(gun):SetStringValue("GrenadeMode", "Bounce");

		pieSlice.Enabled = false;
		local pieSlicePresetNamesToEnable = {"Coalition Grenade Launcher Impact Mode", "Coalition Grenade Launcher Remote Mode"};
		for _, pieSlicePresetNameToEnable in pairs(pieSlicePresetNamesToEnable) do
			local pieSliceToEnable = pieMenu:GetFirstPieSliceByPresetName(pieSlicePresetNameToEnable);
			if pieSliceToEnable ~= nil then
				pieSliceToEnable.Enabled = true;
			end
		end
	end
end

function GrenadeLauncherRemote(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;
	if gun then
		ToMOSRotating(gun):SetStringValue("GrenadeMode", "Remote");

		pieSlice.Enabled = false;
		local pieSlicePresetNamesToEnable = {"Coalition Grenade Launcher Impact Mode", "Coalition Grenade Launcher Bounce Mode"};
		for _, pieSlicePresetNameToEnable in pairs(pieSlicePresetNamesToEnable) do
			local pieSliceToEnable = pieMenu:GetFirstPieSliceByPresetName(pieSlicePresetNameToEnable);
			if pieSliceToEnable ~= nil then
				pieSliceToEnable.Enabled = true;
			end
		end
	end
end

function GrenadeLauncherRemoteDetonate(pieMenuOwner, pieMenu, pieSlice)
	ToHDFirearm(ToAHuman(pieMenuOwner).EquippedItem):SetStringValue("GrenadeTrigger", "Detonate");
end

function GrenadeLauncherRemoteDelete(pieMenuOwner, pieMenu, pieSlice)
	ToHDFirearm(ToAHuman(pieMenuOwner).EquippedItem):SetStringValue("GrenadeTrigger", "Delete");
end