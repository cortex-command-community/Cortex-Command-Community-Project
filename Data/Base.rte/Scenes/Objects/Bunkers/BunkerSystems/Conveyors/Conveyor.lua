function Create(self)
	x = self;
	--Set the speed in pixels per frame at which to move the actors/items.
	self.speed = 2;
end

--[[function Destroy(self)
	--Fling the actors and items in the conveyor's zone with great force in the wrong direction upon the conveyor's destruction.
	if (math.floor(math.deg(self.RotAngle)) == 0) then
		for actor in MovableMan.Actors do
			if (actor.Pos.X >= self.Pos.X - 24) and (actor.Pos.X <= self.Pos.X + 24) and (actor.Pos.Y >= self.Pos.Y - 25) and (actor.Pos.Y <= self.Pos.Y + 25) and (actor.PinStrength <= 0) then
			actor.Vel.X = -25;
			end
		end
		
		for item in MovableMan.Items do
			if (item.Pos.X >= self.Pos.X - 24) and (item.Pos.X <= self.Pos.X + 24) and (item.Pos.Y >= self.Pos.Y - 25) and (item.Pos.Y <= self.Pos.Y + 25) and (item.PinStrength <= 0) then
			item.Vel.X = -25;
			end
		end
	elseif (math.floor(math.deg(self.RotAngle)) == 270) then
		for actor in MovableMan.Actors do
			if (actor.Pos.X >= self.Pos.X - 24) and (actor.Pos.X <= self.Pos.X + 24) and (actor.Pos.Y >= self.Pos.Y - 25) and (actor.Pos.Y <= self.Pos.Y + 25) and (actor.PinStrength <= 0) then
			actor.Vel.Y = 25;
			end
		end

		for item in MovableMan.Items do
			if (item.Pos.X >= self.Pos.X - 24) and (item.Pos.X <= self.Pos.X + 24) and (item.Pos.Y >= self.Pos.Y - 25) and (item.Pos.Y <= self.Pos.Y + 25) and (item.PinStrength <= 0) then
			item.Vel.Y = 25;
			end
		end
	elseif (math.floor(math.deg(self.RotAngle)) == 90) then
		for actor in MovableMan.Actors do
			if (actor.Pos.X >= self.Pos.X - 24) and (actor.Pos.X <= self.Pos.X + 24) and (actor.Pos.Y >= self.Pos.Y - 25) and (actor.Pos.Y <= self.Pos.Y + 25) and (actor.PinStrength <= 0) then
			actor.Vel.X = 25;
			end
		end

		for item in MovableMan.Items do
			if (item.Pos.X >= self.Pos.X - 24) and (item.Pos.X <= self.Pos.X + 24) and (item.Pos.Y >= self.Pos.Y - 25) and (item.Pos.Y <= self.Pos.Y + 25) and (item.PinStrength <= 0) then
			item.Vel.X = 25;
			end
		end
	elseif (math.floor(math.deg(self.RotAngle)) == 180) then
		for actor in MovableMan.Actors do
			if (actor.Pos.X >= self.Pos.X - 24) and (actor.Pos.X <= self.Pos.X + 24) and (actor.Pos.Y >= self.Pos.Y - 25) and (actor.Pos.Y <= self.Pos.Y + 25) and (actor.PinStrength <= 0) then
			actor.Vel.Y = -25;
			end
		end

		for item in MovableMan.Items do
			if (item.Pos.X >= self.Pos.X - 24) and (item.Pos.X <= self.Pos.X + 24) and (item.Pos.Y >= self.Pos.Y - 25) and (item.Pos.Y <= self.Pos.Y + 25) and (item.PinStrength <= 0) then
			item.Vel.Y = -25;
			end
		end
	end
end--]]

