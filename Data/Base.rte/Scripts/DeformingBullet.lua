---- TO USE:
-- Simply add the script to any MO capable of wounding (most commonly MOPixel bullets).
-- Example: ScriptPath = Base.rte/Scripts/DeformingBullet.lua

-- Min/max radius of the entry wound hole, including discoloured outer ring
local entryWoundRadius = {1, 2};

-- Min/max radius of the exit wound hole, including discoloured outer ring
local exitWoundRadius = {2, 3};

-- Whether or not the wounds should count towards GibWoundLimit of the MOSR; mostly for testing
local countTowardsWoundLimit = true;

-- How much to multiply the sharpness by for MOSR collisions only
local sharpnessMultiplier = 1;

function Create(self)
	local var = {};
	var.Pos = self.Pos;
	var.Vel = self.Vel;
	var.Sharpness = self.Sharpness;
	var.ringPositions = {};
	var.canPenetrate = true;
	var.newPos = nil;
	var.newVel = nil;
	var.numberOfHits = 0;
	self.Sharpness = -math.abs(self.Sharpness); -- Set sharpness value to be negative to preserve terrain destruction
	self.var = var;
end

-- Returns a table with all unique colour indexes of the sprite, except transparency.
-- Used for the discoloured outer ring of the wound holes.
local function GetAllSpriteColors(MOSprite)
	if (MOSprite ~= nil) then
		local spriteSize = Vector(MOSprite:GetSpriteWidth()-1, MOSprite:GetSpriteHeight()-1);
		local colorTable = {};
		local colorCount = 0;
		for y = 0, spriteSize.Y do
			for x = 0, spriteSize.X do
				local pixelColor = MOSprite:GetSpritePixelIndex(x, y, MOSprite.Frame);
				if (pixelColor > 0) then
					if (colorCount == 0) then
						colorCount = colorCount + 1;
						colorTable[colorCount] = pixelColor;
					else
						local i = 0;
						local colorFound = false;
						repeat
							i = i + 1;
							colorFound = pixelColor == colorTable[i];
						until colorFound == true or i >= colorCount
						
						if (colorFound == false) then
							colorCount = colorCount + 1;
							colorTable[colorCount] = pixelColor;
						end
					end
				end
			end
		end
		
		return colorTable;
	else
		return {};
	end
end

