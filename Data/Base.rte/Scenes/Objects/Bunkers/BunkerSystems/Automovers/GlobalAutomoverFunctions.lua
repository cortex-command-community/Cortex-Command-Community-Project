if not AutomoverData then
	AutomoverData = {};
	for team = Activity.NOTEAM, Activity.TEAM_4 do
		AutomoverData[team] = {
			energyLevel = 0,
			nodeData = {},
			nodeDataCount = 0,
			teleporterNodes = {},
			teleporterNodesCount = 0,
		};
	end
end

function Automovers_AddNode(node)
	local teamAutomoverData = AutomoverData[node.Team];
	
	local automoverDefaultNodeSize = 48;

	if teamAutomoverData.nodeData[node] == nil then
		for nodeKey, nodeInfo in pairs(teamAutomoverData.nodeData) do
			if type(nodeKey) ~= "string" then
				if nodeInfo.zoneBox ~= nil then
					local width = node:NumberValueExists("ZoneWidth") and node:GetNumberValue("ZoneWidth") or automoverDefaultNodeSize;
					local height = node:NumberValueExists("ZoneHeight") and node:GetNumberValue("ZoneHeight") or automoverDefaultNodeSize;
					local nodeBox = Box(Vector(node.Pos.X - width * 0.5 + 0.5, node.Pos.Y - height * 0.5), Vector(node.Pos.X + width * 0.5, node.Pos.Y + height * 0.5));
					if nodeInfo.zoneBox:IntersectsBox(nodeBox) then
						node.ToDelete = true;
						return false;
					end
				end
			end
		end

		teamAutomoverData.nodeData[node] = {
			size = Vector(),
			zoneBox,
			zoneInternalBox,
			connectedNodeData = {},
			connectingAreas = {},
		}

		if node.PresetName == "Teleporter Node" then
			teamAutomoverData.teleporterNodes[node] = true;
			teamAutomoverData.teleporterNodesCount = teamAutomoverData.teleporterNodesCount + 1;
		end

		local width = node:NumberValueExists("ZoneWidth") and node:GetNumberValue("ZoneWidth") or automoverDefaultNodeSize;
		local height = node:NumberValueExists("ZoneHeight") and node:GetNumberValue("ZoneHeight") or automoverDefaultNodeSize;

		teamAutomoverData.nodeData[node].size = Vector(width, height);
		teamAutomoverData.nodeData[node].zoneBox = Box(Vector(node.Pos.X - width * 0.5, node.Pos.Y - height * 0.5), Vector(node.Pos.X + width * 0.5, node.Pos.Y + height * 0.5));
		teamAutomoverData.nodeData[node].zoneInternalBox = Box(Vector(node.Pos.X - width * 0.25, node.Pos.Y - height * 0.25), Vector(node.Pos.X + width * 0.25, node.Pos.Y + height * 0.25));

		local nodesAffectedByThisAutomover = Automovers_CheckConnections(node);
		if nodesAffectedByThisAutomover ~= nil then
			for _, affectedNode in pairs(nodesAffectedByThisAutomover) do
				Automovers_CheckConnections(affectedNode);
			end
		end

		return true;
	end
	return false;
end

function Automovers_RemoveNode(node)
	local teamAutomoverData = AutomoverData[node.Team];

	local removedNodeTable = teamAutomoverData.nodeData[node];
	if type(removedNodeTable) ~= "nil" then
		teamAutomoverData.nodeData[node] = nil;
		for direction, nodeData in pairs(removedNodeTable.connectedNodeData) do
			teamAutomoverData.nodeData[nodeData.node] = nil;
			nodeData.node:SetNumberValue("shouldReaddNode", 1);
		end
		teamAutomoverData.nodeDataCount = teamAutomoverData.nodeDataCount - 1;
	end

	if node.PresetName == "Teleporter Node" then
		teamAutomoverData.teleporterNodes[node] = nil;
		teamAutomoverData.teleporterNodesCount = teamAutomoverData.teleporterNodesCount - 1;
	end
end

