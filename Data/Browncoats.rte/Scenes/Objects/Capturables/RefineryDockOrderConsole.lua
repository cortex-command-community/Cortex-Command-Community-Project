function OnMessage(self, message, object)
	if message == "Refinery_DockConsoleOrderSuccess" then
		self.lastOrderSuccess = object;
	end
end

function DockConsoleSetupOrder(self, orderList)
	local preActorItemList = {};
	local lastActor
	local finalOrder = {};
	
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
			print("Dock Console was given an order item with a class it couldn't handle: " .. item);
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
	else -- Finally, post-process the new items into their UniqueIDs
		for k, item in ipairs(finalOrder) do
			finalOrder[k] = item.UniqueID;
		end
	end
	
	return finalOrder;
end

function Create(self)
	self.Activity = ToGameActivity(ActivityMan:GetActivity());
	
	self.Message = "";
	self.messageTimer = Timer();
	self.messageTime = 0;
	
	self.closeActorTable = {};
	self.actorUpdateTimer = Timer();
	self.actorUpdateDelay = 50;
	
	self.detectRange = 200;
	
	-- This is horrible and I hate it
	-- Represents Stage (the number after S in the dock capturable name)
	if self:GetNumberValue("DockIdentifier") == 1 then
		self.assignedDocks = {1, 2};
	elseif self:GetNumberValue("DockIdentifier") == 3 then
		self.assignedDocks = {2, 3};
	elseif self:GetNumberValue("DockIdentifier") == 4 then
		self.assignedDocks = {4, 5};
	end
	
	self.orderPieSlice = CreatePieSlice("Refinery Dock Console Order", "Base.rte");
end

function ThreadedUpdate(self)
	if self.actorUpdateTimer:IsPastSimMS(self.actorUpdateDelay) then
		for actor in MovableMan:GetMOsInRadius(self.Pos, self.detectRange, -1, false) do
			if (not self.closeActorTable[actor.UniqueID]) and IsAHuman(actor) or IsACrab(actor) then
				actor = ToActor(actor);
				if actor.Team == self.Team then
					self.closeActorTable[actor.UniqueID] = actor.UniqueID;
				end
			end
		end
		
		self:RequestSyncedUpdate();
	end
	
	if not self.messageTimer:IsPastSimMS(self.messageTime) then
		PrimitiveMan:DrawTextPrimitive(self.Pos + Vector(0, -50), self.Message, true, 1);
	end
end

function SyncedUpdate(self)
	if self.actorUpdateTimer:IsPastSimMS(self.actorUpdateDelay) then
		self.actorUpdateTimer:Reset();
		
		for k, v in pairs(self.closeActorTable) do
			local actor = MovableMan:FindObjectByUniqueID(v);
			if actor and MovableMan:ValidMO(actor) then
				actor = ToActor(actor);
				local dist = SceneMan:ShortestDistance(self.Pos, actor.Pos, true);
				if dist:MagnitudeIsGreaterThan(self.detectRange) then
					actor.PieMenu:RemovePieSlicesByPresetName(self.orderPieSlice.PresetName);
					actor:RemoveNumberValue("DockConsole_Order");
					self.closeActorTable[k] = nil;
				else
					actor.PieMenu:AddPieSliceIfPresetNameIsUnique(self.orderPieSlice, self);
					if actor:NumberValueExists("DockConsole_Order") then
						actor:RemoveNumberValue("DockConsole_Order");
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
						end
						
						local orderList = buyGUI:GetOrderList();						
						local finalOrder = DockConsoleSetupOrder(self, orderList, team);
						
						-- Finally, we make our order table see ask if the Activity can deliver this to one of our chosen docks
						if finalOrder then
							local dockNum = self.assignedDocks[1];
							
							local activityOrder = {self.UniqueID, dockNum, finalOrder};
							self.Activity:SendMessage("Refinery_DockConsoleOrder", activityOrder);
							-- By now we've gotten a response message
							if not self.lastOrderSuccess then
								-- Try again with our second assigned dock
								activityOrder[2] = self.assignedDocks[2];
								self.Activity:SendMessage("Refinery_DockConsoleOrder", activityOrder);
								if not self.lastOrderSuccess then
									self.Message = "No docks available!"
									self.messageTime = 4000;
									self.messageTimer:Reset();
									return;
								end
							end
							-- If we're here we succeeded so drain funds
							self.Activity:SetTeamFunds(funds - orderCost, team);
							
							self.Message = "Order arriving soon..."
							self.messageTime = 4000;
							self.messageTimer:Reset();
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

end