-- Adds a given wound with accompanying hole in the sprite
local function addDeformWound(var, MO, radiusTable, rangeVector, absWoundPos, angleOffset, woundPresetName)
	local MOSprite = ToMOSprite(MO);
	local holeRadius = math.random(radiusTable[1], radiusTable[2]);
	local woundEmitterOffset = Vector(holeRadius, 0):GetRadRotatedCopy(rangeVector.AbsRadAngle + angleOffset); -- Vector to push the created wound in from the new hole
	local holeOffset = SceneMan:ShortestDistance(MO.Pos, absWoundPos, true);
	local woundOffset = holeOffset + woundEmitterOffset; -- Push the wound MO inwards to make it visually spawn on the MO rather than thin air
	local holePos = MOSprite:UnRotateOffset(holeOffset);
	local woundPos = MOSprite:UnRotateOffset(woundOffset);
	
	-- Creates the wound at the default position if the presetname exists; script might bork if no wound is given
	local newWound = nil;
	if (woundPresetName ~= "") then
		newWound = CreateAEmitter(woundPresetName);
		local inboundAngle = rangeVector:GetXFlipped(MO.HFlipped).AbsRadAngle;
		local woundAngle = inboundAngle - (MO.RotAngle * MO.FlipFactor) + math.pi + angleOffset; -- ... We should probably have an MOSprite:UnRotateAngle() function
		-- newWound.Lifetime = 50;
		-- newWound.BurstDamage = 0;
		MO:AddWound(newWound, woundPos, countTowardsWoundLimit);
		newWound.InheritedRotAngleOffset = woundAngle;
	end
	
	-- Makes a hole in the sprite, discolouring the outermost pixels instead of removing them.
	-- Iterates radially, could be made into a square with a distance check if coverage is spotty.
	for i = 0, holeRadius do
		local circumference = holeRadius * 2 * math.pi;
		local angleStep = (math.pi*2)/circumference;
		for q = 1, circumference do
			local pos = Vector(i, 0):GetRadRotatedCopy(angleStep*q).Ceilinged + (holePos - MOSprite.SpriteOffset);
			local color = 0; -- Default hole colour is transparent

			-- If we're at the edge of the hole and the wound has any colours, set pixel colour to a random wound colour instead of transparent
			if (i == holeRadius and IsMOSprite(newWound)) then
				local colorTable = GetAllSpriteColors(ToMOSprite(newWound));
				if (#colorTable > 0) then
					color = colorTable[math.random(1, #colorTable)];
				end
			end

			-- Change pixel colour on all frames of the sprite and, if we're at the edge, make a table of all valid positions on the outer ring
			for frame = 0, MOSprite.FrameCount do
				if (MOSprite:SetSpritePixelIndex(pos.X, pos.Y, frame, color, 0, false) and i == holeRadius) then
					table.insert(var.ringPositions, pos + MOSprite.SpriteOffset);
				end
			end
		end
	end
	
	-- Attempts to displace all wound MOs within the radius to the edge of it
	for wound in MO:GetWounds() do
		local woundDist = wound.ParentOffset - holePos;
		if (woundDist.Magnitude < holeRadius) then
			-- Calculate a vector from hole centre to wound position and set it to equal the radius of the hole, pushing the wound out to the edge
			local newDist = Vector(woundDist.X, woundDist.Y);
			local newOffset = holePos + newDist:SetMagnitude(holeRadius);
			local bitmapOffset = newOffset - MOSprite.SpriteOffset;
			-- If the calculated position isn't transparent, set parentoffset to this
			if (MOSprite:GetSpritePixelIndex(bitmapOffset.X, bitmapOffset.Y, MOSprite.Frame) == -2) then
				wound.ParentOffset = newOffset;
			else
				-- If calculated position was invalid, pick a random position on the outside ring
				if (#var.ringPositions > 0) then
					local pos;
					local bitmapPos;
					local foundPixel = false;
					repeat
						pos = table.remove(var.ringPositions, math.random(1, #var.ringPositions));
						bitmapPos = pos - MOSprite.SpriteOffset;
						foundPixel = MOSprite:GetSpritePixelIndex(bitmapPos.X, bitmapPos.Y, MOSprite.Frame) > 0;
					until
						#var.ringPositions <= 0 or foundPixel
					
					if (foundPixel) then
						wound.ParentOffset = pos;
					else
						-- If, somehow, no valid position is found, delete the wound; this might need changing but is an edge case
						wound.ToDelete = true;
					end
				else
					-- If there are no outer ring positions, delete the wound
					wound.ToDelete = true;
				end
			end
		end
	end
	
	return newWound;
end

function OnCollideWithMO(self, hitMO, hitMORootParent)
	local var = self.var;
	
	-- Calculate MOSR penetration power
	local penetration = self.Mass * var.Sharpness * var.Vel.Magnitude * sharpnessMultiplier;

	-- If the target isn't about to cease existing, the bullet hasn't penetrated this frame and the material of the MO is weak enough to penetrate, proceed
	if hitMO.ToDelete == false and var.canPenetrate and hitMO.Material.StructuralIntegrity <= penetration then
		var.canPenetrate = false; -- Ensure this is only run once per frame
		local rangeVector = var.Vel/3;
		local endPos = var.Pos + rangeVector;
		
		-- We do already have the MO but we need the point of impact
		local raycast = SceneMan:CastMORay(var.Pos, rangeVector, self.RootID, self.IgnoresWhichTeam, 0, true, 0);
		
		if raycast ~= 255 then
			endPos = SceneMan:GetLastRayHitPos(); -- Point of impact, woo
			local MO = ToMOSRotating(MovableMan:GetMOFromID(raycast));
			local MOSprite = ToMOSprite(MO);
			var.ringPositions = {}; -- Reset ring position table for this collision
			local maxPen = penetration / MO.Material.StructuralIntegrity; -- Max penetration depth
			local penVec = rangeVector.Normalized;
			local hitOffset = SceneMan:ShortestDistance(MO.Pos, endPos, true);
			
			-- Add the entry wound
			addDeformWound(var, MO, entryWoundRadius, rangeVector, endPos, 0, MO:GetEntryWoundPresetName());
			
			-- Bit of table bullshit for Lua performance; just use vectors in C++
			local startPos = {hitOffset.X, hitOffset.Y};
			local exitWoundPos = nil;
			local penVecTable = {penVec.X, penVec.Y};
			local penUsed = 0;
			local pixelFound = false;
			-- Check for exit wound
			for i = 1, maxPen do
				local checkPos = Vector(startPos[1] + penVecTable[1]*i, startPos[2] + penVecTable[2]*i);
				checkPos = MOSprite:UnRotateOffset(checkPos);
				checkPos = checkPos - MOSprite.SpriteOffset;
				local pixel = MOSprite:GetSpritePixelIndex(checkPos.X, checkPos.Y, MOSprite.Frame);
				
				-- If we've found a valid pixel and the iterator exits the visible sprite, add exit wound at last found pixel
				if (pixelFound and pixel <= 0) then
					exitWoundPos = Vector(startPos[1] + penVecTable[1]*i, startPos[2] + penVecTable[2]*i);
					pixelFound = false;
				end
				
				-- If outside of sprite dimensions, break loop
				if (pixel < 0) then
					break;
				end
				
				-- If we find a visible pixel
				if (pixel > 0) then
					penUsed = penUsed + MO.Material.StructuralIntegrity;
					pixelFound = true;
				end

				-- If all penetration has been spent, break loop
				if (penUsed >= penetration) then
					break;
				end
			end
			
			-- If a valid exit wound position has been found, add exit wound and set bullet to appear out of this wound with appropriately reduced velocity
			if (exitWoundPos) then
				local exitWound = addDeformWound(var, MO, exitWoundRadius, rangeVector, exitWoundPos + MO.Pos, math.pi, MO:GetExitWoundPresetName());
				var.newVel = rangeVector * 3 * (1-(penUsed / penetration));
				var.newPos = exitWoundPos + MO.Pos;
				self:SetWhichMOToNotHit(MO:GetRootParent(), 0.035); -- Makes sure the bullet only hits this MOSR once
			else
				self.ToDelete = true;
				var.newVel = (endPos - self.Pos) / 3; -- Attempts to prevent the bullet from visually bouncing off for one frame
			end
		end
	end
end

function Update(self)
	local var = self.var;
	var.canPenetrate = true;
	
	-- We have to set new velocities and positions in Update because it borks in OnCollideWithMO
	if (var.newVel) then
		self.Vel = Vector(var.newVel.X, var.newVel.Y);
		var.newVel = nil;
	end
	
	if (var.newPos) then
		self.Pos = Vector(var.newPos.X, var.newPos.Y);
		var.newPos = nil;
	end
end