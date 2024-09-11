--------------------------------------- Instructions ---------------------------------------

------- Require this in your script like so: 

-- self.deliveryCreationHandler = require("Activities/Utility/DeliveryCreationHandler");
-- self.deliveryCreationHandler:Initialize(Activity);

-- This is a utility for creating actors, squads, and crafts with actors in them.
-- The actors will be equipped according to their type and team tech automatically.
-- To use at a basic level, simply do:

-- actorTable, goldCost = CreateSquad(int desiredTeam, optional int actorCount, optional string actorType)
-- actorTable will be a table with directly usable, Created actors in it. Funds are not automatically deducted.

-- craft, actorTable, goldCost = CreateSquadWithCraft(int desiredTeam, bool useRocket, int actorCount, string actorType)
-- The above function returns a fully Created squad inside the inventory of a fully Created dropship/rocket.

-- Elite versions of those two functions (CreateEliteSquad(WithCraft)) will multiply actor MaxHealth by 1.25 and set their AI to maximum skill.

-- Only int desiredTeam is required. Actor count and type will both be randomized otherwise.
-- If there is no type, the resulting actorTable will have several random types of actor in it, rather than all of only one type.
-- They are selected according to their weights, which are set at reasonable defaults but can be replaced with 
-- ReplaceInfantryTypeWeightsTable(int teamToSetWeightsFor, table newWeightsTable).

-- You can also specify a squad makeup yourself by passing a table of type strings instead of squadCount:
-- squadTable = {1 = "Light", 2 = "Light", 3 = "CQB"} etcetera.

-- Each team's available actors and items are calculated on initialization according to their team tech.
-- You can AddAvailablePreset(int team, string presetName, string className, string techName)
-- or RemoveAvailablePreset(int team, string presetName) to add or remove presets from the list.
-- This persists when save/loading.

-- You can also use the infantry creation functions directly but note that they will not return the gold cost for you.

-- Virtual teams can be added with AddVirtualTeam(teamNumber, techName), where teamNumber cannot be between -1 and 4 (real game teams).
-- Virtual teams are for all intents and purposes full-fledged teams that you can use to create deliveries. You can add and remove presets,
-- set weightings, etcetera. Actors will come out with their team set to the virtual team and should be set to an actual team to avoid undefined behavior.


------- Saving/Loading

-- Saving and loading requires you to also have the SaveLoadHandler ready.
-- Simply run OnSave(instancedSaveLoadHandler) and OnLoad(instancedSaveLoadHandler) when appropriate.


-- Valid infantry types:
--	Light - Light trooper with light weaponry.
--	Medium - Heavy trooper with light weaponry.
--	Heavy - Heavy trooper with heavy weaponry.
--	CQB - CQB weaponry.
--	Scout - Light trooper with only a secondary, with extra support items like medikits.
--	Sniper - Sniper weaponry.
--	Grenadier - Explosive weaponry.
--	Engineer - Digging and breaching.

--------------------------------------- Misc. Information ---------------------------------------

-- Team -1 is always Base.rte. If you want to change this you will have to remove each Base.rte preset and add your new presets manually.
-- If you do, make sure no team involved lacks any crucial group preset (like Weapons - Primary, Weapons - Secondary, an AHuman) etc.
-- or things will get ugly. A good alternative if you want team -1 deliveries of a different tech is to create a virtual team and
-- set team to -1 at the end point instead.




local DeliveryCreationHandler = {};

function DeliveryCreationHandler:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;

	return Members;
end

