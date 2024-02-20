--------------------------------------- Instructions ---------------------------------------

------- Require this in your script like so: 

-- self.saveLoadHandler = require("Activities/Utility/SaveLoadHandler");
-- self.saveLoadHandler:Initialize(bool verboseLogging);

-- The main feature is table saving, which will save almost any table as a string.
-- Aside from simple things like numbers, bools, and strings, you can also save:

-- Vector()
-- MOSRotatings and derivatives
-- Area(), by name (it won't reconstruct it on a new Scene)
-- Timer(), including both elapsed sim time and elapsed real time

-- Simply do SaveLoadHandler:SaveTableAsString(string name, table), then yourTableVariableHere = SaveLoadHandler:ReadSavedStringAsTable(string name).


-- There is also SaveLoadHandler:SaveMOLocally(self, string name, MOSRotating mo) and SaveLoadHandler:LoadLocallySavedMO(self, string name)
-- so you can make use of SaveLoadHandler's unique parsing method to save and load a specific MO from any entity script.

-- All of the above functions are intended to be using during official Activity game saving and loading, and will result in the info being saved
-- in the resulting .ini file.

-- If you just want a table serialized and deserialized you can directly use SaveLoadHandler:SerializeTable(table) and SaveLoadHandler:DeserializeTable(string serializedTable).


--------------------------------------- Misc. Information ---------------------------------------

-- Please note that if you're using UniqueIDs you MUST convert these to real MOs before attempting to save. As far as lua is concerned,
-- a UniqueID is just a number, so SaveLoadHandler will save it as such instead of using its special MO method. UniqueIDs reset on game load too,
-- so they are no longer the same anyway.

-- Also, SaveLoadHandler's MO parsing method relies on trawling through entities owned by MovableMan, which means entities that actually exist in the simulation,
-- including those in the inventory of Actors that exist in the simulation.
-- Yes, this means that valid entities that only exist in the lua state and not in the simulation can't really be saved and loaded. Those are wiped
-- as soon a the lua state is deleted (i.e. you save and quit a game).

local SaveLoadHandler = {}

function SaveLoadHandler:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;

	return Members;
end

function SaveLoadHandler:Initialize(verboseLogging)

	if verboseLogging then
		SaveLoadHandler.verboseLogging = true;
		-- dunno why self doesn't work here...
	end

end

function SaveLoadHandler:SerializeTable(val, name, skipnewlines, depth)
	skipnewlines = skipnewlines or false
	depth = depth or 0

	local tmp = string.rep(" ", depth)

	if name then
		if type(name) == "number" then
			tmp = tmp .. "[" .. name .. "]" .. " = "
		else
			tmp = tmp .. name .. " = "
		end
	end

	if type(val) == "table" then
		tmp = tmp .. "{" .. (not skipnewlines and "\n" or "")

		for k, v in pairs(val) do
			tmp = tmp
				.. SaveLoadHandler:SerializeTable(v, k, skipnewlines, depth + 1)
				.. ","
				.. (not skipnewlines and "\n" or "")
		end

		tmp = tmp .. string.rep(" ", depth) .. "}"
	elseif type(val) == "number" then
		tmp = tmp .. tostring(val)
	elseif type(val) == "string" then
		tmp = tmp .. string.format("%q", val)
	elseif type(val) == "boolean" then
		tmp = tmp .. (val and "true" or "false")
	elseif type(val) == "function" then
		tmp = tmp .. '"[inserializable function]"'
		print("ERROR: SaveLoadHandler tried to save a function. This cannot currently be done and the table will not be as expected when loaded.");
	elseif val.Magnitude then -- ghetto vector check
		tmp = tmp .. string.format("%q", "Vector(" .. val.X .. "," .. val.Y .. ")")
	elseif val.PresetName and IsMOSRotating(val) then -- IsMOSRotating freaks out if we give it something that isn't a preset at all... ghetto here too
		val:SetNumberValue("saveLoadHandlerUniqueID", val.UniqueID);
		tmp = tmp .. string.format("%q", "SAVELOADHANDLERUNIQUEID_" .. tostring(val.UniqueID))
	elseif val.FirstBox then -- ghetto area check
		tmp = tmp .. string.format("%q", "SAVELOADHANDLERAREA_" .. tostring(val.Name))
	elseif val.ElapsedSimTimeMS then -- ghetto timer check
		tmp = tmp .. string.format("%q", "SAVELOADHANDLERTIMER_" .. tostring(val.ElapsedSimTimeMS) .. "_" .. tostring(val.ElapsedRealTimeMS))
	else
		tmp = tmp .. '"[inserializable datatype:' .. type(val) .. ']"'
		print("ERROR: SaveLoadHandler tried to save an inserializable datatype. The table will not be as expected when loaded.");
	end
	
	if SaveLoadHandler.verboseLogging and depth == 0 then
		print("INFO: SaveLoadHandler serialized this table:")
		print(tmp)
	end

	return tmp
end

function SaveLoadHandler:FindMOWithId(id)
	for act in MovableMan.AddedActors do
		if math.abs(act:GetNumberValue("saveLoadHandlerUniqueID")) == id then
			--act:removeNumberValue("saveLoadHandlerUniqueID");
			return act;
		end
		for item in act.Inventory do
			if math.abs(item:GetNumberValue("saveLoadHandlerUniqueID")) == id then
				--item:removeNumberValue("saveLoadHandlerUniqueID");
				return item;
			end
		end
		for att in ToMOSRotating(act).Attachables do
			if math.abs(att:GetNumberValue("saveLoadHandlerUniqueID")) == id then
				--item:removeNumberValue("saveLoadHandlerUniqueID");
				return att;
			end
		end
	end
	for act in MovableMan.Actors do
		if math.abs(act:GetNumberValue("saveLoadHandlerUniqueID")) == id then
			--act:removeNumberValue("saveLoadHandlerUniqueID");
			return act;
		end
		for item in act.Inventory do
			if math.abs(item:GetNumberValue("saveLoadHandlerUniqueID")) == id then
				--item:removeNumberValue("saveLoadHandlerUniqueID");
				return item;
			end
		end
		for att in ToMOSRotating(act).Attachables do
			if math.abs(att:GetNumberValue("saveLoadHandlerUniqueID")) == id then
				--item:removeNumberValue("saveLoadHandlerUniqueID");
				return att;
			end
		end
	end

	for item in MovableMan.AddedItems do
		if math.abs(item:GetNumberValue("saveLoadHandlerUniqueID")) == id then
			--item:removeNumberValue("saveLoadHandlerUniqueID");
			return item;
		end
		for att in ToMOSRotating(item).Attachables do
			if math.abs(att:GetNumberValue("saveLoadHandlerUniqueID")) == id then
				--item:removeNumberValue("saveLoadHandlerUniqueID");
				return att;
			end
		end
	end
	for item in MovableMan.Items do
		if math.abs(item:GetNumberValue("saveLoadHandlerUniqueID")) == id then
			--item:removeNumberValue("saveLoadHandlerUniqueID");
			return item;
		end
		for att in ToMOSRotating(item).Attachables do
			if math.abs(att:GetNumberValue("saveLoadHandlerUniqueID")) == id then
				--item:removeNumberValue("saveLoadHandlerUniqueID");
				return att;
			end
		end
	end

	for particle in MovableMan.AddedParticles do
		if math.abs(particle:GetNumberValue("saveLoadHandlerUniqueID")) == id then
			--particle:removeNumberValue("saveLoadHandlerUniqueID");
			return particle;
		end
		if IsMOSRotating(particle) then
			for att in ToMOSRotating(particle).Attachables do
				if math.abs(att:GetNumberValue("saveLoadHandlerUniqueID")) == id then
					--item:removeNumberValue("saveLoadHandlerUniqueID");
					return att;
				end
			end
		end
	end
	for particle in MovableMan.Particles do
		if math.abs(particle:GetNumberValue("saveLoadHandlerUniqueID")) == id then
			--particle:removeNumberValue("saveLoadHandlerUniqueID");
			return particle;
		end
		if IsMOSRotating(particle) then
			for att in ToMOSRotating(particle).Attachables do
				if math.abs(att:GetNumberValue("saveLoadHandlerUniqueID")) == id then
					--item:removeNumberValue("saveLoadHandlerUniqueID");
					return att;
				end
			end
		end
	end

	return nil;
end

function SaveLoadHandler:ParseTableForMOs(tab)
	for k, v in pairs(tab) do
		if type(v) == "string" and string.find(v, "SAVELOADHANDLERUNIQUEID_") then
			local id = math.abs(tonumber(string.sub(v, 25, -1)));
			if SaveLoadHandler.verboseLogging then
				print("INFO: SaveLoadHandler is parsing looking for this ID: " .. id)
			end
			local mo = self:FindMOWithId(id);
			if mo then
				tab[k] = mo;
				if SaveLoadHandler.verboseLogging then
					print("INFO: SaveLoadHandler set this found MO:");
					print(v);
				end
			else
				print("ERROR: SaveLoadHandler could not resolve a saved MO UniqueID! A loaded table is likely broken.");
				print("The saved ID was: " .. v)
			end
		elseif type(v) == "table" then
			self:ParseTableForMOs(v);
		end
	end
end

function SaveLoadHandler:ParseTableForVectors(tab)
	for k, v in pairs(tab) do
		if type(v) == "string" and string.find(v, "Vector%(") then
			local vector = loadstring("return " .. v)();
			tab[k] = vector;	
		elseif type(v) == "table" then
			self:ParseTableForVectors(v);
		end
	end
		
end

function SaveLoadHandler:ParseTableForAreas(tab)
	for k, v in pairs(tab) do
		if type(v) == "string" and string.find(v, "SAVELOADHANDLERAREA_") then
			local areaName = string.sub(v, 21, -1);
			local area = SceneMan.Scene:GetOptionalArea(areaName);
			tab[k] = area;	
		elseif type(v) == "table" then
			self:ParseTableForAreas(v);
		end
	end
		
end

function SaveLoadHandler:ParseTableForTimers(tab)
	for k, v in pairs(tab) do
		if type(v) == "string" and string.find(v, "SAVELOADHANDLERTIMER_") then
			local elapsedTimes = string.sub(v, 22, -1);
			local delimiterPosition = string.find(elapsedTimes, "_");
			local elapsedSimTime = string.sub(elapsedTimes, 1, delimiterPosition - 1);
			local elapsedRealTime = string.sub(elapsedTimes, delimiterPosition + 1, -1);
			tab[k] = Timer();
			tab[k].ElapsedSimTimeMS = tonumber(elapsedSimTime);
			tab[k].ElapsedRealTimeMS = tonumber(elapsedRealTime);
			if SaveLoadHandler.verboseLogging then
				print("INFO: SaveLoadHandler loaded a Timer with SimTime: " .. elapsedSimTime .. " and RealTime: " .. elapsedRealTime);
			end
		elseif type(v) == "table" then
			self:ParseTableForTimers(v);
		end
	end
		
end

function SaveLoadHandler:DeserializeTable(serializedTable, name)

	local tab = loadstring("return " .. serializedTable)()
	-- Parse for saved MOSRotatings
	-- Very mildly inefficient in terms of looping even after resolving a value, but it happens once on startup
	if name and SaveLoadHandler.verboseLogging then
		print("INFO: SaveLoadHandler is parsing this table for MOs: " .. name);
	end
	self:ParseTableForMOs(tab);
	if name and SaveLoadHandler.verboseLogging then
		print("INFO: SaveLoadHandler is parsing this table for Vectors: " .. name);
	end
	self:ParseTableForVectors(tab);
	if name and SaveLoadHandler.verboseLogging then
		print("INFO: SaveLoadHandler is parsing this table for Areas: " .. name);
	end
	self:ParseTableForAreas(tab);
	if name and SaveLoadHandler.verboseLogging then
		print("INFO: SaveLoadHandler is parsing this table for Timers: " .. name);
	end
	self:ParseTableForTimers(tab);
	
	return tab;
	
end

function SaveLoadHandler:ReadSavedStringAsTable(name)
	local savedString = ActivityMan:GetActivity():LoadString(name)
	if savedString == "" then
		savedString = "{}";
	end
	
	return SaveLoadHandler:DeserializeTable(savedString, name);

end

function SaveLoadHandler:SaveTableAsString(name, tab)
	local tabStr = SaveLoadHandler:SerializeTable(tab)
	ActivityMan:GetActivity():SaveString(name, tabStr)
end

function SaveLoadHandler:SaveMOLocally(self, name, mo)
	mo:SetNumberValue("saveLoadHandlerUniqueID", mo.UniqueID);
	self:SetStringValue(name, "SAVELOADHANDLERUNIQUEID_" .. tostring(mo.UniqueID));
end

function SaveLoadHandler:LoadLocallySavedMO(self, name)
	local v = self:GetStringValue(name);
	local didNotFindAnMO = false;
	
	local notFound = true;
	if SaveLoadHandler.verboseLogging then
		print("INFO: SaveLoadHandler is finding this locally saved MO: " .. name);
	end
	local id = math.abs(tonumber(string.sub(v, 25, -1)));
	for particle in MovableMan.AddedParticles do
		if math.abs(particle:GetNumberValue("saveLoadHandlerUniqueID")) == id then
			--particle:removeNumberValue("saveLoadHandlerUniqueID");
			v = particle;
			notFound = false;
			break;
		end
	end
	for act in MovableMan.AddedActors do
		if math.abs(act:GetNumberValue("saveLoadHandlerUniqueID")) == id then
			--act:removeNumberValue("saveLoadHandlerUniqueID");
			v = act;
			notFound = false;
			break;
		end
		for item in act.Inventory do
			if math.abs(item:GetNumberValue("saveLoadHandlerUniqueID")) == id then
				--item:removeNumberValue("saveLoadHandlerUniqueID");
				v = item;
				notFound = false;
				break;
			end
		end
	end
	for item in MovableMan.AddedItems do
		if math.abs(item:GetNumberValue("saveLoadHandlerUniqueID")) == id then
			--item:removeNumberValue("saveLoadHandlerUniqueID");
			v = item;
			notFound = false;
			break;
		end
	end
	for particle in MovableMan.Particles do
		if math.abs(particle:GetNumberValue("saveLoadHandlerUniqueID")) == id then
			--particle:removeNumberValue("saveLoadHandlerUniqueID");
			v = particle;
			notFound = false;
			break;
		end
	end
	for act in MovableMan.Actors do
		if math.abs(act:GetNumberValue("saveLoadHandlerUniqueID")) == id then
			--act:removeNumberValue("saveLoadHandlerUniqueID");
			v = act;
			notFound = false;
			break;
		end
		for item in act.Inventory do
			if math.abs(item:GetNumberValue("saveLoadHandlerUniqueID")) == id then
				--item:removeNumberValue("saveLoadHandlerUniqueID");
				v = item;
				notFound = false;
				break;
			end
		end
	end
	for item in MovableMan.Items do
		if math.abs(item:GetNumberValue("saveLoadHandlerUniqueID")) == id then
			--item:removeNumberValue("saveLoadHandlerUniqueID");
			v = item;
			notFound = false;
			break;
		end
	end
	
	if notFound then
		print("ERROR: SaveLoadHandler could not resolve a locally saved MO!");
		return false;
	end
	
	self:RemoveStringValue(name);
	
	return v;

end

return SaveLoadHandler:Create();