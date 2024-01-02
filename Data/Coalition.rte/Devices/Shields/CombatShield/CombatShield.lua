local function updatePieSlicesAndSetToAutoAttachIfAppropriate(self)
	local rootParent = self:GetRootParent();
	if IsAHuman(rootParent) then
		rootParent = ToAHuman(rootParent);
		
		local attachedToGun = IsHDFirearm(self:GetParent());
		rootParent.PieMenu:GetFirstPieSliceByPresetName("Coalition Shield Detach PieSlice").Enabled = attachedToGun;
		rootParent.PieMenu:GetFirstPieSliceByPresetName("Coalition Shield Attach PieSlice").Enabled = not attachedToGun;
		
		if not attachedToGun and not rootParent:IsPlayerControlled() then
			self:SetNumberValue("AttachToAppropriateFirearm", 1);
		end
	end
end

local function attachToAppropriateHDFirearm(self)
	local parent = self:GetRootParent();
	
	if IsAHuman(parent) then
		parent = ToAHuman(parent);
		for device in parent.Inventory do
			if device.ModuleName == self.ModuleName and IsHDFirearm(device) and not ToHDFirearm(device):IsOneHanded() then
				device = ToHDFirearm(device);
				
				local canAttachCombatShield = true;
				for attachable in device.Attachables do
					if attachable:GetModuleAndPresetName() == "Coalition.rte/Combat Shield" then
						canAttachCombatShield = false;
					end
				end
				
				if canAttachCombatShield then
					self.Frame = 1;
					parent.PieMenu:RemovePieSlicesByOriginalSource(self);
					device:AddAttachable(self:Clone(), Vector(math.floor(device.MuzzleOffset.X * 0.5) - 2, device.MuzzleOffset.Y));
					if device:HasScript("Coalition.rte/Devices/Shields/CombatShield/Support.lua") then
						device:EnableScript("Coalition.rte/Devices/Shields/CombatShield/Support.lua");
					else
						device:AddScript("Coalition.rte/Devices/Shields/CombatShield/Support.lua");
					end
					self.ToDelete = true;
					parent:EquipNamedDevice(device.ModuleName, device.PresetName, true);
					return;
				end
			end
		end
		if parent:IsPlayerControlled() then
			CreateSoundContainer("Error", "Base.rte"):Play(self.Pos, parent:GetController().Player);
		end
	end
end

function Create(self)
	self.updatePieSlicesAndSetToAutoAttachIfAppropriate = updatePieSlicesAndSetToAutoAttachIfAppropriate;
	self.attachToAppropriateHDFirearm = attachToAppropriateHDFirearm;

	self:updatePieSlicesAndSetToAutoAttachIfAppropriate();
end

function Update(self)
	if self:NumberValueExists("AttachToAppropriateFirearm") then
		self:RemoveNumberValue("AttachToAppropriateFirearm");
		self:attachToAppropriateHDFirearm();
	end
end

function OnAttach(self, newParent)
	self:updatePieSlicesAndSetToAutoAttachIfAppropriate();
end

function CoalitionShieldAttach(pieMenuOwner, pieMenu, pieSlice)
	local deviceModuleAndPresetName = pieSlice.OriginalSource:GetModuleAndPresetName();
	
	local device = pieMenuOwner.EquippedItem;
	if device:GetModuleAndPresetName() ~= deviceModuleAndPresetName then
		device = pieMenuOwner.EquippedBGItem;
	end
	
	if device and IsHeldDevice(device) then
		ToHeldDevice(device):SetNumberValue("AttachToAppropriateFirearm", 1);
	end
end

function CoalitionShieldDetach(pieMenuOwner, pieMenu, pieSlice)
	local device = pieMenuOwner.EquippedItem;
	if device and IsHDFirearm(device) then
		device = ToHDFirearm(device);
		for attachable in device.Attachables do
			if attachable:GetModuleAndPresetName() == "Coalition.rte/Combat Shield" then
				pieMenuOwner.PieMenu:GetFirstPieSliceByPresetName("Coalition Shield Detach PieSlice").Enabled = false;
				pieMenuOwner.PieMenu:GetFirstPieSliceByPresetName("Coalition Shield Attach PieSlice").Enabled = true;
				if device:HasScript("Coalition.rte/Devices/Shields/CombatShield/Support.lua") then
					device:DisableScript("Coalition.rte/Devices/Shields/CombatShield/Support.lua");
				end
				attachable.Frame = 0;
				local removedAttachable = device:RemoveAttachable(attachable, false, false);
				pieMenuOwner:AddInventoryItem(removedAttachable:Clone())
				break;
			end
		end
	end
end