function DeliveryCreationHandler:Initialize(activity, verboseLogging)
	
	self.verboseLogging = verboseLogging;
	
	self.Activity = activity;
	
	self.infantryTypeFunctionTable = {};
	self.infantryTypeFunctionTable.Light = self.CreateLightInfantry;
	self.infantryTypeFunctionTable.Medium = self.CreateMediumInfantry;
	self.infantryTypeFunctionTable.Heavy = self.CreateHeavyInfantry;
	self.infantryTypeFunctionTable.CQB = self.CreateCQBInfantry;
	self.infantryTypeFunctionTable.Scout = self.CreateScoutInfantry;
	self.infantryTypeFunctionTable.Sniper = self.CreateSniperInfantry;
	self.infantryTypeFunctionTable.Grenadier = self.CreateGrenadierInfantry;
	self.infantryTypeFunctionTable.Engineer = self.CreateEngineerInfantry;
	
	self.indexedInfantryTypeFunctionTable = {};

	for k, v in pairs(self.infantryTypeFunctionTable) do
		table.insert(self.indexedInfantryTypeFunctionTable, v);
	end
	
	self.teamTechTable = {};
	self.teamTechIDTable = {};
	
	-- stuff we wanna save - the rest can be re-initialized each time, it's fine/even desirable
	self.saveTable = {};
	
	self.saveTable.virtualTeams = {};
	
	self.saveTable.teamRemovedPresets = {};
	self.saveTable.teamAddedPresets = {};
	
	self.saveTable.teamInfantryTypeWeights = {};
	
	self.saveTable.teamExtraItemChances = {};
	
	for i = -1, self.Activity.TeamCount do
		if i == -1 then
			local moduleID = PresetMan:GetModuleID("Base.rte");
			self.teamTechTable[i] = PresetMan:GetDataModule(moduleID);
			self.teamTechIDTable[i] = moduleID;
		else
			local moduleID = PresetMan:GetModuleID(self.Activity:GetTeamTech(i));
			if moduleID ~= -1 then
				self.teamTechTable[i] = PresetMan:GetDataModule(moduleID);
			else
				self.teamTechTable[i] = {["FileName"] = "All"}; -- master of ghetto
			end
			self.teamTechIDTable[i] = moduleID;
		end
		
		self.saveTable.teamRemovedPresets[i] = {};
		self.saveTable.teamAddedPresets[i] = {};
		
		self.saveTable.teamInfantryTypeWeights[i] = {};
		
		-- 10 is standard weighting
		
		self.saveTable.teamInfantryTypeWeights[i].Light = 10;
		self.saveTable.teamInfantryTypeWeights[i].Medium = 10;
		self.saveTable.teamInfantryTypeWeights[i].Heavy = 8;
		self.saveTable.teamInfantryTypeWeights[i].CQB = 7;
		self.saveTable.teamInfantryTypeWeights[i].Scout = 0; -- has to be explicitly enabled
		self.saveTable.teamInfantryTypeWeights[i].Sniper = 5;
		self.saveTable.teamInfantryTypeWeights[i].Grenadier = 5;
		self.saveTable.teamInfantryTypeWeights[i].Engineer = 3;
		
		self.saveTable.teamExtraItemChances[i] = {};
		self.saveTable.teamExtraItemChances[i].Medikit = 0.5;
		self.saveTable.teamExtraItemChances[i].BreachingTool = 0.25;
		self.saveTable.teamExtraItemChances[i].Grenade = 0.25;
		self.saveTable.teamExtraItemChances[i].Digger = 0.15;
			
	end
	
	-- Commence painful setting up of preset lists
	
	self.teamPresetTables = {};
	
	-- Base module items too
	-- If the player tries to set the tech of team -1 and this goes wonky as a result
	-- it's his own damn fault

	for module in PresetMan.Modules do
		if module.FileName == "Base.rte" then
			self.teamTechTable[-1] = module;
			self.teamTechIDTable[-1] = PresetMan:GetModuleID(module.FileName);
		end
	end
	
	for team, module in pairs(self.teamTechTable) do
		
		local iterator = module.Presets;
		-- handle -All-
		if not iterator then
			iterator = PresetMan:GetAllEntities();
		end
	
		self.teamPresetTables[team] = {};
		
		self.teamPresetTables[team]["Craft - Dropships"] = {};
		self.teamPresetTables[team]["Craft - Rockets"] = {};
		
		self.teamPresetTables[team]["Weapons - Primary"] = {};
		self.teamPresetTables[team]["Weapons - Secondary"] = {};
		self.teamPresetTables[team]["Weapons - Light"] = {};
		self.teamPresetTables[team]["Weapons - Heavy"] = {};
		self.teamPresetTables[team]["Weapons - Sniper"] = {};
		self.teamPresetTables[team]["Weapons - CQB"] = {};
		self.teamPresetTables[team]["Weapons - Explosive"] = {};
		
		self.teamPresetTables[team]["Shields"] = {};

		self.teamPresetTables[team]["Bombs"] = {};	
		self.teamPresetTables[team]["Bombs - Grenades"] = {};
		
		self.teamPresetTables[team]["Tools"] = {};
		self.teamPresetTables[team]["Tools - Diggers"] = {};
		self.teamPresetTables[team]["Tools - Breaching"] = {};
		
		self.teamPresetTables[team]["Actors - AHuman"] = {};
		self.teamPresetTables[team]["Actors - ACrab"] = {};
		self.teamPresetTables[team]["Actors - Light"] = {};
		self.teamPresetTables[team]["Actors - Heavy"] = {};
		self.teamPresetTables[team]["Actors - Mecha"] = {};
		self.teamPresetTables[team]["Actors - Turrets"] = {};
		
		for entity in iterator do
			if IsMOSRotating(entity) and ToMOSRotating(entity).Buyable and ToMOSRotating(entity).BuyableMode ~= 2 then
			
				local entityInfoTable = {};
			
				for group in entity.Groups do
					if self.teamPresetTables[team][group] then
						entityInfoTable.PresetName = entity.PresetName;
						entityInfoTable.ClassName = entity.ClassName;
						table.insert(self.teamPresetTables[team][group], entityInfoTable);
					end
				end
				
				if IsAHuman(entity) and not entity:IsInGroup("Brains") then
					entityInfoTable.PresetName = entity.PresetName;
					entityInfoTable.ClassName = entity.ClassName;
					table.insert(self.teamPresetTables[team]["Actors - AHuman"], entityInfoTable);
				elseif IsACrab(entity) and not entity:IsInGroup("Brains") then
					entityInfoTable.PresetName = entity.PresetName;
					entityInfoTable.ClassName = entity.ClassName;
					table.insert(self.teamPresetTables[team]["Actors - ACrab"], entityInfoTable);	
				elseif IsACDropShip(entity) then
					entityInfoTable.PresetName = entity.PresetName;
					entityInfoTable.ClassName = entity.ClassName;
					table.insert(self.teamPresetTables[team]["Craft - Dropships"], entityInfoTable);
				elseif IsACRocket(entity) then
					entityInfoTable.PresetName = entity.PresetName;
					entityInfoTable.ClassName = entity.ClassName;
					table.insert(self.teamPresetTables[team]["Craft - Rockets"], entityInfoTable);	
				end
				
			end
		end
	end
	
	print("INFO: DeliveryCreationHandler initialized!")
	
