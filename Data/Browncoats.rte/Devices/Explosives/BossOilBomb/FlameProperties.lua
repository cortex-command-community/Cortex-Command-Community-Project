function Create(self)

	-- extra hurty particle toggle when on ground/without target
	self.extraParticles = true;
	
	-- team awareness toggle... friendly fire hahahahahahaah
	self.teamAware = true;
	
	self.flameLingerChance = 0.9;
	
	-- i am not sure why this grass interaction occurs, but i'm not going to remove it, Chesterton's Fence and all
	self.grassInteraction = false;

end
