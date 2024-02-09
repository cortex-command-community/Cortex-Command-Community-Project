function OnMessage(self, message, context)
	if message == "BuyDoor_CustomTableOrder" then
		self.Unusable = true;
		local finalOrder = BuyDoorSetupOrder(self, context, true);
		
		if finalOrder then
			self.orderTimer:Reset();
			self.orderDelivering = true;
		else
			print("Buy Door was given a custom table order, but it had no items!");
		end
	end
		
	if message == "BuyDoor_ChangeCooldownTime" then
		self.cooldownTime = context;
		self:SetNumberValue("CooldownTime", context);
	end
end

function BuyDoorSetupOrder(self, orderList, isCustomOrder, team)
	local preActorItemList = {};
	local lastActor
	local finalOrder = {};

	if isCustomOrder then
		self.currentTeam = -1;
		for item in self.Inventory do
			if IsActor(item) then
				self.currentTeam = item.Team;
				break;
			end
		end
		
		-- hopefully added to inv for us...
		-- for i = 1, #orderList do
			-- local item = orderList[i];
			-- print(item)
			-- table.insert(finalOrder, item);
			-- self.currentTeam = item.Team;
		-- end
		
		return finalOrder;
	else
		for item in orderList do
			local class = item.ClassName;
			local typeCast = "To" .. class
			
			local clonedItem = _G[typeCast](item):Clone();
			if IsActor(clonedItem) and team then
				clonedItem.Team = team;
			end

			if IsAHuman(item) then
				lastActor = clonedItem;
				if preActorItemList and #preActorItemList > 0 then
					for k, preActorItem in ipairs(preActorItemList) do
						lastActor:AddInventoryItem(preActorItem);
					end
					preActorItemList = nil;
				end

				table.insert(finalOrder, lastActor);
			elseif IsActor(item) then			
				item = clonedItem;
				table.insert(finalOrder, item);
			elseif IsHeldDevice(item) then
				item = clonedItem;
				if lastActor then
					ToAHuman(lastActor):AddInventoryItem(item);
				else
					table.insert(preActorItemList, item);
				end
			else
				print("Buy Door was given an order item with a class it couldn't handle: " .. item);
			end
		end
	end
	
	-- No AHumans could take the items we bought
	if preActorItemList and #preActorItemList > 0 then
		for k, preActorItem in ipairs(preActorItemList) do
			table.insert(finalOrder, preActorItem);
		end

		preActorItemList = nil;
	end
	
	if #finalOrder == 0 then
		self.Message = "Nothing to order!"
		self.messageTime = 4000;
		self.messageTimer:Reset();
		return nil;
	end
	
	-- Finally, add the order to our inventory
	
	for k, item in pairs(finalOrder) do
		self:AddInventoryItem(item);
	end
	
	self.currentTeam = team;
	
	return finalOrder;
end