end

function DeliveryCreationHandler:OnLoad(saveLoadHandler)
	
	print("INFO: DeliveryCreationHandler loading...");
	self.saveTable = saveLoadHandler:ReadSavedStringAsTable("deliveryCreationHandlerSaveTable");
	
	-- re-add virtual teams
	
	for team, techName in pairs(self.saveTable.virtualTeams) do
		self:AddVirtualTeam(team, techName);
	end
	
	-- redo adding and removing presets
	
	for team, presetsTable in pairs(self.saveTable.teamRemovedPresets) do
		for i, presetName in pairs(self.saveTable.teamRemovedPresets[team]) do
			self:RemoveAvailablePreset(team, presetName, true);
		end
	end
	
	for team, presetsTable in pairs(self.saveTable.teamAddedPresets) do
		for i, presetTable in pairs(self.saveTable.teamAddedPresets[team]) do
			self:AddAvailablePreset(team, presetTable.PresetName, presetTable.ClassName, presetTable.TechName, true);
		end
	end
	
	print("INFO: DeliveryCreationHandler loaded!");
	
end

function DeliveryCreationHandler:OnSave(saveLoadHandler)
	
	print("INFO: DeliveryCreationHandler saving...");
	saveLoadHandler:SaveTableAsString("deliveryCreationHandlerSaveTable", self.saveTable);
	print("INFO: DeliveryCreationHandler saved!");
	
end

