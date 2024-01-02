function Create(self)
	local replacementChance = 0.035;

	if math.random() < replacementChance then
		local oldStockReplacementPresetName = "Old Stock " .. self.PresetName;
		if self.PresetName == "Pistol" and math.random() <= 0.5 then
			oldStockReplacementPresetName = "Old Stock Blaster Pistol";
		end
		local oldStockReplacement = CreateHDFirearm(oldStockReplacementPresetName, self.ModuleName);
		
		if oldStockReplacement then
			if self:IsAttached() then
				local parent = self.Parent;
				local rootParent = self:GetRootParent();
				self:RemoveFromParent();
				
				if IsAHuman(rootParent) then
					rootParent = ToAHuman(rootParent);
					rootParent:AddInventoryItem(oldStockReplacement);
					rootParent:EquipNamedDevice(oldStockReplacement.ModuleName, oldStockReplacementPresetName, true);
				elseif IsACrab(rootParent) then
					ToACrab(rootParent).Turret:AddMountedDevice(oldStockReplacement);
				else
					parent:AddAttachable(oldStockReplacement, self.ParentOffset);
				end
			else
				oldStockReplacement.Pos = self.Pos;
				oldStockReplacement.Vel = self.Vel;
				oldStockReplacement.RotAngle = self.RotAngle;
				oldStockReplacement.AngularVel = self.AngularVel;
				MovableMan:AddItem(oldStockReplacement);
				self.ToDelete = true;
			end
		end
	end
end