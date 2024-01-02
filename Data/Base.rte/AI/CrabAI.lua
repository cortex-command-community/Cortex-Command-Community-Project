require("AI/NativeCrabAI");

function Create(self)
	self.AI = NativeCrabAI:Create(self);
end

function ThreadedUpdateAI(self)
	self.AI:Update(self);
end

function Destroy(self)
	self.AI:Destroy(self);
end