function DeliveryCreationHandler:AddVirtualTeam(team, techName)

	if team and techName then
	
		if self.verboseLogging then
			print("INFO: DeliveryCreationHandler is adding virtual team " .. team .. " with tech name " .. techName);
		end

		if team >= -1 and team < 5 then
			print("ERROR: DeliveryCreationHandler tried to add a virtual team within the range of real teams!");
			return false;
		end
		
		team = math.floor(team);
		
		if self.saveTable.virtualTeams[team] then
			print("ERROR: DeliveryCreationHandler tried to add a virtual team that already existed: " .. team .. "!");
			return false;
		else
			self.saveTable.virtualTeams[team] = techName;
		end

		local moduleID = PresetMan:GetModuleID(techName);
		if moduleID ~= -1 then
			self.teamTechTable[team] = PresetMan:GetDataModule(moduleID);
		else
			if not techName == "All" then
				print("WARNING: DeliveryCreationHandler could not find module " .. techName .. " when adding a virtual team. Defaulting to All.");
			end
			self.teamTechTable[team] = {["FileName"] = "All"}; -- master of ghetto
		end
		self.teamTechIDTable[team] = moduleID;
		
		self.saveTable.teamRemovedPresets[team] = {};
		self.saveTable.teamAddedPresets[team] = {};
		
		self.saveTable.teamInfantryTypeWeights[team] = {};
		
		-- 10 is standard weighting
		
		self.saveTable.teamInfantryTypeWeights[team].Light = 10;
		self.saveTable.teamInfantryTypeWeights[team].Medium = 10;
		self.saveTable.teamInfantryTypeWeights[team].Heavy = 8;
		self.saveTable.teamInfantryTypeWeights[team].CQB = 7;
		self.saveTable.teamInfantryTypeWeights[team].Scout = 0; -- has to be explicitly enabled
		self.saveTable.teamInfantryTypeWeights[team].Sniper = 5;
		self.saveTable.teamInfantryTypeWeights[team].Grenadier = 5;
		self.saveTable.teamInfantryTypeWeights[team].Engineer = 3;
		
		self.saveTable.teamExtraItemChances[team] = {};
		self.saveTable.teamExtraItemChances[team].Medikit = 0.5;
		self.saveTable.teamExtraItemChances[team].BreachingTool = 0.25;
		self.saveTable.teamExtraItemChances[team].Grenade = 0.25;
		self.saveTable.teamExtraItemChances[team].Digger = 0.15;
		
		local iterator = self.teamTechTable[team].Presets;
		-- handle -All-
		if not iterator then
			iterator = PresetMan:GetAllEntities();
		end
	
		self.teamPresetTables[team] = {};
		
		self.teamPresetTables[team]["Craft - Dropships"] = {};
		self.teamPresetTables[team]["Craft - Rockets"] = {};
		self.teamPresetTables[team]["Craft - Crates"] = {};
		
		self.teamPresetTables[team]["Weapons - Primary"] = {};
		self.teamPresetTables[team]["Weapons - Secondary"] = {};
		self.teamPresetTables[team]["Weapons - Light"] = {};
		self.teamPresetTables[team]["Weapons - Heavy"] = {};
		self.teamPresetTables[team]["Weapons - Sniper"] = {};
		self.teamPresetTables[team]["Weapons - CQB"] = {};
		self.teamPresetTables[team]["Weapons - Explosive"] = {};
		
		self.teamPresetTables[team]["Shields"] = {};

		self.teamPresetTables[team]["Bombs"] = {};	
		self.teamPresetTables[team]["Bombs - Grenades"] = {};
		
		self.teamPresetTables[team]["Tools"] = {};
		self.teamPresetTables[team]["Tools - Diggers"] = {};
		self.teamPresetTables[team]["Tools - Breaching"] = {};
		
		self.teamPresetTables[team]["Actors - AHuman"] = {};
		self.teamPresetTables[team]["Actors - ACrab"] = {};
		self.teamPresetTables[team]["Actors - Light"] = {};
		self.teamPresetTables[team]["Actors - Heavy"] = {};
		self.teamPresetTables[team]["Actors - Mecha"] = {};
		self.teamPresetTables[team]["Actors - Turrets"] = {};
		
		for entity in iterator do
			if IsMOSRotating(entity) and ToMOSRotating(entity).Buyable and ToMOSRotating(entity).BuyableMode ~= 2 then
			
				local entityInfoTable = {};
			
				for group in entity.Groups do
					if self.teamPresetTables[team][group] then
						entityInfoTable.PresetName = entity.PresetName;
						entityInfoTable.ClassName = entity.ClassName;
						table.insert(self.teamPresetTables[team][group], entityInfoTable);
					end
				end
				
				if IsAHuman(entity) and not entity:IsInGroup("Brains") then
					entityInfoTable.PresetName = entity.PresetName;
					entityInfoTable.ClassName = entity.ClassName;
					table.insert(self.teamPresetTables[team]["Actors - AHuman"], entityInfoTable);
				elseif IsACrab(entity) and not entity:IsInGroup("Brains") then
					entityInfoTable.PresetName = entity.PresetName;
					entityInfoTable.ClassName = entity.ClassName;
					table.insert(self.teamPresetTables[team]["Actors - ACrab"], entityInfoTable);	
				elseif IsACDropShip(entity) then
					entityInfoTable.PresetName = entity.PresetName;
					entityInfoTable.ClassName = entity.ClassName;
					table.insert(self.teamPresetTables[team]["Craft - Dropships"], entityInfoTable);
				elseif IsACRocket(entity) and entity:IsInGroup("Craft - Rockets") then
					entityInfoTable.PresetName = entity.PresetName;
					entityInfoTable.ClassName = entity.ClassName;
					table.insert(self.teamPresetTables[team]["Craft - Rockets"], entityInfoTable);
				elseif IsACRocket(entity) and entity:IsInGroup("Craft - Crates") then
					entityInfoTable.PresetName = entity.PresetName;
					entityInfoTable.ClassName = entity.ClassName;
					table.insert(self.teamPresetTables[team]["Craft - Crates"], entityInfoTable);	
				end
				
			end
		end		
		
	else
		print("ERROR: DeliveryCreationHandler tried to add a virtual team with no team number or tech name!");
		return false;
	end
	
	if self.verboseLogging then
		print("INFO: DeliveryCreationHandler successfully added virtual team " .. team .. " with tech name " .. techName);
	end
	
	return true;
	
end

