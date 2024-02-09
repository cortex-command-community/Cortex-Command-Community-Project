local modifyGrenadeCount = function(self, numberOfGrenadesToAddOrRemove, doNotDeleteAttachableIfThereAreNoMoreGrenades)
	-- This is probably an unnecessary safety check, but it may be possible for some combination of replenish delays and replenish gui time limits to result in wonky behaviour, so it's best to be extra safe.
	if self.currentGrenadeCount < 0 then
		self.ToDelete = true;
		return;
	end

	self.currentGrenadeCount = self.infiniteGrenades and 1 or (self.currentGrenadeCount + numberOfGrenadesToAddOrRemove);
	self.Mass = self.bandolierMass + (self.grenadeMass * self.currentGrenadeCount);
	self.rootParent:SetNumberValue(self.bandolierKey, self.currentGrenadeCount);
	self.rootParent:SetGoldValue(self.rootParent:GetGoldValue(self.rootParent.ModuleID, 1, 1) + (self.grenadeObjectGoldValue * numberOfGrenadesToAddOrRemove));

	if self.currentGrenadeCount <= 0 and not doNotDeleteAttachableIfThereAreNoMoreGrenades then
		self.ToDelete = true;
	end
end

-- Note: This function returns whether or not the replenished grenade was equipped.
local replenishGrenade = function(self, forceEquipGrenade)
	self.rootParent:AddInventoryItem(self.grenadeObject:Clone());

	if forceEquipGrenade or self.grenadeReplenishDelay < 100 then
		self:modifyGrenadeCount(-1);
		self.rootParent.UpperBodyState = AHuman.WEAPON_READY;
		-- Only actually equip the grenade if the root parent was previously holding one, or we're forcing it to equip it. This avoids issues when, for example, removing a grenade from the root parent's inventory via Lua.
		return (forceEquipGrenade or self.grenadePreviouslyHeldByRootParent) and self.rootParent:EquipNamedDevice(self.grenadeTech, self.grenadeName, true);
	else
		self.grenadeReplenishGUITimer:Reset();
		self:modifyGrenadeCount(-1, true);
	end
	return false;
end

function Create(self)
	self.modifyGrenadeCount = modifyGrenadeCount;
	self.replenishGrenade = replenishGrenade;

	self.DeleteWhenRemovedFromParent = true;

	local rootParent = self:GetRootParent();
	if not IsAHuman(rootParent) then
		self.ToDelete = true;
		return;
	end
	self.rootParent = ToAHuman(rootParent);
	self.rootParentController = self.rootParent:GetController();
	self.isHumanTeam = ActivityMan:GetActivity():IsHumanTeam(self.rootParent.Team);

	self.grenadeName = self:GetStringValue("GrenadeName");
	self.grenadeTech = self:GetStringValue("GrenadeTech");

	local appropriateGrenadeCreateFunction = self:NumberValueExists("GrenadeIsThrownDevice") and CreateThrownDevice or CreateTDExplosive;
	self.grenadeObject = appropriateGrenadeCreateFunction(self.grenadeName, self.grenadeTech);

	self.bandolierName = self:GetStringValue("BandolierName");
	self.bandolierKey = self.grenadeTech .. "/" .. self.bandolierName;
	self.bandolierMass = self:GetNumberValue("BandolierMass");

	self.grenadeReplenishDelay = self:GetNumberValue("ReplenishDelay");
	self.grenadeReplenishTimer = Timer();
	self.grenadeReplenishTimer:SetSimTimeLimitMS(self.grenadeReplenishDelay);

	self.grenadeMass = self:GetNumberValue("GrenadeMass");
	self.grenadesPerBandolier = self:GetNumberValue("GrenadesPerBandolier");
	if self.grenadesPerBandolier == -1 then
		self.infiniteGrenades = true;
	end
	self.grenadeObjectGoldValue = self.grenadeObject:GetGoldValue(self.grenadeObject.ModuleID, 1, 1);

	self.currentGrenadeCount = self.rootParent:GetNumberValue(self.bandolierKey);
	local grenadesToAdd = self:NumberValueExists("GrenadesRemainingInBandolier") and self:GetNumberValue("GrenadesRemainingInBandolier") or self.grenadesPerBandolier;
	self:RemoveNumberValue("GrenadesRemainingInBandolier");
	self:modifyGrenadeCount(self.currentGrenadeCount == 0 and grenadesToAdd or 0);

	self.grenadeAmmoIcon = CreateMOSParticle("Ammo Icon", "Base.rte");

	self.grenadeReplenishGUITimer = Timer();
	self.grenadeReplenishGUITimer:SetSimTimeLimitMS(1500);
	self.grenadeReplenishGUITimer.ElapsedSimTimeMS = self.grenadeReplenishGUITimer:GetSimTimeLimitMS() + 1000;
	self.grenadeReplenishIcon = self.grenadeObject;
	-- TODO maybe change sprite or at least sprite colour for refresh plus
	self.grenadeReplenishPlusIcon = CreateMOSParticle("Particle Heal Effect", "Base.rte");
	
	self.bandolierObjectForDropping = appropriateGrenadeCreateFunction(self.bandolierName, self.grenadeTech);
	
	local rootParentEquippedItemModuleAndPresetName = self.rootParent.EquippedItem ~= nil and self.rootParent.EquippedItem:GetModuleAndPresetName() or nil;
	local rootParentIsHoldingGrenade = rootParentEquippedItemModuleAndPresetName == self.grenadeObject:GetModuleAndPresetName();
	if rootParentEquippedItemModuleAndPresetName ~= self.grenadeObject:GetModuleAndPresetName() then
		self:replenishGrenade(true);
	end
