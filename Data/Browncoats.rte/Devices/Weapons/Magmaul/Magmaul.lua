function OnAttach(self, newParent)
	local rootParent = self:GetRootParent();
	if IsActor(rootParent) and MovableMan:IsActor(rootParent) then
		local magazinePresetName = self.Magazine and self.Magazine.PresetName or self:GetNextMagazineName();
		local pieSliceToAddPresetName = magazinePresetName == "Magazine GL-1 Fuel" and "Magmaul Fire Bomb" or "Magmaul Fuel Bomb";
		ToActor(rootParent).PieMenu:AddPieSliceIfPresetNameIsUnique(CreatePieSlice(pieSliceToAddPresetName, self.ModuleName), self);
	end
end