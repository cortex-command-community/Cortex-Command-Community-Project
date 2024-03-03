--------------------------------------- Instructions ---------------------------------------

-- If using weird tech/virtual team setups you may wanna do this:SendMessage("ActorSpawner_ReplaceDeliveryCreationHandler", instancedDeliveryCreationHandler)
-- ActorSpawner_ManualTriggerAndReturnActor will manually trigger the Actor Spawner and also immediately send a message back to the object, with the object being the actor UniqueID that is
-- about to spawn. Handling it at the end point is your responsibility.
-- If it's a GlobalMessage, the returned actor will go to the Activity.

-- For anything else, just consult the .ini value setup.

local ActorSpawnerFunctions = {};

function ActorSpawnerFunctions.PrepareNewActor(self)
	self.nextActor = nil;
	local randomInfantryTypeTable = {"Light", "Medium", "Heavy", "CQB", "Scout", "Sniper", "Grenadier"};		
	local actor;
	
	if self.spawnType == "Specific" then
		local createFunc = "Create" .. self.specificActorClassName;	
		actor = _G[createFunc](self.specificActorPresetName, self.specificActorTechName);
		if self.spawnEquipment then
			-- create a dummy actor that we only really want the equipment of
			local equippedActor = self.deliveryCreationHandler:CreateSquad(self.customTeamToUseForDCHGeneration, 1, randomInfantryTypeTable[math.random(1, #randomInfantryTypeTable)])[1];
			if equippedActor then
				while equippedActor.InventorySize > 0 do
					actor:AddInventoryItem(equippedActor:RemoveInventoryItemAtIndex(0));
				end
			end
		end	
	elseif self.spawnType == "Random" then
		actor = self.deliveryCreationHandler:CreateSquad(self.customTeamToUseForDCHGeneration, 1, randomInfantryTypeTable[math.random(1, #randomInfantryTypeTable)])[1];
	else -- DCH type, hopefully
		actor = self.deliveryCreationHandler:CreateSquad(self.customTeamToUseForDCHGeneration, 1, self.spawnType)[1];
	end
	
	actor.Pos = self.Pos;
	actor.Team = self.Team;
	self.nextActor = actor;
end
	

function OnGlobalMessage(self, message, object)
	if message == "DEACTIVATEALLACTORSPAWNERS" then
		self.Deactivated = true;
		self:SetNumberValue("Deactivated", 1);
	elseif message == "ACTIVATEALLACTORSPAWNERS" then
		self.Deactivated = false;
		self:RemoveNumberValue("Deactivated");
	elseif message == self.deactivationMessage then
		self.Deactivated = true;
		self:SetNumberValue("Deactivated", 1);
	elseif message == self.activationMessage then
		self.Deactivated = false;
		self:RemoveNumberValue("Deactivated");	
	elseif message  == "ActorSpawner_ReplaceDeliveryCreationHandler" then
		--self.deliveryCreationHandler = object;
	elseif message == "ActorSpawner_ManualTrigger" then
		if object == true or not self.Deactivated then
			self.manualTrigger = true;
		end
	elseif message == "ActorSpawner_ManualTriggerAndReturnActor" then
		self.manualTrigger = true;
		self.Activity:SendMessage("ActorSpawner_ReturnedActor", self.nextActor.UniqueID);
	end
end

function OnMessage(self, message, object)
	if message == "DEACTIVATEALLACTORSPAWNERS" then
		self.Deactivated = true;
		self:SetNumberValue("Deactivated", 1);
	elseif message == "ACTIVATEALLACTORSPAWNERS" then
		self.Deactivated = false;
		self:RemoveNumberValue("Deactivated");
	elseif message == self.deactivationMessage then
		self.Deactivated = true;
		self:SetNumberValue("Deactivated", 1);
	elseif message == self.activationMessage then
		self.Deactivated = false;
		self:RemoveNumberValue("Deactivated");	
	elseif message  == "ActorSpawner_ReplaceDeliveryCreationHandler" then
		print("replaced deliverycreationahndler")
		print(object)
		print(#object)
		--self.deliveryCreationHandler = object;
	elseif message == "ActorSpawner_ManualTrigger" then
		if object == true or not self.Deactivated then
			self.manualTrigger = true;
		end
	elseif message == "ActorSpawner_ManualTriggerAndReturnActor" then
		print("manualtriggerandreturn");
		self.manualTrigger = true;
		if object == "Activity" then
			object = self.Activity;
		end
		object:SendMessage("ActorSpawner_ReturnedActor", self.nextActor.UniqueID);
	end
end

function Create(self)
	self.Activity = ToGameActivity(ActivityMan:GetActivity());
	
	self.Deactivated = self:GetNumberValue("StartDeactivated") == 1 and true or false;
	if self:NumberValueExists("savedDeactivation") then
		self.Deactivated = self:GetNumberValue("savedDeactivation") == 1 and true or false;
	end
	if self.Deactivated then
		self:SetNumberValue("Deactivated", 1);
	else
		self:RemoveNumberValue("Deactivated");
	end
	
	self.deliveryCreationHandler = require("Activities/Utility/DeliveryCreationHandler");
	self.deliveryCreationHandler:Initialize(self.Activity, true);
	
	self.useGlobalMessaging = self:GetNumberValue("SendSpawnedMessageGlobally") == 1 and true or false;
	self.messageOnSpawn = self:GetStringValue("messageOnSpawn");
	
	self.deactivationMessage = self:GetStringValue("DeactivationMessage");
	self.activationMessage = self:GetStringValue("ActivationMessage");
	
	self.triggerType = self:GetStringValue("TriggerType");
	self.triggerRange =  self:NumberValueExists("TriggerRange") and self:GetNumberValue("TriggerRange") or 300;
	self.triggerBehavior = self:GetStringValue("TriggerBehavior");
	self.Scene = SceneMan.Scene;
	if self:StringValueExists("SceneTriggerArea") then
		if self.Scene:HasArea(self:GetStringValue("SceneTriggerArea")) then
			self.triggerArea = self.Scene:GetArea(self:GetStringValue("SceneTriggerArea"));
		end
	end
	
	self.actorCheckTimer = Timer();
	self.actorCheckDelay = 250;
	
	self.specificActorPresetName = self:StringValueExists("ActorPresetName") and self:GetStringValue("ActorPresetName") or nil;
	self.specificActorClassName = self:StringValueExists("ActorClassName") and self:GetStringValue("ActorClassName") or nil;
	self.specificActorClassName = self:StringValueExists("ActorTechName") and self:GetStringValue("ActorTechName") or nil;
	self.spawnEquipment = self:GetNumberValue("SpawnEquipment") == 1 and true or false;

	self.spawnType = self:GetStringValue("SpawnType");
	self.spawnAmount = self:NumberValueExists("SpawnAmount") and self:GetNumberValue("SpawnAmount") or 5;
	if self:NumberValueExists("savedSpawnAmount") then
		self.spawnAmount = self:GetNumberValue("savedSpawnAmount");
		self:RemoveNumberValue("savedSpawnAmount");
	end
	self.spawnTimer = Timer();
	self.spawnDelay = self:NumberValueExists("SpawnTimer") and self:GetNumberValue("SpawnTimer") or 5000;
	self.customTeamToUseForDCHGeneration = (self:NumberValueExists("CustomTeamToUseForDCHGeneration") and self:GetNumberValue("CustomTeamToUseForDCHGeneration") ~= -2)
										   and self:GetNumberValue("CustomTeamToUseForDCHGeneration") or self.Team;
										  
	ActorSpawnerFunctions.PrepareNewActor(self);
end

function ThreadedUpdate(self)

	if self.Activity.ActivityState == Activity.EDITING then
		self.Frame = 0;
	else
		self.Frame = 1;
	end

	if self.triggerType ~= "ManualOnly" then
		self.Triggered = false;
		
		if self.triggerType == "Constant" then
			self.Triggered = true;
		elseif self.actorCheckTimer:IsPastSimMS(self.actorCheckDelay) then
			self.actorCheckTimer:Reset();
			
			local ownTeamActors = 0;
			local enemyTeamActors = 0;
			local totalActors = 0;
			
			if self.triggerType == "Area" then
				for box in self.triggerArea.Boxes do
					for actor in MovableMan:GetMOsInBox(box, -1, true) do
						if IsActor(actor) then
							totalActors = totalActors + 1;
							if actor.Team == self.Team then
								ownTeamActors = ownTeamActors + 1;
							else
								enemyTeamActors = enemyTeamActors + 1;
							end
						end
					end
				end
			else -- ought be Range
				local actorList = MovableMan:GetMOsInRadius(self.Pos, self.triggerRange, -1, true);
				for actor in actorList do
					if IsActor(actor) then
						totalActors = totalActors + 1;
						if actor.Team == self.Team then
							ownTeamActors = ownTeamActors + 1;
						else
							enemyTeamActors = enemyTeamActors + 1;
						end
					end
				end
			end
		end
		
		if self.triggerBehavior == "OnlyOwnTeam" and ownTeamActors > 0 and enemyTeamActors == 0 then
			self.Triggered = true;
		elseif self.triggerBehavior == "OwnTeam" and ownTeamActors > 0 then
			self.Triggered = true;
		elseif self.triggerBehavior == "EnemyTeam" and enemyTeamActors > 0 then
			self.Triggered = true;
		elseif self.triggerBehavior == "Empty" and totalActors == 0 then
			self.Triggered = true;
		end
	end
	
	if (self.Triggered and self.spawnTimer:IsPastSimMS(self.spawnDelay)) or self.manualTrigger then
		self.spawnTimer:Reset();
		
		if self.spawnAmount ~= 0 or self.manualTrigger then			
			
			MovableMan:AddActor(self.nextActor);
			self.manualTrigger = false;
			self.spawnAmount = self.spawnAmount == -1 and -1 or math.max(0, self.spawnAmount - 1);
			
			if self.spawnAmount == 0 then
				self.Deactivated = true;
				self:SetNumberValue("Deactivated", 1);
			end
			
			ActorSpawnerFunctions.PrepareNewActor(self);
			
			self.shouldSendSpawnMessage = true;
			self:RequestSyncedUpdate();
		end
	end
end

function SyncedUpdate(self)
	if self.shouldSendSpawnMessage then
		if self.useGlobalMessaging then
			MovableMan:SendGlobalMessage(self.messageOnSpawn, self.spawnAmount);
		else
			self.Activity:SendMessage(self.messageOnSpawn, self.spawnAmount);
		end

		self.shouldSendSpawnMessage = false;
	end
end

function OnSave(self)
	self:RemoveNumberValue("Deactivated");
	self:SetNumberValue("savedSpawnAmount", self.spawnAmount);
	self:SetNumberValue("savedDeactivation", self.Deactivated and 1 or 0);
end










