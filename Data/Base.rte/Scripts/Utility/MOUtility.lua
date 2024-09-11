--------------------------------------- Instructions ---------------------------------------

------- Require this in your script like so: 

-- self.MOUtility = require("Scripts/Utility/MOUtility");

-- This is a bunch of MO-related utilities. See each function for instructions.
-- For some functions to work you must call self.MOUtility:Update() every frame.

------- Saving/Loading

-- Saving and loading requires you to also have the SaveLoadHandler ready.
-- Simply run OnSave(instancedSaveLoadHandler) and OnLoad(instancedSaveLoadHandler) when appropriate.

--------------------------------------- Misc. Information ---------------------------------------

--

local MOUtility = {}

function MOUtility:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;
	
	self.saveTable = {};
	
	self.saveTable.MOsToSetUnhittable = {};
	self.saveTable.actorsToSetFrozenHP = {};
	
	self.saveTable.originalGHBMOs = {};
	self.saveTable.originalHMOs = {};
	
	self.saveTable.frozenHPs = {};

	return Members;
end

function MOUtility:OnLoad(saveLoadHandler)
	
	print("INFO: MOUtility state loading...");
	self.saveTable = saveLoadHandler:ReadSavedStringAsTable("MOUtilitySaveTable");
	
	self.saveTable.originalGHBMOs = {};
	self.saveTable.originalHMOs = {};
	self.saveTable.frozenHPs = {};
	
	-- just get back to where we were! simple
	
	for _, actor in pairs(self.saveTable.actorsToSetFrozenHP) do
		MOUtility:SetFreezeActorHP(actor, true);
	end

	print("INFO: MOUtility state loaded!");
	
end

function MOUtility:OnSave(saveLoadHandler)
	
	print("INFO: MOUtility state saving...");
	
	saveLoadHandler:SaveTableAsString("MOUtilitySaveTable", self.saveTable);

	print("INFO: MOUtility state saved!");
	
end

-- Internal function.

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

-- Set MO Unhittable

-- Sets an MO and all its children to be GetsHitByMOs false.
-- Remembers original values, so it's non-destructive.

-- Arguments: MovableObject mo, bool unhittable
-- Unhittable arg is optional: you can call this with just an MO to have it act as a toggle.

function MOUtility:SetMOUnhittable(mo, unhittable)

	local toggled = false;
	if unhittable == nil then
		-- treat as toggle
		toggled = true;
		if self.saveTable.MOsToSetUnhittable[mo.UniqueID] then
			unhittable = false;
		else
			unhittable = true;
		end
	end
	
	if mo then
		if self.saveTable.MOsToSetUnhittable[mo.UniqueID] and unhittable == false then
			self.saveTable.MOsToSetUnhittable[mo.UniqueID] = nil;
			-- reset to original values
			self:RecurseUnhittable(mo, unhittable, true);
		elseif unhittable == true then
			self.saveTable.MOsToSetUnhittable[mo.UniqueID] = mo;
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

-- Set MO Unhittable

-- Freezes an actor's HP to its current value. Requires Update to be called every frame.
-- Prevents death by HP but not gibbing.

-- Arguments: Actor actor, bool toggle
-- toggle arg is optional: you can call this with just an actor to have it act as a toggle.

function MOUtility:SetFreezeActorHP(actor, toggle)

	local toggled = false;
	if toggle == nil then
		-- treat as toggle
		toggled = true;
		if self.saveTable.actorsToSetFrozenHP[actor.UniqueID] then
			toggle = false;
		else
			toggle = true;
		end
	end

	if actor then
		if toggle == true then
			self.saveTable.frozenHPs[actor.UniqueID] = actor.Health;
			self.saveTable.actorsToSetFrozenHP[actor.UniqueID] = actor;
		elseif toggle == false and self.saveTable.actorsToSetFrozenHP[actor.UniqueID] then
			self.saveTable.frozenHPs[actor.UniqueID] = nil;
			self.saveTable.actorsToSetFrozenHP[actor.UniqueID] = nil;
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