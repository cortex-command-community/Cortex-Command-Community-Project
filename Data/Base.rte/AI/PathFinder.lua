
-- this actor finds the shortest path between two scene positions
-- global variables for input: gFindPathFrom, gFindPathTo
-- global variables for output: gWaypoints
-- gPathObstMaxHeight is the largets vertical obstacle height along the path

function Create(self)
	self:SetControllerMode(Controller.CIM_DISABLED, -1);
end

function Update(self)
	if self.IsWaitingOnNewMovePath then
		return;
	end

	if #gWayPoints == 0 and #self.MovePath ~= 0 then
		for Wpt in self.MovePath do
			table.insert(gWaypoints, Vector(Wpt.X, Wpt.Y));
		end
	end

	if gFindPathFrom and gFindPathTo then
		self.Pos = gFindPathFrom;
		gFindPathFrom = nil;

		self:ClearAIWaypoints();
		self:AddAISceneWaypoint(gFindPathTo);
		self:UpdateMovePath();

		gFindPathTo = nil;
		gWaypoints = {};
	end
end
