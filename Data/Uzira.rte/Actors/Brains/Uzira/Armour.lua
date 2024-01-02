function OnAttach(self, parent)
	if parent then
		if IsLeg(parent) and ToLeg(parent).Foot then
			if parent:HasScript("Uzira.rte/Actors/Brains/Uzira/UziraLegReplaceFootToMatchArmour.lua") then
				parent:EnableScript("Uzira.rte/Actors/Brains/Uzira/UziraLegReplaceFootToMatchArmour.lua");
			else
				parent:AddScript("Uzira.rte/Actors/Brains/Uzira/UziraLegReplaceFootToMatchArmour.lua");
			end
		elseif IsArm(parent) then
			self.origWoundName = ToArm(parent):GetEntryWoundPresetName();
			ToArm(parent):SetEntryWound("Dent Metal Light", "Base.rte");
		end
	end
end
function OnDetach(self, parent)
	if parent then
		if IsLeg(parent) and ToLeg(parent).Foot then
			if parent:HasScript("Uzira.rte/Actors/Brains/Uzira/UziraLegReplaceFootToMatchArmour.lua") then
				parent:EnableScript("Uzira.rte/Actors/Brains/Uzira/UziraLegReplaceFootToMatchArmour.lua");
			else
				parent:AddScript("Uzira.rte/Actors/Brains/Uzira/UziraLegReplaceFootToMatchArmour.lua");
			end
		elseif IsArm(parent) then
			ToArm(parent):SetEntryWound(self.origWoundName, "Base.rte");
		end
	end
end