function Create(self)
	-- hoo boy... the things we do for draw order
	local us = MovableMan:RemoveActor(self);
	if us then -- fixed issue when reloading scripts
		MovableMan:AddParticle(us);
	end

	self.saveLoadHandler = require("Activities/Utility/SaveLoadHandler");
	self.saveLoadHandler:Initialize(self);
	
	if self:StringValueExists("savedConsoleMO") then
		self.console = self.saveLoadHandler:LoadLocallySavedMO(self, "savedConsoleMO");
	end
	
	if not console then -- just in case the above fails
		self.console = CreateMOSRotating("Buy Door Console", "Base.rte");
		self.console.Pos = self.Pos + Vector(0, -26);
		self.console.Team = self.Team;
		
		MovableMan:AddParticle(self.console);
	end

	-- Frame 0 is used to display the control console that we will place
	if self.Frame == 0 then
		self.Frame = 1;
	end

	self.isStayingOpen = false;
	if self:GetNumberValue("isStayingOpen") == 1 then
		self.isStayingOpen = true;
	else
		self.isStayingOpen = false;
	end

	self.isClosing = false;

	self.stayOpenTimer = Timer();
	self.stayOpenTimer:SetSimTimeLimitMS(self:NumberValueExists("StayOpenDuration") and self:GetNumberValue("StayOpenDuration") or 2000);

	self.openCloseSound = CreateSoundContainer("Background Door Open Close Sound", "Base.rte");
	self.openCloseSound.Pos = self.Pos;
	self.openCloseSound:Play();
	
	self.Activity = ToGameActivity(ActivityMan:GetActivity());
	
	self.cooldownTimer = Timer();
	if self:NumberValueExists("cooldownTimer") then
		self.cooldownTimer.ElapsedRealTimeMS = self:GetNumberValue("cooldownTimer");
		self:RemoveNumberValue("cooldownTimer");
	end

	self.cooldownTime = self:NumberValueExists("CooldownTime") and self:GetNumberValue("CooldownTime") or 3000;
	
	self.orderTimer = Timer();
	if self:NumberValueExists("orderTimer") then
		self.cooldownTimer.ElapsedRealTimeMS = self:GetNumberValue("orderTimer");
		self:RemoveNumberValue("orderTimer");
	end

	self.orderDelay = self:NumberValueExists("OrderDelay") and self:GetNumberValue("OrderDelay") or 5000;
	
	if self:NumberValueExists("orderDelivering") and self:GetNumberValue("orderDelivering") == 1 then
		self.orderDelivering = true;
	end

	if not self.orderDelivering and not self:IsInventoryEmpty() then
		-- we must have been interrupted mid-actual delivery
		self.SpriteAnimMode = MOSprite.ALWAYSPINGPONG;
	end

	self.currentTeam = -1;
	if self:NumberValueExists("currentTeam") then
		self.currentTeam = self:GetNumberValue("currentTeam");
		self:RemoveNumberValue("currentTeam");
	end

	self.spawnTimer = Timer();
	self.spawnDelay = 500;
	
	self.Message = "";
	self.messageTimer = Timer();
	self.messageTime = 0;
	
	self.closeActorTable = {};
	self.actorUpdateTimer = Timer();
	self.actorUpdateDelay = 50;
	
	self.detectRange = 200;
	
	self.orderPieSlice = CreatePieSlice("Buy Door Order", "Base.rte");
end

function ThreadedUpdate(self)
	if self.Frame == self.FrameCount - 1 and not self.isStayingOpen and not self.isClosing then
		self.isStayingOpen = true;
		self.stayOpenTimer:Reset();
		self.SpriteAnimMode = MOSprite.NOANIM;
	elseif self.isStayingOpen and not self:IsInventoryEmpty() then
		-- Spawn the next object
		if self.spawnTimer:IsPastSimMS(self.spawnDelay) then
			self:RequestSyncedUpdate();
		end
	elseif self.isStayingOpen and not self.isClosing and self.stayOpenTimer:IsPastSimTimeLimit() then
		self.SpriteAnimMode = MOSprite.ALWAYSPINGPONG;
		self.isStayingOpen = false;
		self.isClosing = true;
		self.openCloseSound:Play();
	elseif self.Frame == 1 and self.isClosing then
		self.isClosing = false;
		self.SpriteAnimMode = MOSprite.NOANIM;
	end
	
	if self.cooldownTimer:IsPastSimMS(self.cooldownTime) then
		self.Unusable = false;
		if self.orderDelivering then
			self.Unusable = true;
			if self.orderTimer:IsPastSimMS(self.orderDelay) then
				self.SpriteAnimMode = MOSprite.ALWAYSPINGPONG;
				self.isClosing = false;
				self.orderDelivering = false;
				self.openCloseSound:Play();
				
				self.cooldownTimer:Reset();
			else
				PrimitiveMan:DrawTextPrimitive(self.console.Pos, tostring(math.ceil(self.orderDelay/1000 - self.orderTimer.ElapsedSimTimeS)), true, 1);
			end
		end
	elseif self:IsInventoryEmpty() then
		self.Unusable = true;
		PrimitiveMan:DrawTextPrimitive(self.console.Pos + Vector(0, -10), "Reorganizing...", true, 1);
		PrimitiveMan:DrawTextPrimitive(self.console.Pos, tostring(math.ceil(self.cooldownTime/1000 - self.cooldownTimer.ElapsedSimTimeS)), true, 1);
		self.orderDelivering = false;
	end
	
	if self.actorUpdateTimer:IsPastSimMS(self.actorUpdateDelay) then
		for actor in MovableMan:GetMOsInRadius(self.Pos, self.detectRange, -1, false) do
			if (not self.closeActorTable[actor.UniqueID]) and IsAHuman(actor) or IsACrab(actor) then
				actor = ToActor(actor);
				-- nearby enemies disable use
				if actor.Team ~= self.Team then
					self.Unusable = true;
				end

				self.closeActorTable[actor.UniqueID] = actor.UniqueID;
			end
		end
		
		self:RequestSyncedUpdate();
	end
	
	if not self.messageTimer:IsPastSimMS(self.messageTime) then
		PrimitiveMan:DrawTextPrimitive(self.console.Pos, self.Message, true, 1);
	end
	
	--PrimitiveMan:DrawTextPrimitive(self.console.Pos + Vector(0, 20), "Team: " .. self.Team, true, 1);
	
	if self.Unusable then
		PrimitiveMan:DrawTextPrimitive(self.console.Pos + Vector(0, -20), "UNUSABLE", true, 1);
		self:SetNumberValue("BuyDoor_Unusable", 1);
	else
		self:RemoveNumberValue("BuyDoor_Unusable");
	end
