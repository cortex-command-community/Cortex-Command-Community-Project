function LowerGravityScript:StartScript()
	SceneMan.Scene.GlobalAcc = Vector(SceneMan.Scene.GlobalAcc.X, SceneMan.Scene.GlobalAcc.Y * 0.50);
	self:Deactivate();
end