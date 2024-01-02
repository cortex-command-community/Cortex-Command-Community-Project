function Create(self)
	self.TurnofTimer = Timer()
end

function Update(self)
	if self.TurnofTimer:IsPastSimMS(6000) then
		self:EnableEmission(false)
		self.TurnofTimer:Reset()
	end
end
