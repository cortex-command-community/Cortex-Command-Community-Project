function Create(self)
	self.grenadeTableA = {};
	self.grenadeTableB = {};
	self.maxActiveGrenades = 12;

	self.impactModePieSlice = nil;
	self.bounceModePieSlice = nil;
	self.remoteModePieSlice = nil;
	self.removeGrenadesPieSlice = nil;
	self.detonateGrenadesPieSlice = nil;
end

function OnAttach(self, newParent)
	local rootParent = self:GetRootParent();
	if IsActor(rootParent) and MovableMan:IsActor(rootParent) then
		local subPieMenuPieSlice = ToActor(rootParent).PieMenu:GetFirstPieSliceByPresetName("Coalition Grenade Launcher Options");
		if subPieMenuPieSlice ~= nil then
			local grenadeLauncherSubPieMenu = subPieMenuPieSlice.SubPieMenu;
			if grenadeLauncherSubPieMenu ~= nil then
				self.impactModePieSlice = grenadeLauncherSubPieMenu:GetFirstPieSliceByPresetName("Coalition Grenade Launcher Impact Mode");
				self.bounceModePieSlice = grenadeLauncherSubPieMenu:GetFirstPieSliceByPresetName("Coalition Grenade Launcher Bounce Mode");
				self.remoteModePieSlice = grenadeLauncherSubPieMenu:GetFirstPieSliceByPresetName("Coalition Grenade Launcher Remote Mode");
				self.removeGrenadesPieSlice = grenadeLauncherSubPieMenu:GetFirstPieSliceByPresetName("Coalition Grenade Launcher Remove Grenades");
				self.detonateGrenadesPieSlice = grenadeLauncherSubPieMenu:GetFirstPieSliceByPresetName("Coalition Grenade Launcher Detonate Grenades");
			end
		end
	end
end

function WhilePieMenuOpen(self, pieMenu)
	local grenadeMode = self:GetStringValue("GrenadeMode");
	if self.impactModePieSlice ~= nil then
		self.impactModePieSlice.Enabled = grenadeMode ~= "Impact";
	end
	if self.bounceModePieSlice ~= nil then
		self.bounceModePieSlice.Enabled = grenadeMode ~= "Bounce";
	end
	if self.remoteModePieSlice ~= nil then
		self.remoteModePieSlice.Enabled = grenadeMode ~= "Remote";
	end

	local grenadeCount = self:GetNumberValue("CoalitionRemoteGrenades");
	if self.removeGrenadesPieSlice ~= nil then
		self.removeGrenadesPieSlice.Enabled = grenadeCount > 0;
	end
	if self.detonateGrenadesPieSlice ~= nil then
		self.detonateGrenadesPieSlice.Enabled = grenadeCount > 0;
	end
end

function Update(self)
	if self.FiredFrame then
		if self.Magazine then
			self.Magazine.Frame = (self.Magazine.Frame + 1) % self.Magazine.FrameCount;	--To-do: add animation frames for the Magazine
		end

		local mode = self:GetStringValue("GrenadeMode");
		local bullet = CreateActor("Coalition Grenade Launcher Shot", "Coalition.rte");
		bullet.Pos = self.MuzzlePos;
		bullet.Vel = self.Vel + Vector(35 * self.FlipFactor, 0):RadRotate(self.RotAngle):DegRotate(self.ShakeRange * 0.5 - (self.ShakeRange * math.random()));

		local actor = self:GetRootParent();
		if MovableMan:IsActor(actor) then
			actor = ToActor(actor);
			if actor:GetController():IsState(Controller.AIM_SHARP) then
				bullet.Vel = self.Vel + Vector(35 * self.FlipFactor, 0):RadRotate(self.RotAngle):DegRotate(self.SharpShakeRange * 0.5 - (self.SharpShakeRange * math.random()));
			end
			
			if mode == "Remote" then
				if actor:IsPlayerControlled() then
					if self.grenadeTableA[self.maxActiveGrenades] then
						self.grenadeTableA[self.maxActiveGrenades]:SetStringValue("GrenadeMode", "Delete");
					end
					for i = 1, self.maxActiveGrenades do
						self.grenadeTableB[i + 1] = self.grenadeTableA[i];
					end
					self.grenadeTableA = {};
					for i = 1, self.maxActiveGrenades do
						self.grenadeTableA[i] = self.grenadeTableB[i];
					end
					self.grenadeTableB = {};
					self.grenadeTableA[1] = bullet;
					self:SetNumberValue("CoalitionRemoteGrenades", #self.grenadeTableA);
				else
					mode = "Timed";
				end
			end
		end
		bullet.Team = actor.Team;
		bullet.IgnoresTeamHits = true;
		bullet:SetStringValue("GrenadeMode", mode);
		MovableMan:AddParticle(bullet);
	end

	if self:StringValueExists("GrenadeTrigger") then
		local trigger = self:GetStringValue("GrenadeTrigger");
		for i = 1, #self.grenadeTableA do
			if MovableMan:IsParticle(self.grenadeTableA[i]) then
				self.grenadeTableA[i]:SetStringValue("GrenadeMode", trigger);
			end
		end

		self.grenadeTableA = {};
		self:RemoveNumberValue("CoalitionRemoteGrenades");
		self:RemoveStringValue("GrenadeTrigger");
	end
end

function Destroy(self)
	for i = 1, #self.grenadeTableA do
		if MovableMan:IsParticle(self.grenadeTableA[i]) then
			self.grenadeTableA[i]:SetStringValue("GrenadeMode", "Delete");
		end
	end
end