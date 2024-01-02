function BlastRunnerDetonateCall(pieMenuOwner, pieMenu, pieSlice)
	local explosion = CreateMOSRotating("Particle Blast Runner Explosion", "Coalition.rte");
	explosion.Pos = pieMenuOwner.Pos;
	MovableMan:AddParticle(explosion);
	pieMenuOwner.ToDelete = true;
	explosion:GibThis();
end