function DeliveryCreationHandler:ReplaceInfantryTypeWeightsTable(team, newWeights)

	-- keeps old weights if none are given for a particular type

	if team and newWeights and type(newWeights) == "table" then
	
		local weightTable = self.saveTable.teamInfantryTypeWeights[team];
	
		weightTable.Light = newWeights.Light or weightTable.Light;
		weightTable.Medium = newWeights.Medium or weightTable.Medium;
		weightTable.Heavy = newWeights.Heavy or weightTable.Heavy;
		weightTable.CQB = newWeights.CQB or weightTable.CQB;
		weightTable.Scout = newWeights.Scout or weightTable.Scout;
		weightTable.Sniper = newWeights.Sniper or weightTable.Sniper;
		weightTable.Grenadier = newWeights.Grenadier or weightTable.Grenadier;
		weightTable.Engineer = newWeights.Engineer or weightTable.Engineer;	

		self.saveTable.teamInfantryTypeWeights[team] = weightTable; -- not sure if this is required...
		
		return true;
	else
		print("DeliveryCreationHandler tried to replace infantry type weights, but wasn't given both a team and a table!");
		return false;
	end
	
end

function DeliveryCreationHandler:AddAvailablePreset(team, presetName, className, techName, doNotSaveNewEntry)

	-- it'd be great to just need a PresetName here, but the game's way of resolving that to actual
	-- usable entities kinda sucks/is non-existent, so..
	if team and presetName and className and techName then
	
		-- make sure we don't already have it

		for i, groupTable in pairs(self.teamPresetTables[team]) do
			for presetIndex, presetTable in pairs(groupTable) do
				if presetTable.PresetName == presetName then
					print("DeliveryCreationHandler tried to add an available preset that was already there!");
					return false;
				end
			end
		end
	
		-- we need to create the preset to know its groups.
	
		local createFunc = "Create" .. className;	
		local preset = _G[createFunc](presetName, techName);

		local presetInfoTable = {};

		for group in preset.Groups do
			if self.teamPresetTables[team][group] then
				presetInfoTable.PresetName = preset.PresetName;
				presetInfoTable.ClassName = preset.ClassName;
				table.insert(self.teamPresetTables[team][group], presetInfoTable);
			end
		end

		local presetTable = {};
		presetTable.PresetName = preset.PresetName;
		presetTable.ClassName = preset.ClassName;
		presetTable.TechName = preset.ModuleName; -- needed for saveloading re-adding
		
		-- this is true when redoing this stuff from OnLoad, so avoid duplicates entries
		if not doNotSaveNewEntry then
			table.insert(self.saveTable.teamAddedPresets[team], presetTable);
		end
		return true;

	else
		print("DeliveryCreationHandler tried to add an available preset but was not given all of a team, presetName, className, and techName!");
		return false;
	end
	
end

function DeliveryCreationHandler:RemoveAvailablePreset(team, presetName, doNotSaveNewEntry)

	-- check through every group and delete the preset with the given presetname from every one

	local found = false;

	for i, groupTable in pairs(self.teamPresetTables[team]) do
		for presetIndex, presetTable in pairs(groupTable) do
			if presetTable.PresetName == presetName then
				table.remove(groupTable, presetIndex);
				found = true;
			end
		end
	end
	
	-- we just need to save the presetname here for saving/loading removed stuff
	if found then
		-- this is true when redoing this stuff from OnLoad, so avoid duplicates entries
		if not doNotSaveNewEntry then
			table.insert(self.saveTable.teamRemovedPresets[team], presetName);
		end
		print("INFO: DeliveryCreationHandler deleted preset " .. presetName .. " from team " .. team);
		return true;
	else
		return false;
	end
	
end
	

function DeliveryCreationHandler:CheckTwoGroupIntersections(team, group1, group2, returnRandomSelection)

	local groupTable1 = self.teamPresetTables[team][group1];
	local groupTable2 = self.teamPresetTables[team][group2];
	
	local intersectedPresetNameTable = {};
	
	for k1, v1 in pairs(groupTable1) do
		for k2, v2 in pairs(groupTable2) do
			if v1.PresetName == v2.PresetName then
				table.insert(intersectedPresetNameTable, v1);
			end
		end
	end
	
	if #intersectedPresetNameTable > 0 then	
		if returnRandomSelection then
			return intersectedPresetNameTable[math.random(1, #intersectedPresetNameTable)];
		else
			return intersectedPresetNameTable;
		end
	else
		return false;
	end
	
end

function DeliveryCreationHandler:GiveActorRandomAdditions(team, actor, multiplier)

	-- Random additions based on team-wide chances, multiplied by magic numbers for each infantry type
	-- Given chances low enough, an infantry type can have an addition disabled

	local rand = math.random();
	if rand < self.saveTable.teamExtraItemChances[team].Grenade * multiplier then
		actor:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.teamTechTable[team].FileName))
		return;
	end
	
	if rand < self.saveTable.teamExtraItemChances[team].Medikit * multiplier then
		actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
		return;
	end
	
	if rand < self.saveTable.teamExtraItemChances[team].Digger * multiplier then
		presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Tools - Diggers", "Tools - Diggers", "Tools - Diggers", "Tools - Diggers");
		
		local weapon = _G[createFunc](presetName, techName);
		actor:AddInventoryItem(weapon)
		return;
	end
	
	if rand < self.saveTable.teamExtraItemChances[team].BreachingTool * multiplier then
		if math.random() < 0.75 then
			actor:AddInventoryItem(RandomHDFirearm("Tools - Breaching", self.teamTechTable[team].FileName));
		else
			actor:AddInventoryItem(RandomTDExplosive("Tools - Breaching", self.teamTechTable[team].FileName));
		end
	end
	
