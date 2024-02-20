require("Utilities");
require("Scenes/Objects/Bunkers/BunkerSystems/Automovers/GlobalAutomoverFunctions");

local automoverUtilityFunctions = {};
local automoverActorFunctions = {};
local automoverVisualEffectsFunctions = {};local automoverUIFunctions = {};

function Create(self)
	self.movementSpeedMin = 4;
	self.movementSpeedMax = 16;
	self.massLimitMin = 100;
	self.massLimitMax = 5000;
	self.visualEffectsSizes = {
		small = 1,
		medium = 2,
		large = 3,
	};

	----------------------------------------
	--INI and Pie Menu Configurable Fields--
	----------------------------------------
	self.displayingInfoUI = self:NumberValueExists("DisplayInfoUI");
	self.acceptsAllTeams = self:NumberValueExists("AcceptsAllTeams") and self:GetNumberValue("AcceptsAllTeams") ~= 0 or true;
	self.acceptsCrafts =  self:NumberValueExists("AcceptsCrafts") and self:GetNumberValue("AcceptsCrafts") ~= 0 or false;
	self.humansRemainUpright = self:NumberValueExists("HumansRemainUpright") and self:GetNumberValue("HumansRemainUpright") ~= 0 or false;
	self.movementSpeed = self:NumberValueExists("MovementSpeed") and math.max(self.movementSpeedMin, math.min(self.movementSpeedMax, self:GetNumberValue("MovementSpeed"))) or (self.movementSpeedMin);
	self.massLimit = self:NumberValueExists("MassLimit") and math.max(self.massLimitMin, math.min(self.massLimitMax, self:GetNumberValue("MassLimit"))) or (self.massLimitMin * 5);
	self.visualEffectsSelectedType = self:StringValueExists("VisualEffectsType") and self:GetStringValue("VisualEffectsType"):lower() or "chevron";
	self.visualEffectsSelectedSize = self:NumberValueExists("VisualEffectsSize") and math.max(self.visualEffectsSizes.small, math.min(self.visualEffectsSizes.large, self:GetNumberValue("VisualEffectsSize"))) or self.visualEffectsSizes.small;

	---------------------------
	--INI Configurable Fields--
	---------------------------
	self.actorUnstickingDisabled = self:NumberValueExists("ActorUnstickingDisabled") and self:GetNumberValue("ActorUnstickingDisabled") ~= 0 or false;
	self.slowActorVelInNoneMovementDirectionsWhenInZoneBoxDisabled = self:NumberValueExists("SlowActorVelInNoneMovementDirectionsWhenInZoneBoxDisabled") and self:GetNumberValue("SlowActorVelInNoneMovementDirectionsWhenInZoneBoxDisabled") ~= 0 or false;

	self.infoUIUseSmallText = self:NumberValueExists("InfoUIUseSmallText") and self:GetNumberValue("InfoUIUseSmallText") ~= 0 or false;
	self.infoUIBGColour = self:NumberValueExists("InfoUIBGColour") and self:GetNumberValue("InfoUIBGColour") or 127;
	self.infoUIOutlineWidth = self:NumberValueExists("InfoUIOutlineWidth") and self:GetNumberValue("InfoUIOutlineWidth") or 2;
	self.infoUIOutlineColour = self:NumberValueExists("InfoUIOutlineColour") and self:GetNumberValue("InfoUIOutlineColour") or 71;
	self.infoUITransparency = self:NumberValueExists("InfoUITransparency") and math.max(0, math.min(100, self:GetNumberValue("InfoUITransparency"))) or 0;

	-------------
	--Constants--
	-------------
	--TODO make this an actual power system.
	AutomoverData[self.Team].energyLevel = 100;

	self.currentActivity = ActivityMan:GetActivity();
	self.checkWrapping = SceneMan.SceneWrapsX or SceneMan.SceneWrapsY;

	self.initialPieMenuFullInnerRadius = self.PieMenu.FullInnerRadius;

	self.movementModes = {
		freeze = 0,
		move = 1,
		unstickActor = 2,
		teleporting = 3,
		leaveAutomovers = 4,
	};

	self.oppositeDirections = {
		[Directions.Up] = Directions.Down,
		[Directions.Down] = Directions.Up,
		[Directions.Left] = Directions.Right,
		[Directions.Right] = Directions.Left,
	};

	self.visualEffectsMinimumNumberOfSpacesNeededForDrawingObjectEffects = 3;

	self.sceneWaypointThresholdForTreatingTargetAsReached = 12;
	self.movableObjectWaypointThresholdForTreatingTargetAsReached = 48;

	self.movementAcceleration = self.movementSpeedMin * 0.1;

	self.uiLineHeight = { [true] = 10, [false] = 15 };

	-----------------
	--General Setup--
	-----------------
	for _, functionTable in ipairs({ automoverUtilityFunctions, automoverVisualEffectsFunctions, automoverActorFunctions, automoverUIFunctions }) do
		for functionName, functionReference in pairs(functionTable) do
			self[functionName] = functionReference;
		end
	end

	for actor in MovableMan.Actors do
		if actor.PresetName:find("Automover Controller") and actor.Team == self.Team and actor.UniqueID ~= self.UniqueID then
			ActivityMan:GetActivity():SetTeamFunds(ActivityMan:GetActivity():GetTeamFunds(self.Team) + actor:GetGoldValue(0, 0), self.Team);
			actor.ToDelete = true;
		end
	end

	for _, particleCollection in pairs({ MovableMan.Particles, MovableMan.AddedParticles }) do
		for node in particleCollection do
			if node and IsMOSRotating(node) and node.Team == team and node:IsInGroup("Automover Nodes") then
				ToMOSRotating(node):SetNumberValue("shouldReaddNode", 1);
			end
		end
	end

	self.visualEffectsConfig = {
		chevron = {},
		chevronNarrow = {},
	};

	local function addVisualEffectsConfig(self, visualEffectsSize, visualEffectsType)
		local visualEffectsConfigSizeTable = {
			coolDownInterval = visualEffectsSize * 1000,
			spriteObject = nil,
			spriteSize = 0,
			moveTimer = Timer(),
			minFrame = 0,
			maxFrame = 7,
		};
		if visualEffectsSize == self.visualEffectsSizes.small then
			visualEffectsConfigSizeTable.spriteObject = CreateMOSParticle("Automover Chevron Small", "Base.rte");
			visualEffectsConfigSizeTable.spriteSize = visualEffectsType == "chevronNarrow" and 5.33 or 16;
			visualEffectsConfigSizeTable.moveTimer:SetSimTimeLimitMS(visualEffectsType == "chevronNarrow" and 5 or 30);
		elseif visualEffectsSize == self.visualEffectsSizes.medium then
			visualEffectsConfigSizeTable.spriteObject = CreateMOSParticle("Automover Chevron Medium", "Base.rte");
			visualEffectsConfigSizeTable.spriteSize = visualEffectsType == "chevronNarrow" and 10.67 or 32;
			visualEffectsConfigSizeTable.moveTimer:SetSimTimeLimitMS(visualEffectsType == "chevronNarrow" and 30 or 60);
		elseif visualEffectsSize == self.visualEffectsSizes.large then
			visualEffectsConfigSizeTable.spriteObject = CreateMOSParticle("Automover Chevron Large", "Base.rte");
			visualEffectsConfigSizeTable.spriteSize = visualEffectsType == "chevronNarrow" and 16 or 48;
			visualEffectsConfigSizeTable.moveTimer:SetSimTimeLimitMS(visualEffectsType == "chevronNarrow" and 55 or 80);
		end

		self.visualEffectsConfig[visualEffectsType][visualEffectsSize] = visualEffectsConfigSizeTable;
	end

	for _, visualEffectsSize in pairs(self.visualEffectsSizes) do
		addVisualEffectsConfig(self, visualEffectsSize, "chevron");
		addVisualEffectsConfig(self, visualEffectsSize, "chevronNarrow");
	end

	self.visualEffectsData = {};

	self.obstructionCheckCoroutine = coroutine.create(self.checkAllObstructions);
	self.obstructionCheckTimer = Timer(10000);
	self.obstructionsFoundDuringCheck = false;

	self.addAllBoxesCoroutine = coroutine.create(self.addAllBoxes);
	self.addAllPathsCoroutine = coroutine.create(self.addAllPaths);
	self.addAllBoxesAndPathsTimer = Timer(50);
	self.allBoxesAdded = false;
	self.allPathsAdded = false;

	if not SceneMan.Scene:HasArea("NoGravityArea") then
		local noGravArea = Area("NoGravityArea");
		SceneMan.Scene:SetArea(noGravArea);
	end

	self.combinedAutomoverArea = Area();
	self.pathTable = {};

	self.affectedActors = {};
	self.affectedActorsCount = 0;

	self.newActorCheckTimer = Timer(100);
	self.actorMovementUpdateTimer = Timer(15);

	self.heldInputTimer = Timer(50);

	self.leaveAutomoverNetworkPieSlice = CreatePieSlice("Leave Automover Network", "Base.rte");
	self.chooseTeleporterPieSlice = CreatePieSlice("Choose Teleporter", "Base.rte");
end

function Update(self)
	self.Frame = 0;

	self:handlePieButtons();

	if self.currentActivity.ActivityState == Activity.EDITING then
		self:updateActivityEditingMode();
	elseif self.currentActivity.ActivityState == Activity.RUNNING then
		if self.obstructionCheckTimer:IsPastSimTimeLimit() then
			--self:checkForObstructions();
			self.obstructionCheckTimer:Reset();
		end

		if (not self.allBoxesAdded or not self.allPathsAdded) and self.addAllBoxesAndPathsTimer:IsPastSimTimeLimit() then
			self:addAllBoxesAndPaths();
			self.addAllBoxesAndPathsTimer:Reset();
		end

		if self.allBoxesAdded and self.allPathsAdded then
			if self.newActorCheckTimer:IsPastSimTimeLimit() then
				self:checkForNewActors();
				self.newActorCheckTimer:Reset();
			end

			if self.actorMovementUpdateTimer:IsPastSimTimeLimit() then
				self:actorMovementUpdate();
				self.actorMovementUpdateTimer:Reset();
			end

			self:updateVisualEffects();
		end
	end

	if self.displayingInfoUI then
		self.PieMenu.FullInnerRadius = self:updateInfoUI();
	else
		self.PieMenu.FullInnerRadius = self.initialPieMenuFullInnerRadius;
	end
	self.Frame = 1;
end

function Destroy(self)
	for actor in MovableMan.Actors do
		if actor.PresetName:find("Automover Controller") and actor.Team == self.Team and actor.UniqueID ~= self.UniqueID then
			return;
		end
	end
	AutomoverData[self.Team].energyLevel = 0;
	AutomoverData[self.Team].nodeData = {};
	AutomoverData[self.Team].nodeDataCount = 0;
	AutomoverData[self.Team].teleporterNodes = {};
	AutomoverData[self.Team].teleporterNodesCount = 0;
end