function Update(self)
	--Depending on the angle, the conveyor must push objects in a different direction.  self.RotAngle is the angle, but it's in radians rather than degreees.  So, we convert and then round it down.
	if (math.floor(math.deg(self.RotAngle)) == 270) then
		--Cycle through every actor on the level.
		for actor in MovableMan.Actors do
			--Check if the actor is within the conveyor's zone.
			if (actor.Pos.X >= self.Pos.X - 24) and (actor.Pos.X <= self.Pos.X + 24) and (actor.Pos.Y >= self.Pos.Y - 25) and (actor.Pos.Y <= self.Pos.Y + 25) and (actor.PinStrength <= 0) then
			--Move the actor along at the speed defined in the create event.
			actor.Pos.X = actor.Pos.X + self.speed;
			--This decreases the effects of gravity to nearly none and allows for the up-and-down motion to be applied to center the actor.
			actor.Vel.Y = actor.Vel.Y - ((actor.Pos.Y - self.Pos.Y)) / 35;
			--This keeps the actor at a steady speed in the direction that the conveyor is pushing.
			actor.Vel.X = actor.Vel.X / 2;
			--This keeps the actor near the center of the module while still allowing the actor to leave the flow by using its jetpack.
			actor.Pos.Y = ((actor.Pos.Y * 24) + self.Pos.Y) / 25;
			end
		end

		--Repeat the last actions for all items in-game.
		for item in MovableMan.Items do
			if (item.Pos.X >= self.Pos.X - 24) and (item.Pos.X <= self.Pos.X + 24) and (item.Pos.Y >= self.Pos.Y - 25) and (item.Pos.Y <= self.Pos.Y + 25) and (item.PinStrength <= 0) then
			item.Pos.X = item.Pos.X + self.speed;
			item.Vel.Y = item.Vel.Y - ((item.Pos.Y - self.Pos.Y)) / 35;
			item.Vel.X = item.Vel.X / 2;
			item.Pos.Y = ((item.Pos.Y * 24) + self.Pos.Y) / 25;
			end
		end
	elseif (math.floor(math.deg(self.RotAngle)) == 0) then --Continue through every other possible right angle.
		for actor in MovableMan.Actors do
			if (actor.Pos.X >= self.Pos.X - 25) and (actor.Pos.X <= self.Pos.X + 25) and (actor.Pos.Y >= self.Pos.Y - 24) and (actor.Pos.Y <= self.Pos.Y + 24) and (actor.PinStrength <= 0) then
			actor.Pos.Y = actor.Pos.Y - self.speed;
			actor.Vel.X = actor.Vel.X - ((actor.Pos.X - self.Pos.X)) / 35;
			actor.Vel.Y = actor.Vel.Y / 2;
			actor.Pos.X = ((actor.Pos.X * 24) + self.Pos.X) / 25;
			end
		end

		for item in MovableMan.Items do
			if (item.Pos.X >= self.Pos.X - 25) and (item.Pos.X <= self.Pos.X + 25) and (item.Pos.Y >= self.Pos.Y - 24) and (item.Pos.Y <= self.Pos.Y + 24) and (item.PinStrength <= 0) then
			item.Pos.Y = item.Pos.Y - self.speed;
			item.Vel.X = item.Vel.X - ((item.Pos.X - self.Pos.X)) / 35;
			item.Vel.Y = item.Vel.Y / 2;
			item.Pos.X = ((item.Pos.X * 24) + self.Pos.X) / 25;
			end
		end
	elseif (math.floor(math.deg(self.RotAngle)) == 90) then
		for actor in MovableMan.Actors do
			if (actor.Pos.X >= self.Pos.X - 24) and (actor.Pos.X <= self.Pos.X + 24) and (actor.Pos.Y >= self.Pos.Y - 25) and (actor.Pos.Y <= self.Pos.Y + 25) and (actor.PinStrength <= 0) then
			actor.Pos.X = actor.Pos.X - self.speed;
			actor.Vel.Y = actor.Vel.Y - ((actor.Pos.Y - self.Pos.Y)) / 35;
			actor.Vel.X = actor.Vel.X / 2;
			actor.Pos.Y = ((actor.Pos.Y * 24) + self.Pos.Y) / 25;
			end
		end

		for item in MovableMan.Items do
			if (item.Pos.X >= self.Pos.X - 24) and (item.Pos.X <= self.Pos.X + 24) and (item.Pos.Y >= self.Pos.Y - 25) and (item.Pos.Y <= self.Pos.Y + 25) and (item.PinStrength <= 0) then
			item.Pos.X = item.Pos.X - self.speed;
			item.Vel.Y = item.Vel.Y - ((item.Pos.Y - self.Pos.Y)) / 35;
			item.Vel.X = item.Vel.X / 2;
			item.Pos.Y = ((item.Pos.Y * 24) + self.Pos.Y) / 25;
			end
		end
	elseif (math.floor(math.deg(self.RotAngle)) == 180) then
		for actor in MovableMan.Actors do
			if (actor.Pos.X >= self.Pos.X - 25) and (actor.Pos.X <= self.Pos.X + 25) and (actor.Pos.Y >= self.Pos.Y - 24) and (actor.Pos.Y <= self.Pos.Y + 24) and (actor.PinStrength <= 0) then
			actor.Pos.Y = actor.Pos.Y + self.speed;
			actor.Vel.X = actor.Vel.X - ((actor.Pos.X - self.Pos.X)) / 35;
			actor.Vel.Y = actor.Vel.Y / 2;
			actor.Pos.X = ((actor.Pos.X * 24) + self.Pos.X) / 25;
			end
		end

		for item in MovableMan.Items do
			if (item.Pos.X >= self.Pos.X - 25) and (item.Pos.X <= self.Pos.X + 25) and (item.Pos.Y >= self.Pos.Y - 24) and (item.Pos.Y <= self.Pos.Y + 24) and (item.PinStrength <= 0) then
			item.Pos.Y = item.Pos.Y + self.speed;
			item.Vel.X = item.Vel.X - ((item.Pos.X - self.Pos.X)) / 35;
			item.Vel.Y = item.Vel.Y / 2;
			item.Pos.X = ((item.Pos.X * 24) + self.Pos.X) / 25;
			end
		end
	end
end