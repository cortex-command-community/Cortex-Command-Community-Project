function Create(self)
	self.signalStrength = 10000;
	self.signalDecrement = 10;
	self.maxScanRange = 360;
	self.scanSpreadAngle = math.rad(self.ParticleSpreadRange);
	self.flashTimer = Timer();
	self.activity = ActivityMan:GetActivity();
end

function Update(self)
	local parent = self:GetRootParent();
	if IsActor(parent) and ToActor(parent):IsPlayerControlled() then
		if self.detectedItemPos then
			local timerRatio = self.flashTimer.ElapsedSimTimeMS/self.flashDelay;
			if timerRatio < 1 then
				PrimitiveMan:DrawPrimitives(100 * timerRatio, {CirclePrimitive(self.activity:ScreenOfPlayer(ToActor(parent):GetController().Player), self.detectedItemPos, self.detectedItemRadius * timerRatio, 188)});
			else
				self.detectedItemPos = nil;
			end
		end
	end
end

function OnFire(self)
	local unseenResolution = SceneMan:GetUnseenResolution(self.Team);
	local unseenArea = unseenResolution.X * unseenResolution.Y;
	local beamFragment = 400/unseenArea;
	if beamFragment >= 200 then
		beamFragment = 4
	elseif beamFragment >= 81 then
		beamFragment = 3
	elseif beamFragment >= 16 then
		beamFragment = 2
	else
		beamFragment = 1
	end
	local scanSpacing = math.max(unseenResolution.Smallest, 4);
	local traceBaseX = self.FlipFactor * math.cos(self.RotAngle);
	local traceBaseY = -self.FlipFactor * math.sin(self.RotAngle);
	local checkPosOriginX = self.MuzzlePos.X;
	local checkPosOriginY = self.MuzzlePos.Y;
	for o = 1, beamFragment do
		local signalStrength = self.signalStrength;
		local beamPower = beamFragment;
		local angleVariance = self.scanSpreadAngle * (0.5 - math.random());
		for i = 1, self.maxScanRange/scanSpacing do
			local checkPosX = checkPosOriginX + traceBaseX * i * scanSpacing * math.cos(angleVariance) - traceBaseY * i * scanSpacing * math.sin(angleVariance);
			local checkPosY = checkPosOriginY + traceBaseY * i * scanSpacing * math.cos(angleVariance) + traceBaseX * i * scanSpacing * math.sin(angleVariance);
			if SceneMan.SceneWrapsX then
				if checkPosX > SceneMan.SceneWidth then
					checkPosX = checkPosX - SceneMan.SceneWidth;
				elseif checkPosX < 0 then
					checkPosX = checkPosX + SceneMan.SceneWidth;
				end
			end
			
			local materialIntegrity = SceneMan:GetMaterialFromID(SceneMan:GetTerrMatter(checkPosX, checkPosY)).StructuralIntegrity;
			if materialIntegrity == 0 then materialIntegrity = 50; end
			signalStrength = signalStrength - (self.signalDecrement + materialIntegrity) * scanSpacing;
			if signalStrength < 0 then
				break;
			end

			if SceneMan:IsUnseen(checkPosX, checkPosY, self.Team) then
				for _ = 1, (beamFragment^2) do
					SceneMan:RevealUnseen(checkPosX + math.random(-(beamFragment^2), (beamFragment^2)), checkPosY + math.random(-(beamFragment^2), (beamFragment^2)), self.Team);
				end
				beamPower = beamPower - 1;
				if beamPower <= 0 then
					break;
				end
			end
		end
	end

	if self.detectedItemPos == nil then
		local mo = MovableMan:GetMOFromID(SceneMan:CastMORay(self.MuzzlePos, Vector(traceBaseX, traceBaseY), self:GetRootParent().ID, Activity.NOTEAM, rte.airID, true, 1));
		if mo and IsMOSRotating(mo) then
			self.flashTimer:Reset();
			self.detectedItemPos = mo:GetRootParent().Pos;
			self.detectedItemRadius = 5 + mo:GetRootParent().Radius;
			self.flashDelay = 100 + 100 * math.sqrt(self.detectedItemRadius);
		end
	end
end