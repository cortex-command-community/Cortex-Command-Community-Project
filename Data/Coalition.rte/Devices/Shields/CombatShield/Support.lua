function OnAttach(self, newParent)
	local hasShield = false;
	local parent = self:GetRootParent();
	if IsActor(parent) then
		parent = ToActor(parent);
		for attachable in self.Attachables do
			if attachable:GetModuleAndPresetName() == "Coalition.rte/Combat Shield" then
				parent.PieMenu:GetFirstPieSliceByPresetName("Coalition Shield Detach PieSlice").Enabled = true;
				parent.PieMenu:GetFirstPieSliceByPresetName("Coalition Shield Attach PieSlice").Enabled = false;
				hasShield = true;
				break;
			end
		end
	end
	if not hasShield then
		self:DisableScript();
	end
end