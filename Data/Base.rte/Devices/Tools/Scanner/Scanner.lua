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
	local signalStrength = self.signalStrength;
	local angleVariance = self.scanSpreadAngle * 0.5 - (math.random() * self.scanSpreadAngle);
	local unseenResolution = SceneMan:GetUnseenResolution(-1);
	local scanSpacing = math.max(math.min(unseenResolution.X, unseenResolution.Y) * 0.5, 1);
	local trace = Vector(self.FlipFactor, 0):RadRotate(self.RotAngle);
	local checkPos = Vector(self.MuzzlePos.X, self.MuzzlePos.Y);
	for i = 1, self.maxScanRange/scanSpacing do
		trace = Vector(trace.X, trace.Y):SetMagnitude(i * scanSpacing);
		checkPos = self.MuzzlePos + Vector(trace.X, trace.Y):RadRotate(angleVariance);
		if SceneMan.SceneWrapsX then
			if checkPos.X > SceneMan.SceneWidth then
				checkPos = Vector(checkPos.X - SceneMan.SceneWidth, checkPos.Y);
			elseif checkPos.X < 0 then
				checkPos = Vector(checkPos.X + SceneMan.SceneWidth, checkPos.Y);
			end
		end
		
		signalStrength = signalStrength - (self.signalDecrement + SceneMan:GetMaterialFromID(SceneMan:GetTerrMatter(checkPos.X, checkPos.Y)).StructuralIntegrity) * scanSpacing;
		if signalStrength < 0 then
			break;
		end

		if SceneMan:IsUnseen(checkPos.X, checkPos.Y, self.Team) then
			SceneMan:RevealUnseen(checkPos.X, checkPos.Y, self.Team);
			break;
		end
	end

	if self.detectedItemPos == nil then
		local mo = MovableMan:GetMOFromID(SceneMan:CastMORay(self.MuzzlePos, trace, self:GetRootParent().ID, Activity.NOTEAM, rte.airID, true, 1));
		if mo and IsMOSRotating(mo) then
			self.flashTimer:Reset();
			self.detectedItemPos = mo:GetRootParent().Pos;
			self.detectedItemRadius = 5 + mo:GetRootParent().Radius;
			self.flashDelay = 100 + 100 * math.sqrt(self.detectedItemRadius);
		end
	end
end