end

function SyncedUpdate(self)
	if self.isStayingOpen and not self:IsInventoryEmpty() and self.spawnTimer:IsPastSimMS(self.spawnDelay) then
		self.spawnTimer:Reset();

		local item = self:RemoveInventoryItemAtIndex(0);
		item.Pos = self.Pos;
		item.Team = self.currentTeam;
		MovableMan:AddMO(item);

		-- Wait until we spawn the next guy
		if self:IsInventoryEmpty() then
			self.cooldownTimer:Reset();
		end
	end

	if self.actorUpdateTimer:IsPastSimMS(self.actorUpdateDelay) then
		-- TODO pawnis fix me!
		self.actorUpdateTimer:Reset();
		
		for k, v in pairs(self.closeActorTable) do
			local actor = MovableMan:FindObjectByUniqueID(v);
			if actor and MovableMan:ValidMO(actor) then
				actor = ToActor(actor);
				local dist = SceneMan:ShortestDistance(self.Pos, actor.Pos, true);
				if dist:MagnitudeIsGreaterThan(self.detectRange) then
					actor.PieMenu:RemovePieSlicesByPresetName(self.orderPieSlice.PresetName);
					actor:RemoveNumberValue("BuyDoor_Order");
					self.closeActorTable[k] = nil;
				else
					actor.PieMenu:AddPieSliceIfPresetNameIsUnique(self.orderPieSlice, self);
					if actor:NumberValueExists("BuyDoor_Order") then
						actor:RemoveNumberValue("BuyDoor_Order");
						if self:IsInventoryEmpty() and not self.Unusable then
							-- Set up order here
							
							local team = actor.Team;
							local player = actor:GetController().Player;		
							local buyGUI = self.Activity:GetBuyGUI(player);
							local orderCost = buyGUI:GetTotalCartCost();
							
							local funds = self.Activity:GetTeamFunds(team);
							
							if funds < orderCost then
								self.Message = "Insufficient funds!"
								self.messageTime = 4000;
								self.messageTimer:Reset();
								return;
							else
								self.Activity:SetTeamFunds(funds - orderCost, team);
							end
							
							local orderList = buyGUI:GetOrderList();						
							local finalOrder = BuyDoorSetupOrder(self, orderList, false, team);
							
							if finalOrder then
								self.orderTimer:Reset();
								self.orderDelivering = true;
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
	self.saveLoadHandler:SaveMOLocally(self, "savedConsoleMO", self.console);
	self:SetNumberValue("isStayingOpen", self.isStayingOpen and 1 or 0);
	self:SetNumberValue("cooldownTimer", self.cooldownTimer.ElapsedRealTimeMS);
	self:SetNumberValue("orderTimer", self.orderTimer.ElapsedRealTimeMS);
	self:SetNumberValue("orderDelivering", self.orderDelivering and 1 or 0);
	self:SetNumberValue("currentTeam", self.currentTeam);
end