end

function Update(self)
	if self.rootParent and self.rootParent.Health > 0 and MovableMan:ValidMO(self.rootParent) then
		local rootParentEquippedItemModuleAndPresetName = self.rootParent.EquippedItem ~= nil and self.rootParent.EquippedItem:GetModuleAndPresetName() or nil;
		local rootParentIsHoldingGrenade = rootParentEquippedItemModuleAndPresetName == self.grenadeObject:GetModuleAndPresetName();

		-- If the root parent is holding a grenade bandolier, merge it and replace it with a grenade.
		if rootParentEquippedItemModuleAndPresetName == self.bandolierKey then
			local rootParentEquippedItem = self.rootParent.EquippedItem;
			rootParentEquippedItem:RemoveFromParent();
			local bandolierGrenadeCount = rootParentEquippedItem:NumberValueExists("GrenadesRemainingInBandolier") and rootParentEquippedItem:GetNumberValue("GrenadesRemainingInBandolier") or self.grenadesPerBandolier;
			self:modifyGrenadeCount(bandolierGrenadeCount);
			rootParentIsHoldingGrenade = self:replenishGrenade(true);
		end

		local rootParentHasGrenadeInInventory = false;

		-- Merge any grenades or grenade bandoliers in the root parent's inventory.
		if self.rootParent:HasObject(self.grenadeName) or self.rootParent:HasObject(self.bandolierName) then
			local rootParentHasGrenadeOrBandolierSoAnyCopiesCanBeMerged = rootParentIsHoldingGrenade;
			for inventoryItem in self.rootParent.Inventory do
				if inventoryItem:GetModuleAndPresetName() == self.bandolierKey or inventoryItem:GetModuleAndPresetName() == self.grenadeObject:GetModuleAndPresetName() then
					rootParentHasGrenadeInInventory = rootParentHasGrenadeInInventory or inventoryItem.PresetName == self.grenadeName;
					if not rootParentHasGrenadeOrBandolierSoAnyCopiesCanBeMerged then
						rootParentHasGrenadeOrBandolierSoAnyCopiesCanBeMerged = true;
					else
						self:modifyGrenadeCount(inventoryItem.PresetName == self.bandolierName and self.grenadesPerBandolier or 1);
						self.rootParent:RemoveInventoryItem(self.grenadeTech, inventoryItem.PresetName);
					end
				end
			end
		end

		-- Draw the grenade ammo icon and text.
		if self.rootParent.HUDVisible and rootParentIsHoldingGrenade and self.rootParent:IsPlayerControlled() and not (self.rootParent.Jetpack and self.rootParent.Jetpack:IsEmitting()) then
			local distanceBetweenIconAndText = 2 + math.floor(self.grenadeAmmoIcon:GetSpriteWidth() * 0.5);
			local drawPosition = self.rootParent.AboveHUDPos + Vector(-distanceBetweenIconAndText, self.rootParentController:IsState(Controller.PIE_MENU_ACTIVE) and -1	 or 2);

			PrimitiveMan:DrawBitmapPrimitive(self.rootParentController.Player, drawPosition, self.grenadeAmmoIcon, math.pi, 0, true, true);
			drawPosition = drawPosition + Vector(distanceBetweenIconAndText, -self.grenadeAmmoIcon:GetSpriteHeight() * 0.5);
			PrimitiveMan:DrawTextPrimitive(self.rootParentController.Player, drawPosition, (self.infiniteGrenades and "Infinite" or tostring(self.currentGrenadeCount + 1)), true, 0);
		end

		-- Give the root parent a grenade if the timer is ready and they don't already have a copy of the grenade.
		if rootParentIsHoldingGrenade or rootParentHasGrenadeInInventory then
			self.grenadeReplenishTimer:Reset();
		elseif self.grenadeReplenishTimer:IsPastSimTimeLimit() then
			self:replenishGrenade();
			self.grenadeReplenishTimer:Reset();
		end

		-- Draw grenade replenish icons, and delete the Attachable in the case where the icons are finished drawing and the current grenade count is <= 0, but the Attachable needed to stick around to draw the icons.
		if self.isHumanTeam and self.HUDVisible and not self.grenadeReplenishGUITimer:IsPastSimTimeLimit() then
			local grenadeReplenishIconPos = self.rootParent.AboveHUDPos + Vector(25, 24);
			PrimitiveMan:DrawBitmapPrimitive(self.rootParentController.Player, grenadeReplenishIconPos, self.grenadeReplenishIcon, math.pi, 0, true, true);
			PrimitiveMan:DrawBitmapPrimitive(self.rootParentController.Player, grenadeReplenishIconPos + Vector(self.grenadeReplenishIcon:GetSpriteWidth(), 0), self.grenadeReplenishPlusIcon, math.pi, 0, true, true);
		elseif self.currentGrenadeCount <= 0 and self.grenadeReplenishGUITimer:IsPastSimTimeLimit() then
			self.ToDelete = true;
		end
		
		self.grenadePreviouslyHeldByRootParent = rootParentIsHoldingGrenade and self.rootParent.EquippedItem or nil;
	end
