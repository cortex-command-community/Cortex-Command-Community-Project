function Create(self)
	self.effectRadius = 125;
	self.materialThreshold = 25;
	self.strength = self.PinStrength; --Affects the duration of the effect
	self.flashScreen = false; --Do not turn on if you are prone to seizures

	self.actorTable = {};
	local actorCount = 0; --Diminishes the effect the more actors are affected

	for actor in MovableMan.Actors do
		local dist = SceneMan:ShortestDistance(self.Pos, actor.Pos, SceneMan.SceneWrapsX);
		if dist:MagnitudeIsLessThan(self.effectRadius) then
			local skipPx = 1 + math.floor(math.sqrt(dist.Magnitude));
			local strCheck = SceneMan:CastStrengthSumRay(self.Pos, self.Pos + dist, skipPx, rte.airID);
			if strCheck < self.materialThreshold/skipPx then
				--The effect is diminished by target actor mass, material strength and distance
				local resistance = math.sqrt(math.abs(actor.Mass) + actor.Material.StructuralIntegrity + dist.Magnitude + 1) + actorCount;
				actor:SetNumberValue("RoninScrambler", math.floor(actor:GetNumberValue("RoninScrambler") + self.strength/resistance));
				actor:FlashWhite(20);
				if self.flashScreen and actor:IsPlayerControlled() then
					local screen = ActivityMan:GetActivity():ScreenOfPlayer(actor:GetController().Player);
					local white, black = 254, 245;
					FrameMan:FlashScreen(screen, white, 1000);
				end
				table.insert(self.actorTable, actor);
				actorCount = actorCount + 1;
			end
		end
	end
	
	for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
		if SceneMan:AnythingUnseen(team) then
			local size = self.effectRadius * 0.6;
			local dots = 10;
			SceneMan:RestoreUnseenBox(self.Pos.X - (size * 0.5), self.Pos.Y - (size * 0.5), size, size, team);
			for i = 1, dots do
				local vector = Vector(size, 0):RadRotate(6.28 * i/dots);
				local startPos = self.Pos + vector;
				SceneMan:RestoreUnseenBox(startPos.X - (size * 0.5), startPos.Y - (size * 0.5), size, size, team);
			end
		end
	end

	self.buzzSound = CreateSoundContainer("Ronin Scrambler Buzz", "Ronin.rte");
end

function Update(self)
	self.ToSettle = false;
	local actorCount = 0;
	for i = 1, #self.actorTable do
		if MovableMan:IsActor(self.actorTable[i]) then
			local actor = ToActor(self.actorTable[i]);
			if actor:NumberValueExists("RoninScrambler") and actor.Status < Actor.DYING then
				actorCount = actorCount + 1;
				local numberValue = actor:GetNumberValue("RoninScrambler");
				if numberValue > 0 then
					actor.Status = Actor.UNSTABLE;
					local ctrl = actor:GetController();
					local dir = 0;
					if ctrl:IsState(Controller.MOVE_LEFT) then
						dir = dir - 1;
					end

					if ctrl:IsState(Controller.MOVE_RIGHT) then
						dir = dir + 1;
					end

					actor.AngularVel = actor.AngularVel - dir/(1 + math.abs(actor.AngularVel));
					if math.random(50) < numberValue then
						for i = 0, 29 do --Go through and disable the gameplay-related controller states
							ctrl:SetState(i, false);
						end
					end

					local framesPerFlash = 6;
					if (numberValue/framesPerFlash) - math.floor(numberValue/framesPerFlash) == 0 then
						actor:FlashWhite(1);
						if math.random() < 0.5 then
							self.buzzSound:Play(actor.Pos);
						end
					end

					actor:SetNumberValue("RoninScrambler", numberValue - 1);
				else
					actor:RemoveNumberValue("RoninScrambler");
				end
			end
		end
	end

	if actorCount == 0 then
		self.ToDelete = true;
	end
end