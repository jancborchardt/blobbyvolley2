/*=============================================================================
Blobby Volley 2
Copyright (C) 2006 Jonathan Sieber (jonathan_sieber@yahoo.de)
Copyright (C) 2006 Daniel Knobe (daniel-knobe@web.de)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

/* header include */
#include "PlayerInput.h"

/* includes */
#include <ostream>
#include <cassert>

#include "raknet/BitStream.h"

#include "DuelMatch.h"
#include "GameConstants.h"

/* implementation */

/* PlayerInput */
void PlayerInput::setAll( unsigned char all )
{
	left = all & 4;
	right = all & 2;
	up =  all & 1;
}

bool PlayerInput::operator==(const PlayerInput& other) const
{
	return left == other.left && right == other.right && up == other.up;
}

unsigned char PlayerInput::getAll() const
{
	unsigned char c = 0;
	c = (left ? 4 : 0) + (right ? 2 : 0) + (up ? 1 : 0);
	return c;
}

/* PlayerInputAbs */

PlayerInputAbs::PlayerInputAbs() : mFlags( F_RELATIVE ), mTarget(-1)
{

}

PlayerInputAbs::PlayerInputAbs(RakNet::BitStream& stream)
{
	stream.Read( mFlags );
	stream.Read( mTarget );
}

PlayerInputAbs::PlayerInputAbs(bool l, bool r, bool j) : mFlags( F_RELATIVE ), mTarget(-1)
{
	setLeft(l);
	setRight(r);
	setJump(j);
}


// set input
void PlayerInputAbs::setLeft( bool v )
{
	if(v)
		mFlags |= F_LEFT;
	else
		mFlags &= ~F_LEFT;
}

void PlayerInputAbs::setRight( bool v )
{
	if(v)
		mFlags |= F_RIGHT;
	else
		mFlags &= ~F_RIGHT;
}

void PlayerInputAbs::setJump( bool v)
{
	if(v)
		mFlags |= F_JUMP;
	else
		mFlags &= ~F_JUMP;
}

void PlayerInputAbs::setTarget( short target, PlayerSide player )
{
	mFlags &= F_JUMP;	// reset everything but the jump flag, i.e. no left/right and no relative
	mTarget = target;

	if(player == LEFT_PLAYER )
	{
		setLeft(true);
	}
	if(player == RIGHT_PLAYER )
	{
		setRight(true);
	}
}

void PlayerInputAbs::swapSides()
{
	bool left = mFlags & F_LEFT;
	bool right = mFlags & F_RIGHT;

	setLeft(right);
	setRight(left);

	mTarget = RIGHT_PLANE - mTarget;
}

PlayerInput PlayerInputAbs::toPlayerInput( const DuelMatch* match ) const
{
	if( mFlags & F_RELATIVE)
		return PlayerInput( mFlags & F_LEFT, mFlags & F_RIGHT, mFlags & F_JUMP );
	else
	{
		bool left = false;
		bool right = false;

		PlayerSide side = mFlags & F_LEFT ? LEFT_PLAYER : RIGHT_PLAYER;

		// here we load the current position of the player.
		float blobpos = match->getBlobPosition(side).x;

		float distance = std::abs(blobpos - mTarget);

		if ( std::abs(blobpos + BLOBBY_SPEED - mTarget) < distance )
			right = true;
		else if (std::abs(blobpos - BLOBBY_SPEED - mTarget) < distance)
			left = true;
		return PlayerInput( left, right, mFlags & F_JUMP );
	}

}

void PlayerInputAbs::writeTo(RakNet::BitStream& stream) const
{
	stream.Write( mFlags );
	stream.Write( mTarget );
}


std::ostream& operator<< (std::ostream& out, const PlayerInput& input)
{
	out << (input.left ? 't' : 'f') << (input.right ? 't' : 'f') << (input.up ? 't' : 'f');
	return out;
}
