function FindStartPositionWithShortestPathToEndPosition(startPositions, endPosition, team, movePathToGround, digStrength)
	if startPositions == nil or type(startPositions) ~= "table" then
		print("FindShortestPathAsync Error: A table of start positions is required.");
		return;
	end
	if endPosition == nil or type(endPosition) ~= "userdata" or endPosition.ClassName ~= "Vector" then
		print("FindShortestPathAsync Error: An end position is required.");
		return;
	end
	if team == nil then
		print("FindShortestPathAsync Error: A team is required, -1 is allowed.");
		return;
	end
	
	movePathToGround = movePathToGround or false;
	digStrength = digStrength or GetPathFindingDefaultDigStrength();
	
	local closestStartPositionKey;
	local closestStartPosition;
	local totalCostToClosestStartPosition = 1/0;
	local pathRequestsCompleted = 0;
	local totalPathingRequests = 0;
	for startPositionKey, startPosition in pairs(startPositions) do
		SceneMan.Scene:CalculatePathAsync(
			function(pathRequest)
				if pathRequest.TotalCost < totalCostToClosestStartPosition then
					closestStartPositionKey = startPositionKey;
					closestStartPosition = startPosition;
					totalCostToClosestStartPosition = pathRequest.TotalCost;
				end
				pathRequestsCompleted = pathRequestsCompleted + 1;
			end, 
			startPosition, endPosition, movePathToGround, digStrength, team
		);
		totalPathingRequests = totalPathingRequests + 1;
	end
	
	while pathRequestsCompleted < totalPathingRequests do
		if totalCostToClosestStartPosition == 0 then
			break;
		end
		coroutine.yield();
	end
	
	return {key = closestStartPositionKey, position = closestStartPosition, totalCost = totalCostToClosestStartPosition};
end