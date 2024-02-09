function OnMessage(self, message)
	if message == self.deactivationMessage or message == "DEACTIVATEALLITEMDISPENSERS" then
		self.Deactivated = true;
	elseif message == self.activationMessage or message == "ACTIVATEALLITEMDISPENSERS" then
		self.Deactivated = false;
	end
end

function OnGlobalMessage(self, message)
	if message == self.deactivationMessage or message == "DEACTIVATEALLITEMDISPENSERS" then
		self.Deactivated = true;
	elseif message == self.activationMessage or message == "ACTIVATEALLITEMDISPENSERS" then
		self.Deactivated = false;
	end
end

function Create(self)
	self.deactivationMessage = self:GetStringValue("DeactivationMessage");
	self.activationMessage = self:GetStringValue("ActivationMessage");
	
	if self:GetNumberValue("Deactivated") == 1 then
		self.Deactivated = true;
	end
	
	self:RemoveNumberValue("Deactivated");
	
	local createFunc = "Create" .. self:GetStringValue("ItemToDispenseClassName");
	self.itemToDispense = _G[createFunc](self:GetStringValue("ItemToDispensePresetName"), self:GetStringValue("ItemToDispenseTechName"));
	
	self.Activity = ToGameActivity(ActivityMan:GetActivity());
	
	self.cooldownTimer = Timer();
	if self:NumberValueExists("cooldownTimer") then
		self.cooldownTimer.ElapsedRealTimeMS = self:GetNumberValue("cooldownTimer");
		self:RemoveNumberValue("cooldownTimer");
	end
	
	self.cooldownTime = self:GetNumberValue("CooldownTime");
	
	if self:NumberValueExists("itemsDispensed") then
		self.itemsDispensed = self:GetNumberValue("itemsDispensed");
		self:RemoveNumberValue("itemsDispensed");
	else
		self.itemsDispensed = 0;
	end
	
	self.itemLimit = self:GetNumberValue("ItemLimit");
	
	self.Message = "";
	self.messageTimer = Timer();
	self.messageTime = 0;
	
	self.closeActorTable = {};
	self.actorUpdateTimer = Timer();
	self.actorUpdateDelay = 50;
	
	self.detectRange = 50;
	
	self.usePieSlice = CreatePieSlice("Item Dispenser Use", "Base.rte");
end

function ThreadedUpdate(self)
	if self.actorUpdateTimer:IsPastSimMS(self.actorUpdateDelay) then
		for actor in MovableMan:GetMOsInRadius(self.Pos, self.detectRange) do
			if (not self.Deactivated) and (not self.closeActorTable[actor.UniqueID]) and IsAHuman(actor) or IsACrab(actor) then
				actor = ToActor(actor);
				self.closeActorTable[actor.UniqueID] = actor.UniqueID;
			end
		end
		
		self:RequestSyncedUpdate();
	end
	
	if not self.cooldownTimer:IsPastSimMS(self.cooldownTime) then
		PrimitiveMan:DrawTextPrimitive(self.Pos + Vector(0, -20), tostring(self.cooldownTime - self.cooldownTimer.ElapsedSimTimeMS), true, 1);
	end
	
	if not self.messageTimer:IsPastSimMS(self.messageTime) then
		PrimitiveMan:DrawTextPrimitive(self.Pos, self.Message, true, 1);
	end
	
	--PrimitiveMan:DrawTextPrimitive(self.Pos + Vector(0, 20), "Team: " .. self.Team, true, 1);
end

function SyncedUpdate(self)

	if self.actorUpdateTimer:IsPastSimMS(self.actorUpdateDelay) then
		self.actorUpdateTimer:Reset();
		
		for k, v in pairs(self.closeActorTable) do
			local actor = MovableMan:FindObjectByUniqueID(v);
			if actor and MovableMan:ValidMO(actor) then
				actor = ToActor(actor);
				local dist = SceneMan:ShortestDistance(self.Pos, actor.Pos, true);
				if self.Deactivated or dist:MagnitudeIsGreaterThan(self.detectRange) then
					actor.PieMenu:RemovePieSlicesByPresetName(self.usePieSlice.PresetName);
					actor:RemoveNumberValue("ItemDispenser_Use");
					self.closeActorTable[k] = nil;
				else
					actor.PieMenu:AddPieSliceIfPresetNameIsUnique(self.usePieSlice, self);
					if actor:NumberValueExists("ItemDispenser_Use") then
						actor:RemoveNumberValue("ItemDispenser_Use");
						if not self.Deactivated and self.cooldownTimer:IsPastSimMS(self.cooldownTime) then
							
							local team = actor.Team;
							local item = self.itemToDispense:Clone();
							item.Pos = self.Pos;
							item.Team = team
							MovableMan:AddMO(item);
							
							self.itemsDispensed = self.itemsDispensed + 1;
							
							if self.itemLimit >= 0 and self.itemsDispensed >= self.itemLimit then
								self.Deactivated = true;
							end
							
							self.cooldownTimer:Reset();
							
						else
						
							if self.Deactivated then
								if self.itemLimit >= 0 and self.itemsDispensed >= self.itemLimit then
									self.messageTimer:Reset();
									self.messageTime = 3000;
									self.Message = "Out of stock!";									
								else
									self.messageTimer:Reset();
									self.messageTime = 3000;
									self.Message = "Out of order!";
								end
							else
								self.messageTimer:Reset();
								self.messageTime = 3000;
								self.Message = "Not ready yet!";
							end
						end
						
					end
				end
			else
				self.closeActorTable[k] = nil;
			end
		end
	end
end

function OnSave(self)
	self:SetNumberValue("cooldownTimer", self.cooldownTimer.ElapsedRealTimeMS);
	self:SetNumberValue("itemsDispensed", self.itemsDispensed);
	self:SetNumberValue("Deactivated", self.Deactivated and 1 or 0);
end