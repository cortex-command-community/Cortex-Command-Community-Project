function Create(self)
	self.alliedTeam = -1;
end

function Update(self)
	local parent = self:GetRootParent();
	if parent and IsAHuman(parent) then
		self.alliedTeam = ToAHuman(parent).Team;
		self.user = ToAHuman(parent);
	elseif self:IsActivated() then

		local explosive = CreateMOSRotating(self.PresetName .. " Active");
		explosive.Pos = self.Pos;
		explosive.Vel = self.Vel;
		explosive.RotAngle = self.Vel.AbsRadAngle + (self.HFlipped and math.pi * self.FlipFactor or 0);
		explosive.HFlipped = self.HFlipped;
		explosive.Sharpness = self.alliedTeam;
		MovableMan:AddParticle(explosive);

		if self.user and IsAHuman(self.user) then
			if self.user:IsPlayerControlled() then
				if not self.user:HasObject("Detonator") then
					self.user:AddInventoryItem(CreateHDFirearm("Base.rte/Detonator"));
				end
				if not self.user:EquipNamedDevice(self.PresetName, true) then
					self.user:EquipNamedDevice("Detonator", true);
				end
			else
				explosive:SetNumberValue("AutoDetonate", 1);
			end
		end
		self.ToDelete = true;
	else
		self.user = nil;
	end
end