automoverActorFunctions.actorMovementUpdate = function(self)
	tracy.ZoneBegin();
	for actorUniqueID, actorData in pairs(self.affectedActors) do
		local actor = actorData.actor;
		if not MovableMan:ValidMO(actor) or actor.Health <= 0 or not self.combinedAutomoverArea:IsInside(actor.Pos) then
			self:removeActorFromAutomoverTable(actor, actorUniqueID);
		else
			if actor:NumberValueExists("Automover_LeaveAutomoverNetwork") then
				self:setActorMovementModeToLeaveAutomovers(actorData);
				actor:RemoveNumberValue("Automover_LeaveAutomoverNetwork");
			end
			if actor:IsPlayerControlled() and actor:NumberValueExists("Automover_ChooseTeleporter") then
				self:setupManualTeleporterData(actorData);
				actor:RemoveNumberValue("Automover_ChooseTeleporter");
			end

			if actorData.movementMode ~= self.movementModes.leaveAutomovers then
				if actor:IsPlayerControlled() then
					local closestNode = self:findClosestNode(actor.Pos, nil, false, false);
					if closestNode ~= nil and AutomoverData[self.Team].teleporterNodes[closestNode] ~= nil and AutomoverData[self.Team].nodeData[closestNode].zoneBox:IsWithinBox(actor.Pos) then
						actor.PieMenu:AddPieSliceIfPresetNameIsUnique(self.chooseTeleporterPieSlice, self);
					else
						actor.PieMenu:RemovePieSlicesByPresetName(self.chooseTeleporterPieSlice.PresetName);
					end

					if actorData.manualTeleporterData then
						self:chooseTeleporterForPlayerControlledActor(actorData);
						actorData.movementMode = self.movementModes.teleporting;
					else
						actorData.movementMode = self.movementModes.freeze;
					end
				elseif actorData.movementMode == self.movementModes.teleporting then
					self:updateDirectionsFromWaypoints(actorData);
				end

				if actorData.movementMode ~= self.movementModes.teleporting then -- Note: Teleporting movement mode can be set manually or by waypoints. The point is that you can't move the Actor while it's currently teleporting.
					self:updateDirectionsFromActorControllerInput(actorData);
				end

				local actorController = actor:GetController();
				actorController:SetState(Controller.MOVE_LEFT, false);
				actorController:SetState(Controller.MOVE_RIGHT, false);
				actorController:SetState(Controller.MOVE_UP, false);
				actorController:SetState(Controller.MOVE_DOWN, false);
				actorController:SetState(Controller.BODY_JUMP, false);
				actorController:SetState(Controller.BODY_JUMPSTART, false);

				if IsACDropShip(actor) then
					actor = ToACDropShip(actor);
					actor.RightEngine:EnableEmission(false);
					actor.LeftEngine:EnableEmission(false);
					actor.RightThruster:EnableEmission(false);
					actor.LeftThruster:EnableEmission(false);
					actor.RotAngle = 0;
				elseif IsACRocket(actor) then
					actor.RotAngle = 0;
				end

				local actorIsSlowEnoughToUseAutomovers = self:slowDownFastActorToMovementSpeed(actorData);

				if actorIsSlowEnoughToUseAutomovers then
					if not self.actorUnstickingDisabled then
						if actorData.direction == Directions.None or actorData.movementMode ~= self.movementModes.move or actor.Vel:MagnitudeIsGreaterThan(self.movementSpeed - 1) then
							actorData.unstickTimer:Reset();
						elseif actorData.unstickTimer:IsPastSimTimeLimit() then
							actorData.movementMode = self.movementModes.unstickActor;
						end
					end

					local anyCenteringWasDone = false;
					if actorData.movementMode == self.movementModes.move or actorData.movementMode == self.movementModes.unstickActor then
						anyCenteringWasDone = self:centreActorToClosestNodeIfMovingInAppropriateDirection(actorData);
					end

					if actorData.movementMode == self.movementModes.freeze then
						self:updateFrozenActor(actorData);
					elseif actorData.movementMode == self.movementModes.move then
						self:updateMovingActor(actorData, anyCenteringWasDone);
					end
				end
			end
		end
	end
	tracy.ZoneEnd();
end

