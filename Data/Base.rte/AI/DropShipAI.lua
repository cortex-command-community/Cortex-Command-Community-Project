require("AI/NativeDropShipAI");

function Create(self)
	self.AI = NativeDropShipAI:Create(self);
end

function ThreadedUpdateAI(self)
	self.AI:Update(self);
end