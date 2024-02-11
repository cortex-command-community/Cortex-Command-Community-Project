require("AI/NativeTurretAI");

function Create(self)

	self.AI = NativeTurretAI:Create(self);
end

function ThreadedUpdateAI(self)
	self.AI:Update(self);
end

function Destroy(self)
	self.AI:Destroy(self);
end