end

function Destroy(self)
	if self.rootParent and MovableMan:ValidMO(self.rootParent) then
		self.rootParent:RemoveNumberValue(self.bandolierKey);
	end
	if self.currentGrenadeCount > 0 then
		local bandolierPosition = self.Pos;
		local bandolierVel = self.Vel;
		local bandolierRotAngle = self.RotAngle;
		local bandolierAngularVel = self.AngularVel;
		
		if self.grenadePreviouslyHeldByRootParent then
			if MovableMan:ValidMO(self.grenadePreviouslyHeldByRootParent) and not self.grenadePreviouslyHeldByRootParent:IsActivated() then
				self.grenadePreviouslyHeldByRootParent.ToDelete = true;
				self:modifyGrenadeCount(1);
				
				bandolierPosition = self.grenadePreviouslyHeldByRootParent.Pos;
				bandolierVel = self.grenadePreviouslyHeldByRootParent.Vel;
				bandolierRotAngle = self.grenadePreviouslyHeldByRootParent.RotAngle;
				bandolierAngularVel = self.grenadePreviouslyHeldByRootParent.AngularVel;
			end
		end
		
		if self.currentGrenadeCount > 0 then
			self.bandolierObjectForDropping:SetNumberValue("GrenadesRemainingInBandolier", self.currentGrenadeCount);
			
			self.bandolierObjectForDropping.Pos = bandolierPosition;
			self.bandolierObjectForDropping.Vel = bandolierVel;
			self.bandolierObjectForDropping.RotAngle = bandolierRotAngle;
			self.bandolierObjectForDropping.AngularVel = bandolierAngularVel;
			
			self.bandolierObjectForDropping.Mass = self.bandolierMass + (self.grenadeMass * self.currentGrenadeCount);
			
			MovableMan:AddItem(self.bandolierObjectForDropping);
		end
	end
end