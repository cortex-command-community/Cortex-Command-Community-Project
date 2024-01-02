function GrapplePieRetract(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;
	if gun then
		ToMOSRotating(gun):SetNumberValue("GrappleMode", 1);
	end
end

function GrapplePieExtend(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;
	if gun then
		ToMOSRotating(gun):SetNumberValue("GrappleMode", 2);
	end
end