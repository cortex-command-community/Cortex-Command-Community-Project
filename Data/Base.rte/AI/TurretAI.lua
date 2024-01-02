require("AI/NativeTurretAI"); --dofile("Base.rte/Actors/AI/NativeTurretAI.lua");

function Create(self)
	self.AI = NativeTurretAI:Create(self);
end

function UpdateAI(self)
	self.AI:Update(self);
end

function Destroy(self)
	self.AI:Destroy(self);
end