automoverUtilityFunctions.handlePieButtons = function(self)
	self.displayingInfoUI = self:NumberValueExists("DisplayInfoUI");

	if self:NumberValueExists("SwapAcceptsAllTeams") then
		self.acceptsAllTeams = not self.acceptsAllTeams;
		self:RemoveNumberValue("SwapAcceptsAllTeams");
	end

	if self:NumberValueExists("SwapAcceptsCrafts") then
		self.acceptsCrafts = not self.acceptsCrafts;
		self:RemoveNumberValue("SwapAcceptsCrafts");
	end

	if self:NumberValueExists("SwapHumansRemainUpright") then
		self.humansRemainUpright = not self.humansRemainUpright;
		self:RemoveNumberValue("SwapHumansRemainUpright");
	end

	if self:NumberValueExists("ModifyMovementSpeed") and self.heldInputTimer:IsPastSimTimeLimit() then
		if self:GetController():IsState(Controller.SCROLL_UP) or self:GetController():IsState(Controller.HOLD_UP) then
			self.movementSpeed = math.min(self.movementSpeed + 1, self.movementSpeedMax);
		elseif self:GetController():IsState(Controller.SCROLL_DOWN) or self:GetController():IsState(Controller.HOLD_DOWN) then
			self.movementSpeed = math.max(self.movementSpeed - 1, self.movementSpeedMin);
		end
		self.heldInputTimer:Reset()
	end

	if self:NumberValueExists("ModifyMassLimit") and self.heldInputTimer:IsPastSimTimeLimit() then
		if self:GetController():IsState(Controller.SCROLL_UP) or self:GetController():IsState(Controller.HOLD_UP) then
			self.massLimit = math.min(self.massLimit + 25, self.massLimitMax);
		elseif self:GetController():IsState(Controller.SCROLL_DOWN) or self:GetController():IsState(Controller.HOLD_DOWN) then
			self.massLimit = math.max(self.massLimit - 25, self.massLimitMin);
		end
		self.heldInputTimer:Reset();
	end

	if self:NumberValueExists("ModifyVisualEffectsType") then
		local visualEffectsTypes = { "" };
		for visualEffectsType, _ in pairs(self.visualEffectsConfig) do
			visualEffectsTypes[#visualEffectsTypes + 1] = visualEffectsType;
		end

		for visualEffectsTypeIndex, visualEffectsType in ipairs(visualEffectsTypes) do
			if self.visualEffectsSelectedType == visualEffectsType then
				self.visualEffectsSelectedType = visualEffectsTypes[((visualEffectsTypeIndex % #visualEffectsTypes) + 1)];
				break;
			end
		end

		self.visualEffectsData = {};
		self:RemoveNumberValue("ModifyVisualEffectsType");
	end

	if self:NumberValueExists("ModifyVisualEffectsSize") then
		self.visualEffectsSelectedSize = (self.visualEffectsSelectedSize % self.visualEffectsSizes.large) + 1;
		self.visualEffectsData = {};
		self:RemoveNumberValue("ModifyVisualEffectsSize");
	end
end

automoverUtilityFunctions.fuzzyPositionMatch = function(self, pos1, pos2, ignoreXMatching, ignoreYMatching, fuzziness, otherDistanceMustBeNegative)
	local distance = SceneMan:ShortestDistance(pos1, pos2, self.checkWrapping);
	local valuesMatch = {X = ignoreXMatching, Y = ignoreYMatching};
	for axis, ignoreMatching in pairs(valuesMatch) do
		if not ignoreMatching then
			if fuzziness >= 0 then
				valuesMatch[axis] = distance[axis] >= 0 and distance[axis] <= fuzziness;
			else
				valuesMatch[axis] = distance[axis] < 0 and distance[axis] >= fuzziness;
			end

			local otherAxis = axis == "X" and "Y" or "X";
			if otherDistanceMustBeNegative then
				valuesMatch[axis] = valuesMatch[axis] and distance[otherAxis] <= 0;
			else
				valuesMatch[axis] = valuesMatch[axis] and distance[otherAxis] >= 0;
			end
		end
	end

	return valuesMatch.X and valuesMatch.Y;
end

automoverUtilityFunctions.updateActivityEditingMode = function(self)
	local teamNodeTable = AutomoverData[self.Team].nodeData;
	local teamTeleporterTable = AutomoverData[self.Team].teleporterNodes;

	local relevantPlayers = {};
	for player = 0, Activity.PLAYER_4 do
		if self.currentActivity:PlayerHuman(player) and self.currentActivity:GetTeamOfPlayer(player) == self.Team then
			relevantPlayers[#relevantPlayers + 1] = player;
		end
	end

	for _, player in ipairs(relevantPlayers) do
		local selectedEditorObject = ToGameActivity(self.currentActivity):GetEditorGUI(player):GetCurrentObject();
		if selectedEditorObject ~= nil and (selectedEditorObject.PresetName:find("Automover Zone") or selectedEditorObject.PresetName:find("Teleporter Zone")) then
			local selectedEditorObjectPos = selectedEditorObject.Pos;

			local directionMatches = {};
			if not selectedEditorObject.PresetName:find("Vertical Only") then
				directionMatches.left = {};
				directionMatches.right = {};
			end
			if not selectedEditorObject.PresetName:find("Horizontal Only") then
				directionMatches.up = {};
				directionMatches.down = {};
			end

			for direction, matches in pairs(directionMatches) do
				local distanceInDirectionMustBeNegative = direction == "up" or direction == "left";
				for node, nodeData in pairs(teamNodeTable) do
					if selectedEditorObject.PresetName:find("Teleporter Zone") == nil or not teamTeleporterTable[node] then
						local ignoreXMatching = direction == "left" or direction == "right" or (node.PresetName:find("Horizontal Only") ~= nil);
						local ignoreYMatching = direction == "up" or direction == "down" or (node.PresetName:find("Vertical Only") ~= nil);
						if not (ignoreXMatching and ignoreYMatching) then
							if self:fuzzyPositionMatch(selectedEditorObjectPos, node.Pos, ignoreXMatching, ignoreYMatching, 0, distanceInDirectionMustBeNegative) then
								local distance = SceneMan:ShortestDistance(selectedEditorObjectPos, node.Pos, self.checkWrapping);
								if not matches.exact or distance.SqrMagnitude < matches.exact.SqrMagnitude then
									matches.exact = distance;
								end
							elseif self:fuzzyPositionMatch(selectedEditorObjectPos, node.Pos, ignoreXMatching, ignoreYMatching, 12, distanceInDirectionMustBeNegative) then
								local distance = SceneMan:ShortestDistance(selectedEditorObjectPos, node.Pos, self.checkWrapping);
								if not matches.closestFuzzyPositive or distance.SqrMagnitude < matches.closestFuzzyPositive.SqrMagnitude then
									matches.closestFuzzyPositive = distance;
								end
							elseif self:fuzzyPositionMatch(selectedEditorObjectPos, node.Pos, ignoreXMatching, ignoreYMatching, -12, distanceInDirectionMustBeNegative) then
								local distance = SceneMan:ShortestDistance(selectedEditorObjectPos, node.Pos, self.checkWrapping);
								if not matches.closestFuzzyNegative or distance.SqrMagnitude < matches.closestFuzzyNegative.SqrMagnitude then
									matches.closestFuzzyNegative = distance;
								end
							end
						end
					end
				end

				for matchType, distanceToMatch in pairs(matches) do
					if SceneMan:CastStrengthRay(selectedEditorObjectPos, distanceToMatch, 15, Vector(), 4, 0, true) then
						PrimitiveMan:DrawLinePrimitive(selectedEditorObjectPos, selectedEditorObjectPos + distanceToMatch, matchType == "exact" and 12 or 8);
					else
						PrimitiveMan:DrawLinePrimitive(selectedEditorObjectPos, selectedEditorObjectPos + distanceToMatch, matchType == "exact" and 147 or 159);
					end
				end
			end
		end
	end
end

automoverUtilityFunctions.checkForObstructions = function(self)
	tracy.ZoneBegin();
	local _, connectionChangesFound = coroutine.resume(self.obstructionCheckCoroutine, self);
	local coroutineIsDead = coroutine.status(self.obstructionCheckCoroutine) == "dead";
	if connectionChangesFound then
		coroutineIsDead = true;
		self.allBoxesAdded = false;
		self.allPathsAdded = false;
	end

	if coroutineIsDead then
		self.obstructionCheckCoroutine = coroutine.create(self.checkAllObstructions);
		self.obstructionCheckTimer:SetSimTimeLimitMS(10000);
	else
		self.obstructionCheckTimer:SetSimTimeLimitMS(100);
	end
	tracy.ZoneEnd();
end

automoverUtilityFunctions.checkAllObstructions = function(self)
	local teamNodeTable = AutomoverData[self.Team].nodeData;

	local nodeConnectionsHaveChanged = false;
	local checkedNodeCount = 0;

	local oppositeDirections = {
		[Directions.Up] = Directions.Down,
		[Directions.Down] = Directions.Up,
		[Directions.Left] = Directions.Right,
		[Directions.Right] = Directions.Left,
	};

	for node, nodeData in pairs(teamNodeTable) do
		local previousConnectedNodeData = nodeData.connectedNodeData;
		nodeData.connectedNodeData = {};

		local nodesAffectedByThisAutomover = Automovers_CheckConnections(node);
		if nodesAffectedByThisAutomover ~= nil then
			for direction, affectedNode in pairs(nodesAffectedByThisAutomover) do
				local affectedNodeTable = teamNodeTable[affectedNode];
				affectedNodeTable.connectedNodeData[oppositeDirections[direction]] = nil;
				Automovers_CheckConnections(affectedNode);
			end
		end

		for direction, _ in pairs(oppositeDirections) do
			if type(previousConnectedNodeData[direction]) ~= type(nodeData.connectedNodeData[direction]) then
				nodeConnectionsHaveChanged = true;
				self.visualEffectsData[node] = {};
				break;
			end
		end


		checkedNodeCount = checkedNodeCount + 1;
		if checkedNodeCount % 5 == 0 then
			coroutine.yield(nodeConnectionsHaveChanged);
		end
	end
	return nodeConnectionsHaveChanged;
end

automoverUtilityFunctions.addAllBoxesAndPaths = function(self)
	tracy.ZoneBegin();
	if not self.allBoxesAdded and coroutine.status(self.addAllBoxesCoroutine) == "dead" then
		self.addAllBoxesCoroutine = coroutine.create(self.addAllBoxes);
	end

	coroutine.resume(self.addAllBoxesCoroutine, self);
	self.allBoxesAdded = self.allBoxesAdded or coroutine.status(self.addAllBoxesCoroutine) == "dead";

	if not self.allPathsAdded and coroutine.status(self.addAllPathsCoroutine) == "dead" then
		self.addAllPathsCoroutine = coroutine.create(self.addAllPaths);
	end

	coroutine.resume(self.addAllPathsCoroutine, self);

	self.allPathsAdded = self.allPathsAdded or coroutine.status(self.addAllPathsCoroutine) == "dead";
	tracy.ZoneEnd();
end

automoverUtilityFunctions.addAllBoxes = function(self)
	local teamNodeTable = AutomoverData[self.Team].nodeData;

	local addedNodeCount = 0;
	for node, nodeData in pairs(teamNodeTable) do
		if nodeData.zoneBox ~= nil then
			self.combinedAutomoverArea:AddBox(nodeData.zoneBox);
			SceneMan.Scene:GetArea("NoGravityArea"):AddBox(nodeData.zoneBox);
		else
			print("Automover Error: Automover at position " .. tostring(node.Pos) .. " had no ZoneBox.");
		end

		for _, direction in pairs({Directions.Up, Directions.Left}) do
			if nodeData.connectedNodeData[direction] then
				local connectedNode = nodeData.connectedNodeData[direction].node;
				local connectedNodeData = teamNodeTable[connectedNode];

				local connectingBoxTopLeftCornerOffset;
				local connectingBoxBottomRightCornerOffset;
				if direction == Directions.Up then
					local smallerNodeZoneHalfWidth = math.min(nodeData.size.X, connectedNodeData.size.X) * 0.5;
					connectingBoxTopLeftCornerOffset = Vector(-smallerNodeZoneHalfWidth, connectedNodeData.size.Y * 0.5);
					connectingBoxBottomRightCornerOffset = Vector(smallerNodeZoneHalfWidth, -nodeData.size.Y * 0.5);
				else
					local smallerNodeZoneHalfHeight = math.min(nodeData.size.Y, connectedNodeData.size.Y) * 0.5;
					connectingBoxTopLeftCornerOffset = Vector(connectedNodeData.size.X * 0.5, -smallerNodeZoneHalfHeight);
					connectingBoxBottomRightCornerOffset = Vector(-nodeData.size.X * 0.5, smallerNodeZoneHalfHeight);
				end

				nodeData.connectingAreas[direction] = Area();
				--TODO Area::IsInside wraps things when you check it, so maybe can just store the box, if that's all we're using this for
				for wrappedConnectingBox in SceneMan:WrapBox(Box(connectedNode.Pos + connectingBoxTopLeftCornerOffset, node.Pos + connectingBoxBottomRightCornerOffset)) do
					nodeData.connectingAreas[direction]:AddBox(wrappedConnectingBox);
					self.combinedAutomoverArea:AddBox(wrappedConnectingBox);
					SceneMan.Scene:GetArea("NoGravityArea"):AddBox(wrappedConnectingBox);
				end
			end
		end

		addedNodeCount = addedNodeCount + 1;
		if addedNodeCount % 10 == 0 and coroutine.running() then
			coroutine.yield();
		end
	end

	for node, nodeData in pairs(teamNodeTable) do
		for _, direction in pairs({Directions.Down, Directions.Right}) do
			if nodeData.connectedNodeData[direction] then
				nodeData.connectingAreas[direction] = teamNodeTable[nodeData.connectedNodeData[direction].node].connectingAreas[(direction == Directions.Down and Directions.Up or Directions.Left)];
			end
		end
	end

	return true;
end

automoverUtilityFunctions.addAllPaths = function(self)
	local teamNodeTable = AutomoverData[self.Team].nodeData;
	local teamTeleporterTable = AutomoverData[self.Team].teleporterNodes;

	local possibleConnectionDirections = {Directions.Up, Directions.Down, Directions.Left, Directions.Right};

	for node, nodeData in pairs(teamNodeTable) do
		local tentativeNodes = {};
		local confirmedNodes = {};
		confirmedNodes[node] = { distance = 0, direction = Directions.None };
		local addedPathCount = 0;

		local connectedNodeData = nodeData.connectedNodeData;
		for _, direction in ipairs(possibleConnectionDirections) do
			if connectedNodeData[direction] ~= nil and confirmedNodes[connectedNodeData[direction].node] == nil then
				tentativeNodes[connectedNodeData[direction].node] = { distance = confirmedNodes[node].distance + connectedNodeData[direction].distance.Magnitude, direction = direction };
			end
		end

		if teamTeleporterTable[node] then
			for teleporter, _ in pairs(teamTeleporterTable) do
				if teleporter.UniqueID ~= node.UniqueID then
					tentativeNodes[teleporter] = { distance = 48, direction = Directions.Any }; -- Teleporter extra distance is set as the smallest distance between two nodes, and its direction is set to Any to distinguish it.
				end
			end
		end

		while next(tentativeNodes) ~= nil do
			local closestNode;
			local distanceToClosestNode;
			for tentativeNode, tentativeNodeData in pairs(tentativeNodes) do
				if distanceToClosestNode == nil or tentativeNodeData.distance < distanceToClosestNode then
					closestNode = tentativeNode;
					distanceToClosestNode = tentativeNodeData.distance;
				end
			end

			if closestNode == nil then
				break;
			end

			confirmedNodes[closestNode] = { distance = distanceToClosestNode, direction = tentativeNodes[closestNode].direction };
			tentativeNodes[closestNode] = nil;

			local closestNodeConnectedNodeData = teamNodeTable[closestNode].connectedNodeData;
			for _, direction in ipairs(possibleConnectionDirections) do
				if closestNodeConnectedNodeData[direction] ~= nil and confirmedNodes[closestNodeConnectedNodeData[direction].node] == nil then
					local nodeInDirection = closestNodeConnectedNodeData[direction].node;
					if tentativeNodes[nodeInDirection] == nil or tentativeNodes[nodeInDirection].distance > (closestNodeConnectedNodeData[direction].distance.Magnitude + confirmedNodes[closestNode].distance) then
						tentativeNodes[nodeInDirection] = { distance = closestNodeConnectedNodeData[direction].distance.Magnitude + confirmedNodes[closestNode].distance, direction = confirmedNodes[closestNode].direction };
					end
				end
			end

			if teamTeleporterTable[closestNode] then
				for teleporter, _ in pairs(teamTeleporterTable) do
					if teleporter.UniqueID ~= closestNode.UniqueID and confirmedNodes[teleporter] == nil then
						tentativeNodes[teleporter] = { distance = confirmedNodes[closestNode].distance + 48, direction = confirmedNodes[closestNode].direction }; -- Teleporter extra distance is set as the smallest distance between two nodes, and its direction is set to Any to distinguish it.
					end
				end
			end

			addedPathCount = addedPathCount + 1;
			if addedPathCount % 10 == 0 and coroutine.running() then
				coroutine.yield();
			end
		end

		self.pathTable[node] = confirmedNodes;
		if coroutine.running() then
			coroutine.yield();
		end
	end

	return true;
end

automoverUtilityFunctions.findClosestNode = function(self, positionToFindClosestNodeFor, nodeToCheckForPathsFrom, checkForLineOfSight, checkThatPositionIsInsideNodeZoneBoxOrConnectingAreas)
	tracy.ZoneBegin();
	local teamNodeTable = AutomoverData[self.Team].nodeData;

	local nodesToCheck = teamNodeTable;
	if nodeToCheckForPathsFrom ~= nil then
		nodesToCheck = self.pathTable[nodeToCheckForPathsFrom];
	end

	local closestNode;
	local distanceToClosestNodeSqr;
	for node, _ in pairs(nodesToCheck) do
		local nodeData = teamNodeTable[node];
		local distanceToNode = SceneMan:ShortestDistance(node.Pos, positionToFindClosestNodeFor, self.checkWrapping);
		if distanceToClosestNodeSqr == nil or distanceToNode.SqrMagnitude < distanceToClosestNodeSqr then
			local nodeSatisfiesConditions = true;
			if checkForLineOfSight then
				nodeSatisfiesConditions = not SceneMan:CastStrengthRay(node.Pos, distanceToNode, 15, Vector(), 4, 0, true);
			end
			if nodeSatisfiesConditions and checkThatPositionIsInsideNodeZoneBoxOrConnectingAreas then
				nodeSatisfiesConditions = nodeData.zoneBox:IsWithinBox(positionToFindClosestNodeFor);
				if not nodeSatisfiesConditions then
					local connectingAreaDirectionToCheck = Directions.None;
					if distanceToNode.Y + (nodeData.size.Y * 0.5) < 0 then
						connectingAreaDirectionToCheck = Directions.Up;
					elseif distanceToNode.Y - (nodeData.size.Y * 0.5) > 0 then
						connectingAreaDirectionToCheck = Directions.Down;
					elseif distanceToNode.X + (nodeData.size.X * 0.5) < 0 then
						connectingAreaDirectionToCheck = Directions.Left;
					elseif distanceToNode.X - (nodeData.size.X * 0.5) > 0 then
						connectingAreaDirectionToCheck = Directions.Right;
					end
					if connectingAreaDirectionToCheck ~= Directions.None and nodeData.connectingAreas[connectingAreaDirectionToCheck] ~= nil then
						nodeSatisfiesConditions = nodeData.connectingAreas[connectingAreaDirectionToCheck]:IsInside(positionToFindClosestNodeFor);
					end
				end
			end
			if nodeSatisfiesConditions then
				closestNode = node;
				distanceToClosestNodeSqr = distanceToNode.SqrMagnitude;
			end
		end
	end

	tracy.ZoneEnd();
	return closestNode;
end

automoverUtilityFunctions.findNodeWithShortestScenePath = function(self, positionToFindClosestNodeFor, nodeToCheckForPathsFrom, checkThatPositionIsInsideNodeZoneBoxOrConnectingAreas, pathfinderTeam, pathfinderDigStrength)
	local teamNodeTable = AutomoverData[self.Team].nodeData;
	local teamTeleporterTable = AutomoverData[self.Team].teleporterNodes;

	local potentialClosestNodes = {}
	for node, nodeData in pairs(teamNodeTable) do
		local nodeSatisfiesConditions = nodeToCheckForPathsFrom == nil or (self.pathTable[nodeToCheckForPathsFrom] ~= nil and self.pathTable[nodeToCheckForPathsFrom][node] ~= nil);

		if nodeSatisfiesConditions and checkThatPositionIsInsideNodeZoneBoxOrConnectingAreas then
			nodeSatisfiesConditions = nodeData.zoneBox:IsWithinBox(positionToFindClosestNodeFor);
			if not nodeSatisfiesConditions then
				local connectingAreaDirectionToCheck = Directions.None;
				local distanceToNode = SceneMan:ShortestDistance(node.Pos, positionToFindClosestNodeFor, self.checkWrapping);
				if distanceToNode.Y + (nodeData.size.Y * 0.5) < 0 then
					connectingAreaDirectionToCheck = Directions.Up;
				elseif distanceToNode.Y - (nodeData.size.Y * 0.5) > 0 then
					connectingAreaDirectionToCheck = Directions.Down;
				elseif distanceToNode.X + (nodeData.size.X * 0.5) < 0 then
					connectingAreaDirectionToCheck = Directions.Left;
				elseif distanceToNode.X - (nodeData.size.X * 0.5) > 0 then
					connectingAreaDirectionToCheck = Directions.Right;
				end
				if connectingAreaDirectionToCheck ~= Directions.None and nodeData.connectingAreas[connectingAreaDirectionToCheck] ~= nil then
					nodeSatisfiesConditions = nodeData.connectingAreas[connectingAreaDirectionToCheck]:IsInside(positionToFindClosestNodeFor);
				end
			end
		end

		if nodeSatisfiesConditions then
			potentialClosestNodes[node] = node.Pos;
		end
	end

	local shortestPathCoroutine = coroutine.create(FindStartPositionWithShortestPathToEndPosition);
	while coroutine.status(shortestPathCoroutine) ~= "dead" do
		local _, result = coroutine.resume(shortestPathCoroutine, potentialClosestNodes, positionToFindClosestNodeFor, pathfinderTeam, false, pathfinderDigStrength);
		if result then
			return result;
		else
			coroutine.yield();
		end
	end
end

automoverUtilityFunctions.changeScaleOfMOSRotatingAndAttachables = function(self, mosRotatingToChangeScaleOf, scale)
	mosRotatingToChangeScaleOf.Scale = scale;
	for attachable in mosRotatingToChangeScaleOf.Attachables do
		self:changeScaleOfMOSRotatingAndAttachables(attachable, scale);
	end
end

automoverVisualEffectsFunctions.updateVisualEffects = function(self)
	tracy.ZoneBegin();
	if self.visualEffectsSelectedType == "" then
		tracy.ZoneEnd();
		return;
	end

	local selectedVisualEffects = self.visualEffectsConfig[self.visualEffectsSelectedType][self.visualEffectsSelectedSize];

	if selectedVisualEffects.moveTimer:IsPastSimTimeLimit() then
		for node, nodeData in pairs(AutomoverData[self.Team].nodeData) do
			for _, direction in ipairs({Directions.Up, Directions.Left}) do
				self:setupNodeVisualEffectsForDirectionIfAppropriate(node, nodeData, direction);
			end
		end
		self:updateVisualEffectsFrames();
		selectedVisualEffects.moveTimer:Reset();
	end

	self:drawVisualEffects();
	tracy.ZoneEnd();
end

automoverVisualEffectsFunctions.setupNodeVisualEffectsForDirectionIfAppropriate = function(self, node, nodeData, direction)
	local selectedVisualEffects = self.visualEffectsConfig[self.visualEffectsSelectedType][self.visualEffectsSelectedSize];

	local nodeConnectionDataInDirection = nodeData.connectedNodeData[direction];
	if nodeConnectionDataInDirection == nil then
		return;
	end

	if self.visualEffectsData[node] ~= nil and self.visualEffectsData[node][direction] ~= nil and self.visualEffectsData[node][direction].endNode.UniqueID == nodeConnectionDataInDirection.node.UniqueID then
		return;
	end

	local relevantAxis = direction == Directions.Up and "Y" or "X";
	local nodeInDirectionData = AutomoverData[self.Team].nodeData[nodeConnectionDataInDirection.node];
	local minDistanceToShowEffects = (nodeData.size[relevantAxis] * 0.5) + (nodeInDirectionData.size[relevantAxis] * 0.5) + (selectedVisualEffects.spriteSize * self.visualEffectsMinimumNumberOfSpacesNeededForDrawingObjectEffects);
	if math.abs(nodeConnectionDataInDirection.distance[relevantAxis]) < minDistanceToShowEffects then
		return;
	end

	if self.visualEffectsData[node] == nil then
		self.visualEffectsData[node] = {};
	end

	self.visualEffectsData[node][direction] = {
		coolDownTimer = Timer(selectedVisualEffects.coolDownInterval),
		isGoingBackwards = false,
		endNode = nodeConnectionDataInDirection.node,
		rotAngle = direction == Directions.Up and math.rad(90) or math.rad(180),
		drawData = {},
	};

	local numberOfSpritesToDraw, halfRemainderDistance = math.modf((math.abs(nodeConnectionDataInDirection.distance[relevantAxis]) - math.abs(nodeInDirectionData.size[relevantAxis])) / selectedVisualEffects.spriteSize);
	halfRemainderDistance = halfRemainderDistance * selectedVisualEffects.spriteSize * 0.5;

	local firstSpritePos = node.Pos + Vector();
	firstSpritePos[relevantAxis] = firstSpritePos[relevantAxis] - (nodeData.size[relevantAxis] * 0.5) - (selectedVisualEffects.spriteSize * 0.5) - halfRemainderDistance;
	for i = 1, numberOfSpritesToDraw do
		local spriteIndexOffset = Vector();
		spriteIndexOffset[relevantAxis] = -(selectedVisualEffects.spriteSize * (i - 1));
		self.visualEffectsData[node][direction].drawData[i] = {
			position = firstSpritePos + spriteIndexOffset,
			frame = selectedVisualEffects.minFrame - 1,
		};
	end
end

automoverVisualEffectsFunctions.updateVisualEffectsFrames = function(self)
	local selectedVisualEffects = self.visualEffectsConfig[self.visualEffectsSelectedType][self.visualEffectsSelectedSize];

	for node, visualEffectsData in pairs(self.visualEffectsData) do
		for direction, visualEffectsDataInDirection in pairs(visualEffectsData) do
			if #visualEffectsDataInDirection.drawData > 0 and visualEffectsDataInDirection.coolDownTimer:IsPastSimTimeLimit() then
				for index, drawData in ipairs(visualEffectsDataInDirection.drawData) do
					local previousIndex = visualEffectsDataInDirection.isGoingBackwards and index + 1 or index - 1;
					local minPreviousIndexFrame = visualEffectsDataInDirection.isGoingBackwards and selectedVisualEffects.minFrame or selectedVisualEffects.minFrame + 1;
					if drawData.frame <= selectedVisualEffects.maxFrame and (previousIndex < 1 or previousIndex > #visualEffectsDataInDirection.drawData or visualEffectsDataInDirection.drawData[previousIndex].frame >= minPreviousIndexFrame) then
						drawData.frame = drawData.frame + 1;
					end
				end

				local endIndex = visualEffectsDataInDirection.isGoingBackwards and 1 or #visualEffectsDataInDirection.drawData;
				if visualEffectsDataInDirection.drawData[endIndex].frame > selectedVisualEffects.maxFrame then
					visualEffectsDataInDirection.isGoingBackwards = not visualEffectsDataInDirection.isGoingBackwards;
					visualEffectsDataInDirection.rotAngle = NormalizeAngleBetween0And2PI(visualEffectsDataInDirection.rotAngle + math.pi);
					for _, drawData in pairs(visualEffectsDataInDirection.drawData) do
						drawData.frame = selectedVisualEffects.minFrame - 1;
					end
					visualEffectsDataInDirection.coolDownTimer:Reset();
				end
			end
		end
	end
end

automoverVisualEffectsFunctions.drawVisualEffects = function(self)
	local selectedVisualEffects = self.visualEffectsConfig[self.visualEffectsSelectedType][self.visualEffectsSelectedSize];

	for node, visualEffectsData in pairs(self.visualEffectsData) do
		for direction, visualEffectsDataInDirection in pairs(visualEffectsData) do
			for _, drawData in ipairs(visualEffectsDataInDirection.drawData) do
				if drawData.frame >= selectedVisualEffects.minFrame and drawData.frame < selectedVisualEffects.maxFrame then
					PrimitiveMan:DrawBitmapPrimitive(drawData.position, selectedVisualEffects.spriteObject, visualEffectsDataInDirection.rotAngle, drawData.frame);
				end
			end
		end
	end
end

automoverActorFunctions.checkForNewActors = function(self)
	tracy.ZoneBegin();
	for box in self.combinedAutomoverArea.Boxes do
		for movableObject in MovableMan:GetMOsInBox(box, -1, true) do
			if IsActor(movableObject) and self.affectedActors[movableObject.UniqueID] == nil and movableObject.PinStrength == 0 then
				local actor = ToActor(movableObject);
				if actor.AIMode ~= Actor.AIMODE_GOLDDIG and actor.AIMode ~= Actor.AIMODE_PATROL then
					local teamAccepted = self.acceptsAllTeams or movableObject.Team == self.Team;
					local massAccepted = movableObject.Mass < self.massLimit;
					local typeAccepted = self.acceptsCrafts or (not IsACRocket(movableObject) and not IsACDropShip(movableObject));
					if teamAccepted and massAccepted and typeAccepted then
						self:addActorToAutomoverTable(actor);
					end
				end
			end
		end
	end

	tracy.ZoneEnd();
end

automoverActorFunctions.addActorToAutomoverTable = function(self, actor)
	self.affectedActors[actor.UniqueID] = {
		actor = actor,
		movementMode = self.movementModes.freeze,
		direction = Directions.None,
		unstickTimer = Timer(2000),
		currentClosestNode = self:findClosestNode(actor.Pos, nil, false, false),
		waypointData = nil,
	};

	if not actor:NumberValueExists("Automover_OldMoveProximityLimit") then
		actor:SetNumberValue("Automover_OldMoveProximityLimit", actor.MoveProximityLimit);
	end

	-- Make move proximity much lower so we don't get caught on corners
	actor.MoveProximityLimit = 2;

	self.affectedActorsCount = self.affectedActorsCount + 1;

	actor.PieMenu:AddPieSliceIfPresetNameIsUnique(self.leaveAutomoverNetworkPieSlice, self);
	if IsAHuman(actor) then
		ToAHuman(actor).LimbPushForcesAndCollisionsDisabled = true;
	end
end

automoverActorFunctions.removeActorFromAutomoverTable = function(self, actor, optionalActorUniqueID)
	if actor.UniqueID ~= 0 and optionalActorUniqueID == nil then
		optionalActorUniqueID = actor.UniqueID;
	end

	if MovableMan:ValidMO(actor) then
		if actor:NumberValueExists("Automover_OldMoveProximityLimit") then
			actor.MoveProximityLimit = actor:GetNumberValue("Automover_OldMoveProximityLimit");
			actor:RemoveNumberValue("Automover_OldMoveProximityLimit");
		end

		actor.PieMenu:RemovePieSlicesByPresetName(self.leaveAutomoverNetworkPieSlice.PresetName);
		actor.PieMenu:RemovePieSlicesByPresetName(self.chooseTeleporterPieSlice.PresetName);

		if IsAHuman(actor) then
			ToAHuman(actor).LimbPushForcesAndCollisionsDisabled = false;
		end

		if self.affectedActors[optionalActorUniqueID] ~= nil then
			self:convertWaypointDataToActorWaypoints(self.affectedActors[optionalActorUniqueID]);
		end
	end

	self.affectedActors[optionalActorUniqueID] = nil;
	self.affectedActorsCount = self.affectedActorsCount - 1;
end

automoverActorFunctions.setActorMovementModeToLeaveAutomovers = function(self, actorData)
	local actor = actorData.actor;

	actor.PieMenu:RemovePieSlicesByPresetName(self.leaveAutomoverNetworkPieSlice.PresetName);
	actor.PieMenu:RemovePieSlicesByPresetName(self.chooseTeleporterPieSlice.PresetName);
	self:changeScaleOfMOSRotatingAndAttachables(actor, 1);
	if IsAHuman(actor) then
		ToAHuman(actor).LimbPushForcesAndCollisionsDisabled = false;
	end
	self:convertWaypointDataToActorWaypoints(actorData);

	actorData.movementMode = self.movementModes.leaveAutomovers;
end

automoverActorFunctions.convertActorWaypointsToWaypointData = function(self, actorData)
	if actorData.movementMode ~= self.movementModes.teleporting then
		return;
	end
	
	local actor = actorData.actor;
	actorData.waypointData = {};
	local waypointData = actorData.waypointData;

	if actor.MOMoveTarget then
		waypointData.movableObjectTarget = actor.MOMoveTarget;
	else
		waypointData.sceneTargets = {}
		if actor.MovePathEnd ~= actor.Pos then
			waypointData.sceneTargets[#waypointData.sceneTargets + 1] = Vector(actor.MovePathEnd.X, actor.MovePathEnd.Y);
		end
		for actorSceneWaypoint in actor.SceneWaypoints do
			waypointData.sceneTargets[#waypointData.sceneTargets + 1] = Vector(actorSceneWaypoint.X, actorSceneWaypoint.Y);
		end
	end

	waypointData.previousAIMode = actor.AIMode;
	actor.AIMode = Actor.AIMODE_SENTRY;
	actor:ClearAIWaypoints();
end

automoverActorFunctions.convertWaypointDataToActorWaypoints = function(self, actorData)
	local actor = actorData.actor;
	local waypointData = actorData.waypointData;

	if waypointData then
		actor.AIMode = waypointData.previousAIMode;
		if waypointData.movableObjectTarget then
			actor:AddAIMOWaypoint(waypointData.movableObjectTarget);
		elseif waypointData.sceneTargets then
			for _, sceneTarget in ipairs(waypointData.sceneTargets) do
				actor:AddAISceneWaypoint(sceneTarget);
			end
		end

		actorData.waypointData = nil;
	end
end

automoverActorFunctions.setupManualTeleporterData = function(self, actorData)
	local teamTeleporterTable = AutomoverData[self.Team].teleporterNodes;

	local actor = actorData.actor;

	actorData.waypointData = nil;
	actorData.manualTeleporterData = {};
	manualTeleporterData = actorData.manualTeleporterData;

	manualTeleporterData.actorTeleportationStage = 0;
	manualTeleporterData.teleporterVisualsTimer = Timer(1000);

	local startingTeleporter = self:findClosestNode(actor.Pos, nil, false, false);
	manualTeleporterData.sortedTeleporters = {{ node = startingTeleporter, distance = 0 }};

	for teleporterNode, _ in pairs(teamTeleporterTable) do
		if teleporterNode.UniqueID ~= startingTeleporter.UniqueID then
			local xDistanceToTeleporter = SceneMan:ShortestDistance(startingTeleporter.Pos, teleporterNode.Pos, self.checkWrapping).X;

			local teleporterNodeAddedToSortedTable = false;
			for index, sortedTeleporterData in ipairs(manualTeleporterData.sortedTeleporters) do
				if xDistanceToTeleporter <= sortedTeleporterData.distance then
					table.insert(manualTeleporterData.sortedTeleporters, index, { node = teleporterNode, distance = xDistanceToTeleporter });
					teleporterNodeAddedToSortedTable = true;
					break;
				end
			end

			if not teleporterNodeAddedToSortedTable then
				table.insert(manualTeleporterData.sortedTeleporters, #manualTeleporterData.sortedTeleporters + 1, { node = teleporterNode, distance = xDistanceToTeleporter });
			end
		end
	end

	for index, sortedTeleporterData in pairs(manualTeleporterData.sortedTeleporters) do
		if sortedTeleporterData.node.UniqueID == startingTeleporter.UniqueID then
			manualTeleporterData.currentChosenTeleporter = index;
		end
	end

	self.heldInputTimer:Reset(); -- Note: Because manual teleporting is done with Fire and Pie Menu, we need to reset this timer when we set things up, so we don't instantly trigger teleporting or canceling.
end

automoverActorFunctions.chooseTeleporterForPlayerControlledActor = function(self, actorData)
	local actor = actorData.actor;
	local actorController = actor:GetController();
	local manualTeleporterData = actorData.manualTeleporterData;

	if manualTeleporterData.actorTeleportationStage == 0 then
		if actorController:IsState(Controller.PRESS_LEFT) or (actorController:IsState(Controller.HOLD_LEFT) and self.heldInputTimer:IsPastSimMS(250)) then
			manualTeleporterData.currentChosenTeleporter = manualTeleporterData.currentChosenTeleporter - 1;
			if manualTeleporterData.currentChosenTeleporter <= 0 then
				manualTeleporterData.currentChosenTeleporter = #manualTeleporterData.sortedTeleporters;
			end

			self.heldInputTimer:Reset();
		elseif actorController:IsState(Controller.PRESS_RIGHT) or (actorController:IsState(Controller.HOLD_RIGHT) and self.heldInputTimer:IsPastSimMS(250)) then
			manualTeleporterData.currentChosenTeleporter = manualTeleporterData.currentChosenTeleporter + 1;
			if manualTeleporterData.currentChosenTeleporter > #manualTeleporterData.sortedTeleporters then
				manualTeleporterData.currentChosenTeleporter = 1;
			end

			self.heldInputTimer:Reset();
		elseif actorController:IsState(Controller.PRESS_PRIMARY) and self.heldInputTimer:IsPastSimTimeLimit() then
			manualTeleporterData.actorTeleportationStage = 1;
			manualTeleporterData.teleporterVisualsTimer:Reset();
		elseif actorController:IsState(Controller.PRESS_SECONDARY) and self.heldInputTimer:IsPastSimTimeLimit() then
			actorData.manualTeleporterData = nil;
			return;
		end

		local player = actorController.Player;
		CameraMan:SetScrollTarget(manualTeleporterData.sortedTeleporters[manualTeleporterData.currentChosenTeleporter].node.Pos, 1, player);
		FrameMan:ClearScreenText(player);
		FrameMan:SetScreenText("CHOOSING TELEPORTER: Move Left or Right to change teleporter. Press Fire to teleport. Open the Pie Menu to cancel.", player, 0, 100, false);
	else
		self:changeScaleOfMOSRotatingAndAttachables(actor, (manualTeleporterData.actorTeleportationStage == 2 and manualTeleporterData.teleporterVisualsTimer.SimTimeLimitProgress or 1 - manualTeleporterData.teleporterVisualsTimer.SimTimeLimitProgress));
		self:centreActorToClosestNodeIfMovingInAppropriateDirection(actorData, true);
		if manualTeleporterData.teleporterVisualsTimer.SimTimeLimitProgress >= 1 then
			if manualTeleporterData.actorTeleportationStage == 1 then
				actor.Pos = manualTeleporterData.sortedTeleporters[manualTeleporterData.currentChosenTeleporter].node.Pos;
				manualTeleporterData.actorTeleportationStage = manualTeleporterData.actorTeleportationStage + 1;
				manualTeleporterData.teleporterVisualsTimer:Reset();
			else
				actorData.manualTeleporterData = nil;
				return;
			end
		end
	end

	actor:FlashWhite(100);
	self:updateFrozenActor(actorData);
	return;
end

automoverActorFunctions.updateDirectionsFromActorControllerInput = function(self, actorData)
	tracy.ZoneBegin();
	local actor = actorData.actor;
	local actorController = actor:GetController();

	actorData.direction = Directions.None;

	local analogMove = actorController.AnalogMove;

	if not actor:IsPlayerControlled() and actor.MovePathSize > 0 then
		-- Just get the direction to the next waypoint
		local wptPos;

		-- ugh
		for pos in actor.MovePath do
			wptPos = pos;
			break;
		end

		analogMove = wptPos - actor.Pos;

		-- the ai only removes points if it's not flying and moving, so let's remove the point if needed
		if analogMove:MagnitudeIsLessThan(3) then
			actor:RemoveMovePathBeginning();
		end

		if (actor.Pos - actor.PrevPos):MagnitudeIsLessThan(0.05) then
			-- choose a random direction to get unstuck
			-- TODO, it'd be better if the AI logic can communicate this to us instead!
			analogMove:RadRotate(RangeRand(-math.pi,math.pi));
		end
	end

	local deadZone = 0.1;
	if analogMove:MagnitudeIsGreaterThan(deadZone) then
		if math.abs(analogMove.X) < math.abs(analogMove.Y) then
			actorData.direction = analogMove.Y > 0 and Directions.Down or Directions.Up;
		else
			actorData.direction = analogMove.X > 0 and Directions.Right or Directions.Left;
		end
	else
		if not actorController:IsState(Controller.PIE_MENU_ACTIVE) then
			if actorController:IsState(Controller.PRESS_UP) or actorController:IsState(Controller.HOLD_UP) then
				actorData.direction = Directions.Up;
			elseif actorController:IsState(Controller.PRESS_DOWN) or actorController:IsState(Controller.HOLD_DOWN) then
				actorData.direction = Directions.Down;
			elseif actorController:IsState(Controller.PRESS_LEFT) or actorController:IsState(Controller.HOLD_LEFT) then
				actorData.direction = Directions.Left;
			elseif actorController:IsState(Controller.PRESS_RIGHT) or actorController:IsState(Controller.HOLD_RIGHT) then
				actorData.direction = Directions.Right;
			end
		end
	end

	if actorData.movementMode ~= self.movementModes.unstickActor then
		if actorData.direction == Directions.None and actorData.movementMode ~= self.movementModes.leaveAutomovers then
			actorData.movementMode = self.movementModes.freeze;
		elseif actorData.direction ~= Directions.None then
			actor.PieMenu:AddPieSliceIfPresetNameIsUnique(self.leaveAutomoverNetworkPieSlice, self);
			actorData.movementMode = self.movementModes.move;
			self:convertWaypointDataToActorWaypoints(actorData);
		end
	end

	tracy.ZoneEnd();
end

automoverActorFunctions.updateDirectionsFromWaypoints = function(self, actorData)
	local teamNodeTable = AutomoverData[self.Team].nodeData;

	local actor = actorData.actor;

	if actorData.waypointData == nil and (actor.AIMode == Actor.AIMODE_GOTO or actor.AIMode == Actor.AIMODE_BRAINHUNT or actor.AIMode == Actor.AIMODE_SQUAD) and (actor.MovePathSize > 0 or actor:GetWaypointListSize() > 0) then
		self:convertActorWaypointsToWaypointData(actorData);
	end

	local waypointData = actorData.waypointData;
	if waypointData ~= nil then
		if (waypointData.previousNode == nil or waypointData.nextNode == nil or waypointData.endNode == nil) and waypointData.setupActorWaypointDataCoroutine == nil then
			actorData.movementMode = self.movementModes.freeze;
			waypointData.setupActorWaypointDataCoroutine = coroutine.create(self.setupActorWaypointData);
		elseif waypointData.setupActorWaypointDataCoroutine ~= nil and coroutine.status(waypointData.setupActorWaypointDataCoroutine) ~= "dead" then
			coroutine.resume(waypointData.setupActorWaypointDataCoroutine, self, actorData);
		else
			waypointData.setupActorWaypointDataCoroutine = nil;
			if actorData.movementMode == self.movementModes.freeze then
				actorData.movementMode = self.movementModes.move;
			end
			if actorData.movementMode == self.movementModes.teleporting then
				self:handleTeleportingActorToAppropriateTeleporterForWaypoint(actorData);
			elseif waypointData.previousNode.UniqueID ~= waypointData.endNode.UniqueID and not actorData.waypointData.targetIsBetweenPreviousAndNextNode then
				if teamNodeTable[waypointData.nextNode].zoneInternalBox:IsWithinBox(actor.Pos) then
					self:handleActorInNextNodeZoneInternalBox(actorData);
				end
			else
				self:handleActorThatHasReachedItsEndNode(actorData);
			end
		end
	end
end

automoverActorFunctions.setupActorWaypointData = function(self, actorData)
	local teamNodeTable = AutomoverData[self.Team].nodeData;
	local teamTeleporterTable = AutomoverData[self.Team].teleporterNodes;

	local actor = actorData.actor;
	local waypointData = actorData.waypointData;

	waypointData.previousNode = self:findClosestNode(actor.Pos, nil, true, true);
	if not waypointData.previousNode then
		self:setActorMovementModeToLeaveAutomovers(actorData);
		return;
	end
	local targetPosition = waypointData.movableObjectTarget ~= nil and waypointData.movableObjectTarget.Pos or waypointData.sceneTargets[1];
	waypointData.targetPosition = Vector(targetPosition.X, targetPosition.Y);
	waypointData.targetIsInsideAutomoverArea = self.combinedAutomoverArea:IsInside(waypointData.targetPosition);
	waypointData.targetIsBetweenPreviousAndNextNode = false;
	waypointData.actorReachedTargetInsideAutomoverArea = false;
	waypointData.actorReachedEndNodeForTargetOutsideAutomoverArea = false;
	waypointData.teleporterVisualsTimer = Timer(1000);

	local nodeWithShortestPathCoroutine = coroutine.create(self.findNodeWithShortestScenePath);
	while coroutine.status(nodeWithShortestPathCoroutine) ~= "dead" do
		local _, result = coroutine.resume(nodeWithShortestPathCoroutine, self, waypointData.targetPosition, waypointData.previousNode, waypointData.targetIsInsideAutomoverArea, actor.Team, actor.DigStrength);
		if result then
			waypointData.endNode = result.key;
		else
			coroutine.yield();
		end
	end

	if waypointData.endNode == nil then
		nodeWithShortestPathCoroutine = coroutine.create(self.findNodeWithShortestScenePath);
		while coroutine.status(nodeWithShortestPathCoroutine) ~= "dead" do
			local _, result = coroutine.resume(nodeWithShortestPathCoroutine, self, waypointData.targetPosition, waypointData.previousNode, false, actor.Team, actor.DigStrength);
			if result then
				waypointData.endNode = result.key;
			else
				coroutine.yield();
			end
		end

		if waypointData.endNode == nil then
			waypointData.endNode = self:findClosestNode(waypointData.targetPosition, waypointData.previousNode, false, false);
		end

		if waypointData.targetIsInsideAutomoverArea then
			nodeWithShortestPathCoroutine = coroutine.create(self.findNodeWithShortestScenePath);
			local _, result = coroutine.resume(nodeWithShortestPathCoroutine, self, waypointData.targetPosition, nil, true, actor.Team, actor.DigStrength);
			if result then
				local closestPotentiallyNonConnectedNode = result.key;
				local nonConnectedNodeThatEncompassesTargetExists = closestPotentiallyNonConnectedNode ~= nil and self.pathTable[waypointData.previousNode][closestPotentiallyNonConnectedNode] == nil;
				if nonConnectedNodeThatEncompassesTargetExists then
					waypointData.targetIsInsideAutomoverArea = false;
				end
			else
				coroutine.yield();
			end
		end
	end

	if waypointData.previousNode.UniqueID == waypointData.endNode.UniqueID then
		self:accountForSameStartingAndEndingNodeWhenSettingUpActorWaypointData(actorData);
	else
		actorData.direction = self.pathTable[waypointData.previousNode][waypointData.endNode].direction;
		if actorData.direction == Directions.Any then
			waypointData.nextNode = waypointData.previousNode;
			if not self:makeActorMoveToStartingNodeIfAppropriateWhenSettingUpActorWaypointData(actorData) then
				actorData.movementMode = self.movementModes.teleporting;
			end
		else
			waypointData.nextNode = teamNodeTable[waypointData.previousNode].connectedNodeData[actorData.direction].node;
			self:makeActorMoveToStartingNodeIfAppropriateWhenSettingUpActorWaypointData(actorData);
		end

		if waypointData.nextNode.UniqueID == waypointData.endNode.UniqueID and waypointData.targetIsInsideAutomoverArea then
			local areaToCheckForTargetPos = teamNodeTable[waypointData.previousNode].connectingAreas[actorData.direction];
			waypointData.targetIsBetweenPreviousAndNextNode = areaToCheckForTargetPos ~= nil and areaToCheckForTargetPos:IsInside(waypointData.targetPosition);
		end
	end
end

automoverActorFunctions.accountForSameStartingAndEndingNodeWhenSettingUpActorWaypointData = function(self, actorData)
	local teamNodeTable = AutomoverData[self.Team].nodeData;

	local actor = actorData.actor;
	local waypointData = actorData.waypointData;

	if waypointData.targetIsInsideAutomoverArea then
		local zoneBox = teamNodeTable[waypointData.previousNode].zoneBox;
		local actorAndTargetAreInSameArea = zoneBox:IsWithinBox(actor.Pos) or zoneBox:IsWithinBox(waypointData.targetPosition);
		local directionOfConnectingAreaActorIsIn = Directions.None;
		local directionOfConnectingAreaTargetIsIn = Directions.None;
		if not actorAndTargetAreInSameArea then
			for direction, connectingArea in pairs(teamNodeTable[waypointData.previousNode].connectingAreas) do
				if connectingArea:IsInside(actor.Pos) then
					directionOfConnectingAreaActorIsIn = direction;
				end
				if connectingArea:IsInside(waypointData.targetPosition) then
					directionOfConnectingAreaTargetIsIn = direction;
				end
			end
			actorAndTargetAreInSameArea = directionOfConnectingAreaActorIsIn == directionOfConnectingAreaTargetIsIn;
		end

		if actorAndTargetAreInSameArea then
			local distanceFromActorToTargetPosition = SceneMan:ShortestDistance(waypointData.targetPosition, actor.Pos, self.checkWrapping);
			local relevantAxis = math.abs(distanceFromActorToTargetPosition.X) > math.abs(distanceFromActorToTargetPosition.Y) and "X" or "Y";
			if relevantAxis == "X" then
				actorData.direction = distanceFromActorToTargetPosition[relevantAxis] > 0 and Directions.Left or Directions.Right;
			else
				actorData.direction = distanceFromActorToTargetPosition[relevantAxis] > 0 and Directions.Up or Directions.Down;
			end
			waypointData.nextNode = waypointData.previousNode;
			actorData.targetIsBetweenPreviousAndNextNode = true;
		else
			actorData.direction = self.oppositeDirections[directionOfConnectingAreaActorIsIn];
			waypointData.nextNode = waypointData.previousNode;
			waypointData.endNode = teamNodeTable[waypointData.previousNode].connectedNodeData[directionOfConnectingAreaTargetIsIn].node;
		end
	else
		self:makeActorMoveToStartingNodeIfAppropriateWhenSettingUpActorWaypointData(actorData);
	end
end

automoverActorFunctions.makeActorMoveToStartingNodeIfAppropriateWhenSettingUpActorWaypointData = function(self, actorData)
	local actor = actorData.actor;
	local waypointData = actorData.waypointData;

	local distanceFromActorToPreviousNode = SceneMan:ShortestDistance(waypointData.previousNode.Pos, actor.Pos, self.checkWrapping);
	local relevantAxis;
	local distanceThreshold;
	if actorData.direction == Directions.Any or actorData.direction == Directions.None then
		relevantAxis = (math.abs(distanceFromActorToPreviousNode.X) > math.abs(distanceFromActorToPreviousNode.Y)) and "X" or "Y";
		distanceThreshold = 0;
	else
		relevantAxis = (actorData.direction == Directions.Up or actorData.direction == Directions.Down) and "X" or "Y";
		distanceThreshold = 24;
	end

	if math.abs(distanceFromActorToPreviousNode[relevantAxis]) > distanceThreshold then
		if relevantAxis == "X" then
			actorData.direction = distanceFromActorToPreviousNode[relevantAxis] > 0 and Directions.Left or Directions.Right;
		else
			actorData.direction = distanceFromActorToPreviousNode[relevantAxis] > 0 and Directions.Up or Directions.Down;
		end
		waypointData.nextNode = waypointData.previousNode;
		return true;
	end

	return false;
end

automoverActorFunctions.handleTeleportingActorToAppropriateTeleporterForWaypoint = function(self, actorData)
	local teamTeleporterTable = AutomoverData[self.Team].teleporterNodes;

	local actor = actorData.actor;
	local waypointData = actorData.waypointData;

	local actorHasTeleported = waypointData.previousNode.UniqueID ~= waypointData.nextNode.UniqueID
	if waypointData.teleporterVisualsTimer:IsPastSimTimeLimit() then
		if not actorHasTeleported then
			if teamTeleporterTable[waypointData.endNode] then
				waypointData.nextNode = waypointData.endNode;
			else
				local closestTeleporterDistance;
				for teleporterNode, _ in pairs(teamTeleporterTable) do
					if self.pathTable[teleporterNode][waypointData.endNode] ~= nil and (closestTeleporterDistance == nil or self.pathTable[teleporterNode][waypointData.endNode].distance < closestTeleporterDistance) then
						closestTeleporterDistance = self.pathTable[teleporterNode][waypointData.endNode].distance;
						waypointData.nextNode = teleporterNode;
					end
				end
			end
			actor.Pos = waypointData.nextNode.Pos;
			waypointData.teleporterVisualsTimer:Reset();
		else
			actorData.movementMode = self.movementModes.freeze;
			self:changeScaleOfMOSRotatingAndAttachables(actor, 1);
		end
	else
		self:changeScaleOfMOSRotatingAndAttachables(actor, (actorHasTeleported and waypointData.teleporterVisualsTimer.SimTimeLimitProgress or 1 - waypointData.teleporterVisualsTimer.SimTimeLimitProgress));
		if not actorHasTeleported then
			self:centreActorToClosestNodeIfMovingInAppropriateDirection(actorData, true);
		end
		actor:FlashWhite(100);
		self:updateFrozenActor(actorData);
	end
end

automoverActorFunctions.handleActorInNextNodeZoneInternalBox = function(self, actorData)
	local teamNodeTable = AutomoverData[self.Team].nodeData;
	local teamTeleporterTable = AutomoverData[self.Team].teleporterNodes;

	local actor = actorData.actor;
	local waypointData = actorData.waypointData;

	waypointData.previousNode = waypointData.nextNode;
	if waypointData.previousNode.UniqueID == waypointData.endNode.UniqueID then
		return;
	end

	if teamTeleporterTable[waypointData.previousNode] then
		actor.Pos = waypointData.nextNode.Pos;
	end

	actorData.direction = self.pathTable[waypointData.previousNode][waypointData.endNode].direction;
	if actorData.direction == Directions.Any then
		actorData.movementMode = self.movementModes.teleporting;
		waypointData.teleporterVisualsTimer:Reset();
		return;
	else
		waypointData.nextNode = teamNodeTable[waypointData.previousNode].connectedNodeData[actorData.direction].node;
	end

	waypointData.nextNode = teamNodeTable[waypointData.previousNode].connectedNodeData[actorData.direction].node;
	if waypointData.movableObjectTarget then
		waypointData.targetPosition = Vector(waypointData.movableObjectTarget.Pos.X, waypointData.movableObjectTarget.Pos.Y);
	end

	if waypointData.nextNode.UniqueID == waypointData.endNode.UniqueID and waypointData.targetIsInsideAutomoverArea then
		local areaToCheckForTargetPos = teamNodeTable[waypointData.previousNode].connectingAreas[actorData.direction];
		waypointData.targetIsBetweenPreviousAndNextNode = areaToCheckForTargetPos ~= nil and areaToCheckForTargetPos:IsInside(waypointData.targetPosition);
	end
end

automoverActorFunctions.handleActorThatHasReachedItsEndNode = function(self, actorData)
	local teamNodeTable = AutomoverData[self.Team].nodeData;

	local actor = actorData.actor;
	local waypointData = actorData.waypointData;

	local function getDirectionForDistanceLargerAxis(distanceVector, minimumDistanceThreshold)
		local direction = Directions.None;

		local largerDistanceAxis = (math.abs(distanceVector.X) > math.abs(distanceVector.Y)) and "X" or "Y";
		if distanceVector[largerDistanceAxis] > minimumDistanceThreshold then
			direction = largerDistanceAxis == "Y" and Directions.Up or Directions.Left;
		elseif distanceVector[largerDistanceAxis] < -minimumDistanceThreshold then
			direction = largerDistanceAxis == "Y" and Directions.Down or Directions.Right;
		end
		return direction;
	end

	if waypointData.targetIsInsideAutomoverArea then
		if not waypointData.actorReachedTargetInsideAutomoverArea then
			local distanceFromActorToTargetPosition = SceneMan:ShortestDistance(waypointData.targetPosition, actor.Pos, self.checkWrapping);
			local thresholdForTreatingTargetAsReached = waypointData.movableObjectTarget == nil and self.sceneWaypointThresholdForTreatingTargetAsReached or self.movableObjectWaypointThresholdForTreatingTargetAsReached;
			actorData.direction = getDirectionForDistanceLargerAxis(distanceFromActorToTargetPosition, thresholdForTreatingTargetAsReached);
			if actorData.direction == Directions.None then
				waypointData.actorReachedTargetInsideAutomoverArea = true;
				actorData.movementMode = self.movementModes.freeze;
			end
		elseif waypointData.actorReachedTargetInsideAutomoverArea then
			actorData.movementMode = self.movementModes.freeze;
			if waypointData.movableObjectTarget ~= nil then
				if SceneMan:ShortestDistance(actor.Pos, waypointData.movableObjectTarget.Pos, self.checkWrapping):MagnitudeIsGreaterThan(self.movableObjectWaypointThresholdForTreatingTargetAsReached * 2) then
					waypointData.previousNode = nil;
					waypointData.nextNode = nil;
					waypointData.endNode = nil;
				end
			else
				if #waypointData.sceneTargets > 1 then
					table.remove(waypointData.sceneTargets, 1);
					waypointData.previousNode = nil;
					waypointData.nextNode = nil;
					waypointData.endNode = nil;
				else
					actorData.waypointData = nil;
				end
			end
		end
	else
		if teamNodeTable[waypointData.endNode].zoneBox:IsWithinBox(actor.Pos) and waypointData.exitPath == nil then
			waypointData.exitPath = {};
			SceneMan.Scene:CalculatePathAsync(
				function(pathRequest)
					if pathRequest.Status ~= PathRequest.Solved then
						waypointData.exitPath[#waypointData.exitPath + 1] = waypointData.targetPosition;
					else
						for scenePathEntryPosition in pathRequest.Path do
							waypointData.exitPath[#waypointData.exitPath + 1] = scenePathEntryPosition;
						end
					end
				end,
				actor.Pos, waypointData.targetPosition, false, GetPathFindingDefaultDigStrength(), self.Team
			);
		elseif waypointData.exitPath ~= nil and #waypointData.exitPath > 0 then
			local distanceFromActorToFirstExitPathPosition = SceneMan:ShortestDistance(waypointData.exitPath[1], actor.Pos, self.checkWrapping);
			if #waypointData.exitPath > 1 and distanceFromActorToFirstExitPathPosition:MagnitudeIsLessThan(20) then
				table.remove(waypointData.exitPath, 1);
				distanceFromActorToFirstExitPathPosition = SceneMan:ShortestDistance(waypointData.exitPath[1], actor.Pos, self.checkWrapping);
			end
			actorData.direction = getDirectionForDistanceLargerAxis(distanceFromActorToFirstExitPathPosition, 0);

			local endNodeData = teamNodeTable[waypointData.endNode];
			if #waypointData.exitPath > 0 and not endNodeData.zoneBox:IsWithinBox(waypointData.exitPath[1]) and (endNodeData.connectedNodeData[actorData.direction] == nil or not endNodeData.connectingAreas[actorData.direction]:IsInside(waypointData.exitPath[1])) then
				local velocityToAddToActor = distanceFromActorToFirstExitPathPosition.Normalized:FlipX(true):FlipY(true) * self.movementAcceleration * 5;
				if math.abs(velocityToAddToActor.X) < 1 and velocityToAddToActor.Y < 0 and SceneMan.GlobalAcc.Y > 0 then
					velocityToAddToActor.Y = velocityToAddToActor.Y * 2;
				end
				actor.Vel = actor.Vel + velocityToAddToActor;
			end
		end
	end
end

automoverActorFunctions.slowDownFastActorToMovementSpeed = function(self, actorData)
	local actor = actorData.actor;

	if actor.Vel:MagnitudeIsGreaterThan(self.movementSpeed * 1.1) then
		actor.Vel = actor.Vel / 1.3;
		return false;
	end

	if actor.AngularVel > self.movementSpeed then
		actor.AngularVel = actor.AngularVel - (self.movementAcceleration);
		return false;
	elseif actor.AngularVel < -self.movementSpeed then
		actor.AngularVel = actor.AngularVel + (self.movementAcceleration);
		return false;
	end

	return true;
end

automoverActorFunctions.centreActorToClosestNodeIfMovingInAppropriateDirection = function(self, actorData, forceCentring)
	tracy.ZoneBegin();
	local teamNodeTable = AutomoverData[self.Team].nodeData;

	local actor = actorData.actor;
	local actorDirection = actorData.direction;

	local oldClosestNode = actorData.currentClosestNode;
	local closestNode = self:findClosestNode(actor.Pos, actorData.currentClosestNode, true, true) or actorData.currentClosestNode;
	actorData.currentClosestNode = closestNode;
	if not closestNode then
		actor:FlashWhite(100);
		actor:MoveOutOfTerrain(0);
		self:setActorMovementModeToLeaveAutomovers(actorData);
		tracy.ZoneEnd();
		return false;
	end

	local isStuck = actorData.movementMode == self.movementModes.unstickActor;
	forceCentring = forceCentring or isStuck;
	local directionToUseForCentering;
	local directionConnectingArea;
	if not forceCentring and not teamNodeTable[closestNode].zoneBox:IsWithinBox(actor.Pos) then
		for direction, connectingArea in pairs(teamNodeTable[closestNode].connectingAreas) do
			if connectingArea:IsInside(actor.Pos) then
				directionToUseForCentering = direction;
				directionConnectingArea = connectingArea;
				break;
			end
		end
	end

	if forceCentring or actorDirection == directionToUseForCentering or actorDirection == self.oppositeDirections[directionToUseForCentering] then
		local actorSizeCenteringAdjustment = Vector(0, math.min(24, math.max(0, actor.Radius - 32)));
		if IsACRocket(actor) or IsACDropShip(actor) then
			actorSizeCenteringAdjustment.Y = 0;
		end
		local distanceToClosestNode = SceneMan:ShortestDistance(closestNode.Pos + actorSizeCenteringAdjustment, actor.Pos, self.checkWrapping);
		local centeringAxes;
		if forceCentring then
			centeringAxes = { "X", "Y" };
		else
			centeringAxes = { (directionToUseForCentering == Directions.Up or directionToUseForCentering == Directions.Down) and "X" or "Y"; }
		end
		local gravityAdjustment = SceneMan.GlobalAcc * TimerMan.DeltaTimeSecs * -1;
		local centeringSpeedAndDistance = self.movementAcceleration * 5;

		for _, centeringAxis in pairs(centeringAxes) do
			if actor.MovePathSize > 0 then
				-- Collect all points ahead of us in the box to adjust to centre
				local positionsToFixUp = {};
				for pos in actor.MovePath do
					if pos[centeringAxis] == closestNode.Pos[centeringAxis] or directionConnectingArea == nil or not directionConnectingArea:IsInside(pos) then
						break;
					end
					local adjustedPos = pos;
					adjustedPos[centeringAxis] = closestNode.Pos[centeringAxis];
					table.insert(positionsToFixUp, adjustedPos);
				end

				-- Clear these points from our move path
				for i = 1, #positionsToFixUp do
					actor:RemoveMovePathBeginning();
				end

				-- And add the adjusted point back in (in reverse order, as they're at the beginning of our movepath)
				for i = #positionsToFixUp, 1, -1 do
					actor:AddToMovePathBeginning(positionsToFixUp[i]);
				end
			end

			if distanceToClosestNode[centeringAxis] > centeringSpeedAndDistance then
				actor.Vel[centeringAxis] = -centeringSpeedAndDistance + gravityAdjustment[centeringAxis];
			elseif distanceToClosestNode[centeringAxis] < -centeringSpeedAndDistance then
				actor.Vel[centeringAxis] = centeringSpeedAndDistance + gravityAdjustment[centeringAxis];
			else
				if isStuck then
					actorData.movementMode = self.movementModes.move;
					actorData.unstickTimer:Reset();
				end

				actor.Vel[centeringAxis] = gravityAdjustment[centeringAxis];
				actor.Pos[centeringAxis] = closestNode.Pos[centeringAxis] + actorSizeCenteringAdjustment[centeringAxis];
			end
		end

		tracy.ZoneEnd();
		return true;
	end

	tracy.ZoneEnd();
	return false;
end

automoverActorFunctions.updateFrozenActor = function(self, actorData)
	tracy.ZoneBegin();
	local actor = actorData.actor;

	local gravityAdjustment = SceneMan.GlobalAcc * TimerMan.DeltaTimeSecs * -1;
	for _, axis in pairs({"X", "Y"}) do
		if math.abs(actor.Vel[axis]) > self.movementAcceleration * 2 then
			actor.Vel[axis] = actor.Vel[axis] + ((actor.Vel[axis] > self.movementAcceleration and -self.movementAcceleration or self.movementAcceleration) * 2) + gravityAdjustment[axis];
		end
		if math.abs(actor.Vel[axis]) <= self.movementAcceleration * 2 then
			actor.Vel[axis] = gravityAdjustment[axis];
			if IsAHuman(actor) then
				ToAHuman(actor).ProneState = AHuman.NOTPRONE;
				actor:GetController():SetState(Controller.BODY_CROUCH, false);
			end
		end
	end

	tracy.ZoneEnd();
end

automoverActorFunctions.updateMovingActor = function(self, actorData, anyCenteringWasDone)
	tracy.ZoneBegin();
	local actor = actorData.actor;
	local actorDirection = actorData.direction;

	local actorController = actor:GetController();
	if not actorController:IsMouseControlled() and not actorController:IsGamepadControlled() then
		if actorDirection == Directions.Left and actor.HFlipped == false then
			actor.HFlipped = true;
		elseif actorDirection == Directions.Right and actor.HFlipped == true then
			actor.HFlipped = false;
		end
	end

	local directionMovementTable = {
		[Directions.Up] = { acceleration = Vector(0, -self.movementAcceleration), aHumanRotAngle = 0, aHumanProneState = AHuman.NOTPRONE, aHumanCrouchState = false },
		[Directions.Down] = { acceleration = Vector(0, self.movementAcceleration), aHumanRotAngle = math.pi, aHumanProneState = AHuman.GOPRONE, aHumanCrouchState = true },
		[Directions.Left] = { acceleration = Vector(-self.movementAcceleration, 0), aHumanRotAngle = math.pi * 0.5, aHumanProneState = AHuman.GOPRONE, aHumanCrouchState = true },
		[Directions.Right] = { acceleration = Vector(self.movementAcceleration, 0), aHumanRotAngle = math.pi * -0.5, aHumanProneState = AHuman.GOPRONE, aHumanCrouchState = true },
	};

	local gravityAdjustment = SceneMan.GlobalAcc * TimerMan.DeltaTimeSecs * -1;
	for direction, movementTable in pairs(directionMovementTable) do
		if actorDirection == direction then
			if not anyCenteringWasDone and  not self.slowActorVelInNoneMovementDirectionsWhenInZoneBoxDisabled then
				local slowdownAxis = (direction == Directions.Up or direction == Directions.Down) and "X" or "Y";
				actor.Vel[slowdownAxis] = actor.Vel[slowdownAxis] * 0.75;
			end

			actor.Vel = (actor.Vel + movementTable.acceleration):CapMagnitude(self.movementSpeed);
			actor.Vel = actor.Vel + gravityAdjustment;

			local rotAngleGoal = 0;
			local verticalRotationAdjustment = 1;
			if IsAHuman(actor) then
				if not self.humansRemainUpright then
					rotAngleGoal = movementTable.aHumanRotAngle;
					verticalRotationAdjustment = direction == Directions.Down and -actor.FlipFactor or 1;
					ToAHuman(actor).ProneState = movementTable.aHumanProneState;
				end
				actorController:SetState(Controller.BODY_CROUCH, movementTable.aHumanCrouchState);
			end
			if actor.RotAngle > rotAngleGoal + self.movementAcceleration then
				actor.RotAngle = actor.RotAngle - (self.movementAcceleration * 0.25 * verticalRotationAdjustment);
			elseif actor.RotAngle < rotAngleGoal - self.movementAcceleration then
				actor.RotAngle = actor.RotAngle + (self.movementAcceleration * 0.25 * verticalRotationAdjustment);
			else
				actor.RotAngle = rotAngleGoal;
			end
		end
	end

	tracy.ZoneEnd();
end

automoverUIFunctions.updateInfoUI = function(self)
	local desiredPieMenuFullInnerRadius = 128;

	local formattedVisualEffectsNameTable = {
		types = { [""] = "None", chevron = "Pulse", chevronNarrow = "Tight Pulse" },
		sizes = { [self.visualEffectsSizes.small] = "Small", [self.visualEffectsSizes.medium] = "Medium", [self.visualEffectsSizes.large] = "Large" },
	};
	local textDataTable = {
		"Controlled Automovers: " .. tostring(AutomoverData[self.Team].nodeDataCount - AutomoverData[self.Team].teleporterNodesCount),
		"Controlled Teleporters: " .. tostring(AutomoverData[self.Team].teleporterNodesCount),
		"Number of Affected Actors: " .. tostring(self.affectedActorsCount),
		"All Teams Accepted: " .. (self.acceptsAllTeams and "True" or "False"),
		"Crafts Accepted: " .. (self.acceptsCrafts and "True" or "False"),
		"Humans Remain Upright: " .. (self.humansRemainUpright and "True" or "False");
		"Movement Speed: " .. tostring(self.movementSpeed),
		"Mass Limit: " .. tostring(self.massLimit),
	};
	if self.visualEffectsSelectedType == "" then
		textDataTable[#textDataTable + 1] = "Selected Visual Effects: None"
	else
		textDataTable[#textDataTable + 1] = "Selected Visual Effects: " .. formattedVisualEffectsNameTable.sizes[self.visualEffectsSelectedSize] .. " " .. formattedVisualEffectsNameTable.types[self.visualEffectsSelectedType];
	end

	if self:GetNumberValue("ModifyMovementSpeed") > 0 then
		table.insert(textDataTable, 1, "---------------------------");
		table.insert(textDataTable, 1, "Change the Movement Speed");
		table.insert(textDataTable, 1, "Move or Scroll Up or Down to");
		table.insert(textDataTable, 1, "---------------------------");
		desiredPieMenuFullInnerRadius = 160;
	elseif self:GetNumberValue("ModifyMassLimit") > 0 then
		table.insert(textDataTable, 1, "--------------------------");
		table.insert(textDataTable, 1, "Change the Mass Limit");
		table.insert(textDataTable, 1, "Move or Scroll Up or Down to");
		table.insert(textDataTable, 1, "--------------------------");
		desiredPieMenuFullInnerRadius = 160;
	end

	local centerPoint = Vector(self.Pos.X, self.Pos.Y);
	local config = { useSmallText = self.infoUIUseSmallText, bgColour = self.infoUIBGColour, outlineWidth = self.infoUIOutlineWidth, outlineColour = self.infoUIOutlineColour, transparency = self.infoUITransparency };

	self:drawTextBox(textDataTable, centerPoint, config);

	return desiredPieMenuFullInnerRadius;
end

automoverUIFunctions.drawTextBox = function(self, textDataTable, maxSizeBoxOrCenterPoint, config)
	config = self:setupTextBoxConfigIfNeeded(config, maxSizeBoxOrCenterPoint);
	local centerPoint, maxSize = self:setupCenterPointAndMaxSizeAndFixTextDataTableIfNecessary(textDataTable, maxSizeBoxOrCenterPoint, config);
	local topLeft = Vector(centerPoint.X - maxSize.X * 0.5, centerPoint.Y - maxSize.Y * 0.5);

	local boxFillPrimitives = {};
	local player = self:GetController().Player;
	boxFillPrimitives[#boxFillPrimitives + 1] = BoxFillPrimitive(player, Vector(topLeft.X - config.padding, topLeft.Y - config.padding), Vector(topLeft.X + maxSize.X + config.padding, topLeft.Y + maxSize.Y + config.padding), config.outlineColour);
	boxFillPrimitives[#boxFillPrimitives + 1] = BoxFillPrimitive(player, Vector(topLeft.X - config.padding + config.outlineWidth, topLeft.Y - config.padding + config.outlineWidth), Vector(topLeft.X + maxSize.X + config.padding - config.outlineWidth, topLeft.Y + maxSize.Y + config.padding - config.outlineWidth), config.bgColour);
	PrimitiveMan:DrawPrimitives(config.transparency, boxFillPrimitives)

	for textIndex, textDataEntry in ipairs(textDataTable) do
		local linePos = Vector(topLeft.X, topLeft.Y + (textIndex - 1) * self.uiLineHeight[config.useSmallText]);
		local alignment = type(textDataEntry.alignment) ~= "nil" and textDataEntry.alignment or 1;
		if alignment == 1 then
			linePos.X = linePos.X + maxSize.X * 0.5;
		elseif alignment == 2 then
			linePos.X = linePos.X + maxSize.X;
		end
		PrimitiveMan:DrawTextPrimitive(player, linePos, textDataEntry.text, config.useSmallText, alignment);
	end
end

automoverUIFunctions.setupTextBoxConfigIfNeeded = function(self, config, maxSizeBoxOrCenterPoint)
	if type(config) == "nil" then
		config = {};
	end

	if type(config.useSmallText) == "nil" then
		config.useSmallText = false;
	end

	if type(config.scaleBoxToFitText) == "nil" or maxSizeBoxOrCenterPoint.ClassName == "Vector" then
		config.scaleBoxToFitText = (maxSizeBoxOrCenterPoint.ClassName == "Vector");
	end

	if type(config.snapSize) == "nil" then
		config.snapSize = config.useSmallText and 2 or 4;
	end

	if type(config.padding) == "nil" then
		config.padding = config.useSmallText and 3 or 6;
	end

	if type(config.bgColour) == "nil" then
		config.bgColour = 127;
	end

	if type(config.outlineWidth) == "nil" then
		config.outlineWidth = 2;
	end

	if type(config.outlineColour) == "nil" then
		config.outlineColour = 71;
	end

	if type(config.transparency) == "nil" then
		config.transparency = 0;
	end

	return config;
end

automoverUIFunctions.setupCenterPointAndMaxSizeAndFixTextDataTableIfNecessary = function(self, textDataTable, maxSizeBoxOrCenterPoint, config)
	local centerPoint = maxSizeBoxOrCenterPoint;
	local maxSize = Vector(math.huge, math.huge);
	if maxSizeBoxOrCenterPoint.ClassName == "Box" then
		centerPoint = maxSizeBoxOrCenterPoint.Center;
		maxSize = Vector(maxSizeBoxOrCenterPoint.Width - config.padding * 2, maxSizeBoxOrCenterPoint.Height - config.padding * 2);
	end

	maxSize = self:getScaledMaxSizeAndFixTextDataTableIfNecessary(textDataTable, maxSize, config);
	return centerPoint, maxSize;
end

automoverUIFunctions.getScaledMaxSizeAndFixTextDataTableIfNecessary = function(self, textDataTable, maxSize, config)
	local numberOfLines = 0;
	local maxSizeIfScalingBoxToFitText = Vector(0, 0);
	for i = 1, #textDataTable do
		if type(textDataTable[i]) == "string" then
			textDataTable[i] = { text = textDataTable[i] };
		end

		numberOfLines = numberOfLines + self:getNumberOfLinesForText(textDataTable[i].text, maxSize.X, config.useSmallText);
		if config.scaleBoxToFitText then
			local textWidth = FrameMan:CalculateTextWidth(textDataTable[i].text, config.useSmallText);
			maxSizeIfScalingBoxToFitText.X = math.max(maxSizeIfScalingBoxToFitText.X, textWidth);
		end
	end
	local totalHeight = numberOfLines * self.uiLineHeight[config.useSmallText];
	for i = #textDataTable, 1, -1 do
		if totalHeight > maxSize.Y then
			totalHeight = totalHeight - self.uiLineHeight[config.useSmallText] * self:getNumberOfLinesForText(textDataTable[i].text, maxSize.X, config.useSmallText);
			table.remove(textDataTable, i);
		else
			break;
		end
	end

	maxSizeIfScalingBoxToFitText.Y = totalHeight;

	if config.scaleBoxToFitText then
		maxSizeIfScalingBoxToFitText.X = maxSizeIfScalingBoxToFitText.X + config.snapSize - maxSizeIfScalingBoxToFitText.X % config.snapSize;
		maxSizeIfScalingBoxToFitText.Y = maxSizeIfScalingBoxToFitText.Y + config.snapSize - maxSizeIfScalingBoxToFitText.Y % config.snapSize;
		maxSize = Vector(math.min(maxSize.X, maxSizeIfScalingBoxToFitText.X), math.min(maxSize.Y, maxSizeIfScalingBoxToFitText.Y));
	end

	return maxSize;
end

automoverUIFunctions.getNumberOfLinesForText = function(self, textString, maxWidth, useSmallText)
	local numberOfLines = 1;
	local stringWidth = FrameMan:CalculateTextWidth(textString, useSmallText);
	if stringWidth > maxWidth then
		numberOfLines = numberOfLines + math.modf(stringWidth / maxWidth);
	end
	
	return numberOfLines;
end