end

function DeliveryCreationHandler:SelectPresetByGroupPair(team, primaryGroup, secondaryGroup, fallbackGroup, baseFallbackGroup)

	-- This will attempt to find one preset of both groups
	-- Then one preset of the primary group
	-- Then one preset of the secondary group, etc.
	
	-- Having one of the later groups be the same as an earlier group will in practice
	-- skip that step
	
	local actingTech = self.teamTechTable[team];
	
	local presetTable = self:CheckTwoGroupIntersections(team, primaryGroup, secondaryGroup, true)
	if not presetTable then
		actingTech = self.teamTechTable[team];
		actingGroupTable = self.teamPresetTables[team][primaryGroup];
		
		if #actingGroupTable == 0 then
			actingGroupTable = self.teamPresetTables[team][secondaryGroup];
		end
		if #actingGroupTable == 0 then
			actingGroupTable = self.teamPresetTables[team][fallbackGroup];
		end
		if #actingGroupTable == 0 then
			--print("WARNING: DeliveryCreationHandler fell back to Base.rte definitions when trying to create an object of primary group " .. primaryGroup .. " for team " .. team .. "!");
			-- Base.rte has to have something, we are in trouble otherwise
			actingGroupTable = self.teamPresetTables[-1][baseFallbackGroup];
			actingTech = self.teamTechTable[-1];
		end
		
		presetTable = actingGroupTable[math.random(1, #actingGroupTable)];
	end
	
	local presetName = presetTable.PresetName;
	local className = presetTable.ClassName;
	local createFunc = "Create" .. className;	
	local techName = actingTech.FileName or "All";
	
	
	--print(presetName)
	--print(createFunc)
	--print(techName)
	--print("FALLBACK GROUP: " .. baseFallbackGroup); 
	
	return presetName, createFunc, techName;

end

-- TODO: Use this when crabs don't suck
function DeliveryCreationHandler:CreateCrab(team)
	local actor = RandomACrab("Actors - Mecha", self.teamTechTable[team].FileName);
	if actor then
		actor.AIMode = Actor.AIMODE_BRAINHUNT;
		actor.Team = team;
		return actor;
	end
end

function DeliveryCreationHandler:CreateRandomInfantry(team)

	-- random weighted select
	local totalPriority = 0;
	for infantryType, weight in pairs(self.saveTable.teamInfantryTypeWeights[team]) do
		totalPriority = totalPriority + weight;
	end
	
	local randomSelect = math.random(1, totalPriority);
	for infantryType, func in pairs(self.infantryTypeFunctionTable) do
		randomSelect = randomSelect - self.saveTable.teamInfantryTypeWeights[team][infantryType];
		if randomSelect <= 0 then
			self.infantryFunc = func;
			--print("selected random inf: " .. infantryType);
			break;
		end
	end
	
	if self.infantryFunc then
		local actor = self:infantryFunc(team);
		return actor;
	else
		print("Something when wrong when DeliveryCreationHandler was picking a random infantry type to create!");
		return false;
	end
	
end

function DeliveryCreationHandler:CreateLightInfantry(team)
	
	local presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Actors - Light", "Actors - AHuman", "Actors - AHuman", "Actors - AHuman");
	local actor = _G[createFunc](presetName, techName);
	
	-- Weapons
	
	-- Primary light
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Light", "Weapons - Primary", "Weapons - Primary", "Weapons - Primary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	-- Secondary light
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Secondary", "Weapons - Light", "Weapons - Secondary", "Weapons - Secondary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	self:GiveActorRandomAdditions(team, actor, 1.5);

	actor.AIMode = Actor.AIMODE_BRAINHUNT;
	actor.Team = team;
	return actor;
	
end

function DeliveryCreationHandler:CreateHeavyInfantry(team)
	
	local presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Actors - Heavy", "Actors - AHuman", "Actors - AHuman", "Actors - AHuman");
	local actor = _G[createFunc](presetName, techName);
	
	-- Weapons
	
	-- Primary heavy
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Heavy", "Weapons - Primary", "Weapons - Primary", "Weapons - Primary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	-- Secondary
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Secondary", "Weapons - Secondary", "Weapons - Secondary", "Weapons - Secondary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	if math.random() < 0.1 then
		-- Extra shield
		presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Shields", "Shields", "Shields", "Shields");
		
		local shield = _G[createFunc](presetName, techName);
		actor:AddInventoryItem(shield)
	end
	
	self:GiveActorRandomAdditions(team, actor, 1);

	actor.AIMode = Actor.AIMODE_BRAINHUNT;
	actor.Team = team;
	return actor;
	
end

function DeliveryCreationHandler:CreateMediumInfantry(team)
	
	local presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Actors - Heavy", "Actors - AHuman", "Actors - AHuman", "Actors - AHuman");
	local actor = _G[createFunc](presetName, techName);
	
	-- Weapons
	
	-- Primary light
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Light", "Weapons - Primary", "Weapons - Primary", "Weapons - Primary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	-- Secondary
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Secondary", "Weapons - Secondary", "Weapons - Secondary", "Weapons - Secondary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	self:GiveActorRandomAdditions(team, actor, 1);

	actor.AIMode = Actor.AIMODE_BRAINHUNT;
	actor.Team = team;
	return actor;
	
end

function DeliveryCreationHandler:CreateCQBInfantry(team)
	
	local actorGroup = math.random() < 0.5 and "Actors - Heavy" or "Actors - Light";
	local presetName, createFunc, techName = self:SelectPresetByGroupPair(team, actorGroup, "Actors - AHuman", "Actors - AHuman", "Actors - AHuman");
	local actor = _G[createFunc](presetName, techName);
	
	-- Weapons
	
	-- Primary CQB
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - CQB", "Weapons - Primary", "Weapons - Primary", "Weapons - Primary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	-- Secondary
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Secondary", "Weapons - Secondary", "Weapons - Secondary", "Weapons - Secondary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	-- Extra breaching tool
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Tools - Breaching", "Tools - Breaching", "Tools - Breaching", "Tools - Breaching");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	self:GiveActorRandomAdditions(team, actor, 1);

	actor.AIMode = Actor.AIMODE_BRAINHUNT;
	actor.Team = team;
	return actor;
	
end

function DeliveryCreationHandler:CreateScoutInfantry(team)
	local presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Actors - Light", "Actors - AHuman", "Actors - AHuman", "Actors - AHuman");
	local actor = _G[createFunc](presetName, techName);
	
	-- Weapons
	
	-- Secondary
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Secondary", "Weapons - Secondary", "Weapons - Secondary", "Weapons - Secondary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	-- Extra medikit
	actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
	
	self:GiveActorRandomAdditions(team, actor, 1.7);

	actor.AIMode = Actor.AIMODE_BRAINHUNT;
	actor.Team = team;
	return actor;
end

function DeliveryCreationHandler:CreateSniperInfantry(team)
	
	local actorGroup = math.random() < 0.25 and "Actors - Heavy" or "Actors - Light";
	local presetName, createFunc, techName = self:SelectPresetByGroupPair(team, actorGroup, "Actors - AHuman", "Actors - AHuman", "Actors - AHuman");
	local actor = _G[createFunc](presetName, techName);
	
	-- Weapons
	
	-- Primary Sniper
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Sniper", "Weapons - Primary", "Weapons - Primary", "Weapons - Primary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	-- Secondary
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Secondary", "Weapons - Secondary", "Weapons - Secondary", "Weapons - Secondary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	self:GiveActorRandomAdditions(team, actor, 1);

	actor.AIMode = Actor.AIMODE_BRAINHUNT;
	actor.Team = team;
	return actor;
	
end

function DeliveryCreationHandler:CreateGrenadierInfantry(team)
	local actorGroup = math.random() < 0.5 and "Actors - Heavy" or "Actors - Light";
	local presetName, createFunc, techName = self:SelectPresetByGroupPair(team, actorGroup, "Actors - AHuman", "Actors - AHuman", "Actors - AHuman");
	local actor = _G[createFunc](presetName, techName);
	
	-- Weapons
	
	-- Primary explosive
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Explosive", "Weapons - Primary", "Weapons - Primary", "Weapons - Primary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	-- Secondary explosive (like the imperatus one!)
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Secondary", "Weapons - Explosive", "Weapons - Secondary", "Weapons - Secondary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	-- Extra grenade
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Bombs - Grenades", "Bombs - Grenades", "Bombs - Grenades", "Bombs - Grenades");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	self:GiveActorRandomAdditions(team, actor, 0.75);

	actor.AIMode = Actor.AIMODE_BRAINHUNT;
	actor.Team = team;
	return actor;
end

function DeliveryCreationHandler:CreateEngineerInfantry(team)

	local presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Actors - Light", "Actors - AHuman", "Actors - AHuman", "Actors - AHuman");
	local actor = _G[createFunc](presetName, techName);
	
	-- Weapons
	
	-- Primary light
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Weapons - Primary", "Weapons - Light", "Weapons - Primary", "Weapons - Primary");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	-- Digger
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Tools - Diggers", "Tools - Diggers", "Tools - Diggers", "Tools - Diggers");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	-- Breaching tool
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, "Tools - Breaching", "Tools - Breaching", "Tools - Breaching", "Tools - Breaching");
	
	local weapon = _G[createFunc](presetName, techName);
	actor:AddInventoryItem(weapon)
	
	-- No additions
	--self:GiveActorRandomAdditions(team, actor, 0.75);

	actor.AIMode = Actor.AIMODE_BRAINHUNT;
	actor.Team = team;
	return actor;
end

function DeliveryCreationHandler:CreateSquad(team, squadCountOrTypeTable, squadType)

	local squadTable = {};
	local goldCost = 0;
	
	local crabToHumanSpawnRatio = self.Activity:GetCrabToHumanSpawnRatio(self.teamTechIDTable[team]);
	
	if type(squadCountOrTypeTable) == "table" then
	
		for k, infantryType in pairs(squadCountOrTypeTable) do
			local actor;
			
			self.infantryFunc = self.infantryTypeFunctionTable[infantryType];
			
			if not self.infantryFunc then
				print("DeliveryCreationHandler tried to create infantry of an invalid type!");
				return false;
			end
			
			actor = self:infantryFunc(team);
			
			if actor then
				table.insert(squadTable, actor)
				goldCost = goldCost + ToSceneObject(actor):GetTotalValue(self.teamTechIDTable[team], 1);
			end
		end
	
	else
	
		if squadCountOrTypeTable == nil then
			squadCountOrTypeTable = math.random(2, 3);
		end

		for i = 1, squadCountOrTypeTable do
			local actor;
			if squadType then
				self.infantryFunc = self.infantryTypeFunctionTable[squadType];
				
				if not self.infantryFunc then
					print("DeliveryCreationHandler tried to create infantry of an invalid type!");
					return false;
				end				
					
				actor = self:infantryFunc(team);
			elseif math.random() < crabToHumanSpawnRatio then
				-- TODO make crabs not suck
				--actor = self:CreateCrab(team);
				actor = self:CreateRandomInfantry(team);
			else
				actor = self:CreateRandomInfantry(team);
			end
			if actor then
				table.insert(squadTable, actor)
				goldCost = goldCost + ToSceneObject(actor):GetTotalValue(self.teamTechIDTable[team], 1);
			end
		end
	end
	
	--print("buydoor goldcost: " .. goldCost);
	return squadTable, goldCost

end

function DeliveryCreationHandler:CreateEliteSquad(team, squadCountOrTypeTable, squadType)

	local squadTable, goldCost = self:CreateSquad(team, squadCountOrTypeTable, squadType)
	
	for k, actor in pairs(squadTable) do
		actor:SetNumberValue("AIAimSpeed", 0.04);
		actor:SetNumberValue("AIAimSkill", 0.04);
		actor:SetNumberValue("AISkill", 100);
		actor.MaxHealth = actor.MaxHealth * 1.25;
		actor.Health = actor.MaxHealth;
	end
	
	return squadTable, goldCost
	
end

function DeliveryCreationHandler:CreateSquadWithCraft(team, forceRocketUsage, squadCountOrTypeTable, squadType)

	local craftGroup = "Craft - Dropships";
	if forceRocketUsage then
		craftGroup = "Craft - Rockets";
	end
	presetName, createFunc, techName = self:SelectPresetByGroupPair(team, craftGroup, craftGroup, craftGroup, craftGroup);
	
	local craft = _G[createFunc](presetName, techName);
	craft.Team = team;
	--print(craft)
	
	local squad = self:CreateSquad(team, squadCountOrTypeTable, squadType);
	for i = 1, #squad do
		craft:AddInventoryItem(squad[i]);
		if craft.InventoryMass > craft.MaxInventoryMass then
			break;
		end
	end
	
	local goldCost = ToSceneObject(craft):GetTotalValue(self.teamTechIDTable[team], 1);
	
	return craft, squad, goldCost
end

function DeliveryCreationHandler:CreateEliteSquadWithCraft(team, forceRocketUsage, squadCountOrTypeTable, squadType)

	local craft, squad, goldCost = self:CreateSquadWithCraft(team, forceRocketUsage, squadCountOrTypeTable, squadType)
	
	for k, actor in pairs(squad) do
		actor:SetNumberValue("AIAimSpeed", 0.04);
		actor:SetNumberValue("AIAimSkill", 0.04);
		actor:SetNumberValue("AISkill", 100);
		actor.MaxHealth = actor.MaxHealth * 1.25;
		actor.Health = actor.MaxHealth;
	end
	
	return craft, squad, goldCost
end

return DeliveryCreationHandler:Create();