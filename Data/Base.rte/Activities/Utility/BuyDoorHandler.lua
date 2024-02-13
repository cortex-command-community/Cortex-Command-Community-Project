--------------------------------------- Instructions ---------------------------------------

------- Require this in your script like so: 

-- self.buyDoorHandler = require("Activities/Utility/BuyDoorHandler");
-- self.buyDoorHandler:Initialize(Activity, bool newGame, bool verboseLogging);

-- This is a simple utility for activities to send orders to buy doors.
-- Create your items/actors beforehand and put them in a table.

-- SendCustomOrder(table order, int team) will pick a random buy door on the map of the same team
-- and send that order to it if any are available.

-- You can use GetAvailableBuyDoorsInArea(Area area, int team) to get a table of indexes that point to non-busy buy doors in that area,
-- then use SendCustomOrder(table order, int team, int index) to send to one of them (pick them randomly yourself beforehand).
-- Note that it will only give you non-busy indexes and as such may not give you all of the buy doors in that area.

-- If you manually set up your table of buy doors and want the indices to be the same in BuyDoorHandler,
-- use ReplaceBuyDoorTable(newTable). newTable should be an integer-indexed table of references to real buy doors.

------- Saving/Loading

-- Saving and loading requires you to also have the SaveLoadHandler ready.
-- Simply run OnSave(instancedSaveLoadHandler) and OnLoad(instancedSaveLoadHandler) when appropriate.

--------------------------------------- Misc. Information ---------------------------------------

--




local BuyDoorHandler = {};

function BuyDoorHandler:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;

	return Members;
end

function BuyDoorHandler:Initialize(activity, newGame, verboseLogging)

	if verboseLogging then
		self.verboseLogging = true;
	end
	
	self.Activity = activity;
	
	if newGame then
	
		-- find and save buy doors
		
		self.buyDoorTable = {};
		
		for mo in MovableMan.AddedActors do
			if mo.PresetName == "Reinforcement Door" then
				table.insert(self.buyDoorTable, ToMOSRotating(mo));
			end
		end
		
	end
	
	print("INFO: BuyDoorHandler initialized!")
	
end

function BuyDoorHandler:OnLoad(saveLoadHandler)
	
	print("INFO: BuyDoorHandler loading...");
	self.buyDoorTable = saveLoadHandler:ReadSavedStringAsTable("buyDoorHandlerBuyDoorTable");
	print("INFO: BuyDoorHandler loaded!");
	
end

function BuyDoorHandler:OnSave(saveLoadHandler)
	
	print("INFO: BuyDoorHandler saving...");
	saveLoadHandler:SaveTableAsString("buyDoorHandlerBuyDoorTable", self.buyDoorTable);
	print("INFO: BuyDoorHandler saved!");
	
end

function BuyDoorHandler:ReplaceBuyDoorTable(newTable)

	if newTable then
		self.buyDoorTable = newTable;
		return true;
	end
	
	return false;

end

function BuyDoorHandler:IsBuyDoorBusy(specificIndex)

	return self.buyDoorTable[specificIndex]:NumberValueExists("BuyDoor_Unusable");
	
end

function BuyDoorHandler:GetAvailableBuyDoorsInArea(area, team)

	local usableIndexesTable = {};
	for i = 1, #self.buyDoorTable do
		if not self.buyDoorTable[i]:NumberValueExists("BuyDoor_Unusable") and (team and self.buyDoorTable[i].Team == team) or (not team) then
			if area:IsInside(self.buyDoorTable[i].Pos) then
				table.insert(usableIndexesTable, i);
			end
		end
	end
	
	if #usableIndexesTable > 0 then
		return usableIndexesTable;
	else
		return false;
	end
	
end

function BuyDoorHandler:ChangeCooldownTime(index, newTime)

	if index and newTime then
		if self.buyDoorTable[index] then
			self.buyDoorTable[index]:SendMessage("BuyDoor_ChangeCooldownTime", newTime);
		else
			print("ERROR: BuyDoorHandler was asked to change the cooldown time of an index that didn't exist!");
			return false;
		end
	else
		print("ERROR: BuyDoorHandler was asked to change a cooldown time, but was not given an index or a new time!");
		return false;
	end
	
	return true;

end
function BuyDoorHandler:SendCustomOrder(order, team, specificIndex)
	
	if specificIndex then
		if (not self.buyDoorTable[specificIndex]:NumberValueExists("BuyDoor_Unusable")) and self.buyDoorTable[specificIndex]:IsInventoryEmpty() then
			for k, item in pairs(order) do
				self.buyDoorTable[specificIndex]:AddInventoryItem(item);
			end
			self.buyDoorTable[specificIndex]:SendMessage("BuyDoor_CustomTableOrder");
		else
			--print("Buy Door Handler was asked to send a custom order to a busy specific index!");
			return false;
		end
	else
		-- we trust the buy door to tell us if it can be used or not
		-- it's either busy or has enemies nearby if it can't
		local usableIndexesTable = {};
		for i = 1, #self.buyDoorTable do
			if self.buyDoorTable[specificIndex]:IsInventoryEmpty() and not self.buyDoorTable[i]:NumberValueExists("BuyDoor_Unusable") and (team and self.buyDoorTable[i].Team == team) or (not team) then
				table.insert(usableIndexesTable, i);
			end
		end
		if #usableIndexesTable > 0 then
			self.buyDoorTable[usableIndexesTable[math.random(1, #usableIndexesTable)]]:SendMessage("BuyDoor_CustomTableOrder", order);
		else
			--print("Buy Door Handler could not find any non-busy Buy Doors to send a custom order to!");
			return false;
		end
	end

	return true;

end


return BuyDoorHandler:Create();