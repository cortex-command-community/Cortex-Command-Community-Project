function Create(self)
	self.grenadeName = self:StringValueExists("GrenadeName") and self:GetStringValue("GrenadeName") or "Frag Grenade";
	self.grenadeTech = self:StringValueExists("GrenadeTech") and self:GetStringValue("GrenadeTech") or "Base.rte";

	self.grenadeIsThrownDevice = self:NumberValueExists("GrenadeIsThrownDevice");
	local appropriateGrenadeCreateFunction = self.grenadeIsThrownDevice and CreateThrownDevice or CreateTDExplosive;
	self.grenadeObject = appropriateGrenadeCreateFunction(self.grenadeName, self.grenadeTech);

	self.bandolierKey =  self.grenadeTech .. "/" .. self.PresetName;
	self.bandolierMass = self:NumberValueExists("BandolierMass") and self:GetNumberValue("BandolierMass") or 1.5;

	self.replenishDelay = self:NumberValueExists("ReplenishDelay") and self:GetNumberValue("ReplenishDelay") or 0;

	self.grenadeMass = self:NumberValueExists("GrenadeMass") and self:GetNumberValue("GrenadeMass") or self.grenadeObject.Mass;
	self.grenadesPerBandolier = self:NumberValueExists("GrenadeCount") and self:GetNumberValue("GrenadeCount") or 3;
	self.grenadesRemainingInBandolier = self:NumberValueExists("GrenadesRemainingInBandolier") and self:GetNumberValue("GrenadesRemainingInBandolier") or self.grenadesPerBandolier;
end

function OnAttach(self, newParent)
	local rootParent = self:GetRootParent();
	if IsAHuman(rootParent) then
		rootParent = ToAHuman(rootParent);
	end
	if rootParent and not rootParent:NumberValueExists(self.bandolierKey) then
		local attachable = CreateAttachable("Grenade Bandolier", "Base.rte");

		attachable:SetStringValue("BandolierName", self.PresetName);
		attachable:SetNumberValue("BandolierMass", self.bandolierMass);

		attachable:SetNumberValue("ReplenishDelay", self.replenishDelay);

		attachable:SetStringValue("GrenadeName", self.grenadeName);
		attachable:SetStringValue("GrenadeTech", self.grenadeTech);
		if self.grenadeIsThrownDevice then
			attachable:SetNumberValue("GrenadeIsThrownDevice", 1);
		end
		attachable:SetNumberValue("GrenadeMass", self.grenadeMass);
		attachable:SetNumberValue("GrenadesPerBandolier", self.grenadesPerBandolier);
		if self.grenadesPerBandolier ~= self.grenadesRemainingInBandolier then
			attachable:SetNumberValue("GrenadesRemainingInBandolier", self.grenadesRemainingInBandolier);
		end

		rootParent:AddAttachable(attachable);
		self.ToDelete = true;
	end
end