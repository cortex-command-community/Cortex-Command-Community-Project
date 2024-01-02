require("AI/NativeHumanAI");

function Create(self)
	self.AI = NativeHumanAI:Create(self);
end
function ThreadedUpdateAI(self)
	self.AI:Update(self);
end
function Destroy(self)
	self.AI:Destroy(self);
end