local function pathBetweenNodesIsNotObstructed(startNode, targetNode, direction)
	local teamAutomoverData = AutomoverData[startNode.Team];
	
	local startNodeData = teamAutomoverData.nodeData[startNode];
	local targetNodeData = teamAutomoverData.nodeData[targetNode];
	local smallerNodeZoneSize = Vector(math.min(startNodeData.size.X, targetNodeData.size.X), math.min(startNodeData.size.Y, targetNodeData.size.Y));

	local spreadRaysHorizontally = direction == Directions.Up or direction == Directions.Down;
	local rayOffsets = {
		spreadRaysHorizontally and Vector(-smallerNodeZoneSize.X * 0.25, 0) or Vector(0, -smallerNodeZoneSize.Y * 0.25),
		Vector(),
		spreadRaysHorizontally and Vector(smallerNodeZoneSize.X * 0.25, 0) or Vector(0, smallerNodeZoneSize.Y * 0.25),
	};

	local checkWrapping = SceneMan.SceneWrapsX or SceneMan.SceneWrapsY;
	local rayVector1 = SceneMan:ShortestDistance(startNode.Pos, targetNode.Pos, checkWrapping);
	local rayVector2 = SceneMan:ShortestDistance(targetNode.Pos, startNode.Pos, checkWrapping);
	for _, rayOffset in ipairs(rayOffsets) do
		if SceneMan:CastStrengthRay(startNode.Pos + rayOffset, rayVector1, 15, Vector(), 4, 0, true) or SceneMan:CastStrengthRay(targetNode.Pos + rayOffset, rayVector2, 15, Vector(), 4, 0, true) then
			return false;
		end
	end
	return true;
end

function Automovers_CheckConnections(node)
	local teamAutomoverData = AutomoverData[node.Team];

	local nodeData = teamAutomoverData.nodeData[node];
	local nodesInDirections = { [Directions.Up] = {}, [Directions.Down] = {}, [Directions.Left] = {}, [Directions.Right] = {} };
	local affectedNodes = {};

	local thisNodeIsHorizontalOnly = node.PresetName:find("Horizontal Only");
	local thisNodeIsVerticalOnly = node.PresetName:find("Vertical Only");

	local checkWrapping = SceneMan.SceneWrapsX or SceneMan.SceneWrapsY;

	for otherNode, otherNodeData in pairs(teamAutomoverData.nodeData) do
		if type(otherNode) ~= "string" and MovableMan:IsParticle(otherNode) and (not teamAutomoverData.teleporterNodes[node] or not teamAutomoverData.teleporterNodes[otherNode]) then
			local otherNodeIsHorizontalOnly = otherNode.PresetName:find("Horizontal Only");
			local otherNodeIsVerticalOnly = otherNode.PresetName:find("Vertical Only");
			local distanceToOtherNode = SceneMan:ShortestDistance(node.Pos, otherNode.Pos, checkWrapping);
			local width = nodeData.size.X * 0.5 + nodeData.size.X * 0.5;
			local height = nodeData.size.Y * 0.5 + nodeData.size.Y * 0.5;
			if not thisNodeIsHorizontalOnly and not otherNodeIsHorizontalOnly and distanceToOtherNode.Y <= -height and distanceToOtherNode.X == 0 then
				table.insert(nodesInDirections[Directions.Up], otherNode);
			elseif not thisNodeIsHorizontalOnly and not otherNodeIsHorizontalOnly and distanceToOtherNode.Y >= height and distanceToOtherNode.X == 0 then
				table.insert(nodesInDirections[Directions.Down], otherNode);
			elseif not thisNodeIsVerticalOnly and not otherNodeIsVerticalOnly and distanceToOtherNode.X <= -width and distanceToOtherNode.Y == 0 then
				table.insert(nodesInDirections[Directions.Left], otherNode);
			elseif not thisNodeIsVerticalOnly and not otherNodeIsVerticalOnly and distanceToOtherNode.X >= width and distanceToOtherNode.Y == 0 then
				table.insert(nodesInDirections[Directions.Right], otherNode);
			end
		end
	end

	for direction, nodesInDirection in pairs(nodesInDirections) do
		local closestNode;
		local distanceToClosestNode;
		for _, nodeInDirection in pairs(nodesInDirection) do
			local distanceToNode = SceneMan:ShortestDistance(node.Pos, nodeInDirection.Pos, checkWrapping);
			if (distanceToClosestNode == nil or distanceToNode.SqrMagnitude < distanceToClosestNode.SqrMagnitude) then
				closestNode = nodeInDirection;
				distanceToClosestNode = distanceToNode;
			end
		end
		
		if closestNode ~= nil and pathBetweenNodesIsNotObstructed(node, closestNode, direction, nodeData.size) then
			affectedNodes[direction] = closestNode;
			nodeData.connectedNodeData[direction] = { node = closestNode, distance = distanceToClosestNode };
			if nodeData.connectingAreas[direction] == nil then
				nodeData.connectingAreas[direction] = Area();
			end
		end
	end
	return affectedNodes;
end