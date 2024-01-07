local MOUtility = {}

function MOUtility:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;
	
	self.saveTable = {};
	
	self.saveTable.originalGHBMOs = {};
	self.saveTable.originalHMOs = {};

	return Members;
end

function MOUtility:OnLoad(saveLoadHandler)
	
	print("INFO: MOUtility state loading...");
	self.saveTable = saveLoadHandler:ReadSavedStringAsTable("MOUtilitySaveTable");
	print("INFO: MOUtility state loaded!");
	
end

function MOUtility:OnSave(saveLoadHandler)
	
	print("INFO: MOUtility state saving...");
	saveLoadHandler:SaveTableAsString("MOUtilitySaveTable", self.saveTable);	
	print("INFO: MOUtility state saved!");
	
end

function MOUtility:RecurseUnhittable(mo, unhittable, reset)
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
		mo.GetsHitByMOs = unhittable and false or true;
		self.saveTable.originalHMOs[mo.UniqueID] = mo.HitsMOs;
		mo.HitsMOs = unhittable and false or true;
		for att in mo.Attachables do
			self:RecurseUnhittable(att, unhittable, reset)
		end
	end
end

function MOUtility:SetMOUnhittable(mo, unhittable)
	
	if mo and unhittable ~= nil then
		if self.saveTable.originalGHBMOs[mo.UniqueID] and unhittable == false then
			-- reset to original values
			self:RecurseUnhittable(mo, unhittable, true);
		elseif unhittable == true then
			self:RecurseUnhittable(mo, unhittable, false);
		end
	else
		print("ERROR: MOUtility tried to set an MO to be unhittable but was not given an mo or a new unhittable state!");
		return false;
	end
	
	return true;

end

return MOUtility:Create();