local MOUtility = {}

function MOUtility:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;
	
	self.saveTable = {};
	
	self.saveTable.originalGHBMOs = {};
	self.saveTable.originalHMOs = {};
	
	self.saveTable.frozenHPs = {};

	return Members;
end

function MOUtility:OnLoad(saveLoadHandler)
	
	print("INFO: MOUtility state loading...");
	self.saveTable = saveLoadHandler:ReadSavedStringAsTable("MOUtilitySaveTable");
	
	self.saveTable.movableObjects = {};
	self.saveTable.movableObjects = {};
	
	-- match old uniqueid indexes to new uniqueid values, copying over old uniqueid's original value to a new entry using the new uniqueid
	-- then delete old uniqueid index
	-- try not to get confused...
	for uniqueID, _ in pairs(self.saveTable.originalGHBMOs) do
		self.saveTable.originalGHBMOs[self.saveTable.movableObjects[uniqueID].UniqueID] = self.saveTable.originalGHBMOs[uniqueID];
		self.saveTable.originalGHBMOs[uniqueID] = nil;
	end
	for uniqueID, _ in pairs(self.saveTable.originalHMOs) do
		self.saveTable.originalHMOs[self.saveTable.movableObjects[uniqueID].UniqueID] = self.saveTable.originalHMOs[uniqueID];
		self.saveTable.originalHMOs[uniqueID] = nil;
	end
	for uniqueID, _ in pairs(self.saveTable.frozenHPs) do
		local actor = MovableMan:FindObjectByUniqueID(self.saveTable.movableObjects[uniqueID].UniqueID);
		if actor then
			self.saveTable.frozenHPs[actor.UniqueID] = actor.Health;
		end
		self.saveTable.frozenHPs[uniqueID] = nil;
	end
	self.saveTable.movableObjects = {};
	self.saveTable.movableObjects = {};
	print("INFO: MOUtility state loaded!");
	
end

function MOUtility:OnSave(saveLoadHandler)
	
	print("INFO: MOUtility state saving...");
	
	self.saveTable.movableObjects = {};
	self.saveTable.movableObjects = {};
	
	-- turn uniqueids into mos
	for uniqueID, _ in pairs(self.saveTable.originalGHBMOs) do
		local mo = MovableMan:FindObjectByUniqueID(uniqueID);
		if mo then
			self.saveTable.movableObjects[uniqueID] = mo;
		end
	end
	for uniqueID, _ in pairs(self.saveTable.originalHMOs) do
		local mo = MovableMan:FindObjectByUniqueID(uniqueID);
		if mo then
			self.saveTable.movableObjects[uniqueID] = mo;
		end
	end
	for uniqueID, _ in pairs(self.saveTable.frozenHPs) do
		local mo = MovableMan:FindObjectByUniqueID(uniqueID);
		if mo then
			self.saveTable.movableObjects[uniqueID] = mo;
		end
	end
	
	saveLoadHandler:SaveTableAsString("MOUtilitySaveTable", self.saveTable);
	
	print("INFO: MOUtility state saved!");
	
end

function MOUtility:RecurseUnhittable(mo, unhittable, reset)
	local valueToSet;
	if unhittable == true then
		valueToSet = false;
	end
	if reset then
		mo.GetsHitByMOs = self.saveTable.originalGHBMOs[mo.UniqueID];
		self.saveTable.originalGHBMOs[mo.UniqueID] = nil;
		mo.HitsMOs = self.saveTable.originalHMOs[mo.UniqueID];
		self.saveTable.originalHMOs[mo.UniqueID] = nil;
		for att in mo.Attachables do
			self:RecurseUnhittable(att, unhittable, reset)
		end
	else
		self.saveTable.originalGHBMOs[mo.UniqueID] = mo.GetsHitByMOs;
		mo.GetsHitByMOs = valueToSet;
		self.saveTable.originalHMOs[mo.UniqueID] = mo.HitsMOs;
		mo.HitsMOs = valueToSet;
		for att in mo.Attachables do
			self:RecurseUnhittable(att, unhittable, reset)
		end
	end
end

function MOUtility:SetMOUnhittable(mo, unhittable)

	local toggled = false;
	if unhittable == nil then
		-- treat as toggle
		toggled = true;
		if self.saveTable.originalGHBMOs[mo.UniqueID] then
			unhittable = false;
		else
			unhittable = true;
		end
	end
	
	if mo then
		if self.saveTable.originalGHBMOs[mo.UniqueID] and unhittable == false then
			-- reset to original values
			self:RecurseUnhittable(mo, unhittable, true);
		elseif unhittable == true then
			self:RecurseUnhittable(mo, unhittable, false);
		end
	else
		print("ERROR: MOUtility tried to set an MO to be unhittable but was not given an MO!");
		return false;
	end
	
	if toggled then
		return unhittable;
	else
		return true;
	end

end

function MOUtility:SetFreezeActorHP(actor, toggle)

	local toggled = false;
	if unhittable == nil then
		-- treat as toggle
		toggled = true;
		local found;
		for uniqueID, _ in pairs(self.saveTable.frozenHPs) do
			if uniqueID == actor.UniqueID then
				found = true;
				toggle = false;
				break;
			end
		end
			
		if not found then
			toggle = true;
		end
	end

	if actor then
		if toggle == true then
			self.saveTable.frozenHPs[actor.UniqueID] = actor.Health;
		elseif toggle == false and self.saveTable.frozenHPs[actor.UniqueID] then
			self.saveTable.frozenHPs[actor.UniqueID] = nil;
		end
	else
		print("ERROR: MOUtility tried to freeze an actor HP but was not given an actor!");
		return false;
	end


	if toggled then
		return toggle;
	else
		return true;
	end

end

function MOUtility:Update()

	for uniqueID, health in pairs(self.saveTable.frozenHPs) do
		local actor = MovableMan:FindObjectByUniqueID(uniqueID);
		if actor then
			actor = ToActor(actor)
			actor.Health = health;
			if actor.Status > 2 then
				actor.Status = 1;
			end
		else
			table.remove(self.saveTable.frozenHPs, uniqueID);
		end
	end

end

return MOUtility:Create();