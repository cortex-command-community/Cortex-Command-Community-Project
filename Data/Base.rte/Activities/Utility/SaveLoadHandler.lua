local SaveLoadHandler = {}

-- From Bebomonky's Last Cortex mod, from MyNameIsTrez

function SaveLoadHandler:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;

	return Members;
end

function SaveLoadHandler:Initialize()
	
	--print("SaveLoadHandlerinited")
	
	-- congrats, we did nothing
	
end

function SaveLoadHandler:SerializeTable(val, name, skipnewlines, depth)
	skipnewlines = skipnewlines or false
	depth = depth or 0
	
	--print("looking at val: ");
	--print(val)
	print("with name: ")
	print(name)

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
	elseif val.Magnitude then -- ghetto vector check
		tmp = tmp .. string.format("%q", "Vector(" .. val.X .. "," .. val.Y .. ")")
	elseif val.PresetName and IsMOSRotating(val) then -- IsMOSRotating freaks out if we give it something that isn't a preset at all... ghetto here too
		val:SetNumberValue("saveLoadHandlerUniqueID", val.UniqueID);
		tmp = tmp .. string.format("%q", "SAVELOADHANDLERUNIQUEID_" .. tostring(val.UniqueID))
	elseif val.FirstBox then -- ghetto area check
		tmp = tmp .. string.format("%q", "SAVELOADHANDLERAREA_" .. tostring(val.Name))
	else
		tmp = tmp .. '"[inserializeable datatype:' .. type(val) .. ']"'
	end
	
	print("saved this table just now:")
	print(tmp)

	return tmp
end

function SaveLoadHandler:ParseTableForMOs(tab)

	for k, v in pairs(tab) do
		local mo;
		local didNotFindAnMO = true;
		if type(v) == "string" and string.find(v, "SAVELOADHANDLERUNIQUEID_") then
			local id = math.abs(tonumber(string.sub(v, 25, -1)));
			print("ID: " .. id);
			for particle in MovableMan.AddedParticles do
				if math.abs(particle:GetNumberValue("saveLoadHandlerUniqueID")) == id then
					--particle:removeNumberValue("saveLoadHandlerUniqueID");
					mo = particle;
					didNotFindAnMO = false;
					break;
				end
			end
			for act in MovableMan.AddedActors do
				if math.abs(act:GetNumberValue("saveLoadHandlerUniqueID")) == id then
					--act:removeNumberValue("saveLoadHandlerUniqueID");
					mo = act;
					didNotFindAnMO = false;
					break;
				end
				for item in act.Inventory do
					if math.abs(item:GetNumberValue("saveLoadHandlerUniqueID")) == id then
						--item:removeNumberValue("saveLoadHandlerUniqueID");
						mo = item;
						didNotFindAnMO = false;
						break;
					end
				end
			end
			for item in MovableMan.AddedItems do
				if math.abs(item:GetNumberValue("saveLoadHandlerUniqueID")) == id then
					--item:removeNumberValue("saveLoadHandlerUniqueID");
					mo = item;
					didNotFindAnMO = false;
					break;
				end
			end
			for particle in MovableMan.Particles do
				if math.abs(particle:GetNumberValue("saveLoadHandlerUniqueID")) == id then
					--particle:removeNumberValue("saveLoadHandlerUniqueID");
					mo = particle;
					didNotFindAnMO = false;
					break;
				end
			end
			for act in MovableMan.Actors do
				if math.abs(act:GetNumberValue("saveLoadHandlerUniqueID")) == id then
					--act:removeNumberValue("saveLoadHandlerUniqueID");
					mo = act;
					didNotFindAnMO = false;
					break;
				end
				for item in act.Inventory do
					if math.abs(item:GetNumberValue("saveLoadHandlerUniqueID")) == id then
						--item:removeNumberValue("saveLoadHandlerUniqueID");
						mo = item;
						didNotFindAnMO = false;
						break;
					end
				end
			end
			for item in MovableMan.Items do
				if math.abs(item:GetNumberValue("saveLoadHandlerUniqueID")) == id then
					--item:removeNumberValue("saveLoadHandlerUniqueID");
					mo = item;
					didNotFindAnMO = false;
					break;
				end
			end
			if didNotFindAnMO then
				print("WARNING: SaveLoadHandler could not resolve a saved MO UniqueID! A loaded table is likely broken.");
				print("Not found: " .. v)
			end
		elseif type(v) == "table" then
			self:ParseTableForMOs(v);
		end
		if mo then
			tab[k] = mo;
			print("Set found MO!");
			print(v);
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

function SaveLoadHandler:ReadSavedStringAsTable(name)
	print(ActivityMan:GetActivity())
	print(ActivityMan:GetActivity():LoadString(name))
	local savedString = ActivityMan:GetActivity():LoadString(name)
	if savedString == "" then
		savedString = "{}";
	end

	local tab = loadstring("return " .. savedString)()
	-- Parse for saved MOSRotatings
	-- Very mildly inefficient in terms of looping even after resolving a value, but it happens once on startup
	print("Parsing table for MOs: " .. name);
	self:ParseTableForMOs(tab);
	self:ParseTableForVectors(tab);
	self:ParseTableForAreas(tab);
	
	return tab;

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
	print("Finding locally saved MO: " .. name);
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
		print("WARNING: SaveLoadHandler could not resolve a locally saved MO!");
		return false;
	end
	
	self:RemoveStringValue(name);
	
	return v;

end

return SaveLoadHandler:Create();