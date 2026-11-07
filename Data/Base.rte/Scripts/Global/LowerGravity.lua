-- A scene's gravity is saved with the game, so what it was before this changed it has to be too.
function LowerGravityScript:StartScript()
	self:Apply();
end

function LowerGravityScript:EndScript()
	local activity = ActivityMan:GetActivity();
	if activity and SceneMan.Scene then
		local originalGravity = activity:LoadNumber("LowerGravityScript_OriginalGravity");
		if originalGravity ~= 0 then
			SceneMan.Scene.GlobalAcc = Vector(SceneMan.Scene.GlobalAcc.X, originalGravity);
			activity:SaveNumber("LowerGravityScript_OriginalGravity", 0);
		end
	end
end

function LowerGravityScript:UpdateScript()
	self:Apply();
end

function LowerGravityScript:Apply()
	local activity = ActivityMan:GetActivity();
	if activity and SceneMan.Scene and activity:LoadNumber("LowerGravityScript_OriginalGravity") == 0 then
		local gravity = SceneMan.Scene.GlobalAcc;
		activity:SaveNumber("LowerGravityScript_OriginalGravity", gravity.Y);
		SceneMan.Scene.GlobalAcc = Vector(gravity.X, gravity.Y * 0.50);
	end
end
