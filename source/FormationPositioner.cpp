/* FormationPositioner.cpp
Copyright (c) 2019 by Peter van der Meer

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "FormationPositioner.h"

#include "Angle.h"
#include "Point.h"

using namespace std;



void FormationPositioner::Start()
{
	int startRing = 0;
	for(auto &it : ringPos)
	{
		// Track in which ring the ring-positioner ended in last run.
		int endRing = it.second.ring;
		// Set starting ring for current ring-positioner.
		it.second.ring = max(startRing, it.first);
		// Store starting ring for next ring-positioner.
		startRing = endRing + 1;
		
		// Reset all other iterator values to start of ring.
		it.second.activeLine = 0;
		it.second.lineSlot = 0;
		it.second.lineSlots = -1;
	}
}



Point FormationPositioner::NextPosition(int minimumRing)
{
	// Retrieve the correct ring-positioner.
	RingPositioner &rPos = ringPos[minimumRing];
	
	// If there are no active lines, then just return center point.
	if(rPos.activeLine < 0)
		return Point();
	
	// Handle trigger to initialize lineSlots if required.
	if(rPos.lineSlots < 0)
		rPos.lineSlots = pattern->LineSlots(rPos.ring, rPos.activeLine);
	
	// Iterate to next line if the current line is full.
	if(rPos.lineSlot >= rPos.lineSlots)
	{
		int nextLine = pattern->NextLine(rPos.ring, rPos.activeLine);
		// If no new active line, just return center point.
		if(nextLine < 0)
		{
			rPos.activeLine = -1;
			return Point();
		}
		// If we get back to an earlier line, then we moved a ring up.
		if(nextLine <= rPos.activeLine)
			rPos.ring++;
		
		rPos.lineSlot = 0;
		rPos.activeLine = nextLine;
		rPos.lineSlots = pattern->LineSlots(rPos.ring, rPos.activeLine);
	}
	
	Point relPos = pattern->Position(rPos.ring, rPos.activeLine, rPos.lineSlot) * rPos.activeScalingFactor;
	// Set values for next ring.
	rPos.lineSlot++;

	// Calculate new direction, if the formationLead is moving, then we use the movement vector.
	// Otherwise we use the facing vector.
	Point velocity = formationLead->Velocity();
	Angle direction;
	if(velocity.Length() > 0.1)
		direction = Angle(velocity);
	else
		direction = formationLead->Facing();

	return formationLead->Position() + direction.Rotate(relPos);
}
