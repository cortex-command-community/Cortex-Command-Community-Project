function ConstructorModeCancel(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;

	if gun and IsMOSRotating(gun) then
		gun = ToMOSRotating(gun);
		gun:SetNumberValue("BuildMode", 1);
	end
end

function ConstructorModeConstruction(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;

	if gun and IsMOSRotating(gun) then
		gun = ToMOSRotating(gun);
		gun:SetNumberValue("BuildMode", 2);
	end
end

function ConstructorModeExcavation(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;

	if gun and IsMOSRotating(gun) then
		gun = ToMOSRotating(gun);
		gun:SetNumberValue("BuildMode", 2);
	end
end

function ConstructorDigMode(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;

	if gun and IsMOSRotating(gun) then
		gun = ToMOSRotating(gun);
		gun:SetStringValue("ConstructorMode", "Dig");
		pieMenu:ReplacePieSlice(pieSlice, CreatePieSlice("Constructor Spray Mode", "Base.rte"));
		pieMenu:ReplacePieSlice(pieMenu:GetFirstPieSliceByPresetName("Constructor Order Construction"), CreatePieSlice("Constructor Order Excavation", "Base.rte"));
	end
end

function ConstructorSprayMode(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;

	if gun and IsMOSRotating(gun) then
		gun = ToMOSRotating(gun);
		gun:SetStringValue("ConstructorMode", "Spray");
		pieMenu:ReplacePieSlice(pieSlice, CreatePieSlice("Constructor Dig Mode", "Base.rte"));
		pieMenu:ReplacePieSlice(pieMenu:GetFirstPieSliceByPresetName("Constructor Order Excavation"), CreatePieSlice("Constructor Order Construction", "Base.rte"));
	end
end