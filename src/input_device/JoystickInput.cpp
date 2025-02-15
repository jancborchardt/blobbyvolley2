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
#include "InputDevice.h"

/* other includes */
#include "JoystickPool.h"

// ********************************************************************************************************
// 		Interface Definition
// ********************************************************************************************************

/*! \class JoystickInputDevice
	\brief Ingame Joystick input
*/
class JoystickInputDevice : public InputDevice
{
	private:
		bool getAction(const JoystickAction& action);

		JoystickAction mLeftAction;
		JoystickAction mRightAction;
		JoystickAction mJumpAction;
	public:
		~JoystickInputDevice() override = default;
		JoystickInputDevice(JoystickAction laction, JoystickAction raction,
				JoystickAction jaction);

		PlayerInputAbs transferInput() override;
};

// ********************************************************************************************************
// 		Class Implementation
// ********************************************************************************************************

// -------------------------------------------------------------------------------------------------
//		Creator Function
// -------------------------------------------------------------------------------------------------

InputDevice* createJoystrickInput(JoystickAction left, JoystickAction right, JoystickAction jump)
{
	return new JoystickInputDevice(left, right, jump);
}

// -------------------------------------------------------------------------------------------------
// 		Joystick Input Device
// -------------------------------------------------------------------------------------------------

JoystickInputDevice::JoystickInputDevice(JoystickAction laction, JoystickAction raction, JoystickAction jaction)
	: mLeftAction(laction), mRightAction(raction), mJumpAction(jaction)
{
}

PlayerInputAbs JoystickInputDevice::transferInput()
{
	return { getAction(mLeftAction), getAction(mRightAction), getAction(mJumpAction) };
}

bool JoystickInputDevice::getAction(const JoystickAction& action)
{
	if (action.joy != nullptr)
	{
		switch (action.type)
		{
			case JoystickAction::AXIS:
				if (action.number < 0)
				{
					if (SDL_JoystickGetAxis(action.joy,
						-action.number - 1) < -15000)
						return true;
				}
				else if (action.number > 0)
				{
					if (SDL_JoystickGetAxis(action.joy,
						action.number - 1) > 15000)
						return true;
				}
				break;

			case JoystickAction::BUTTON:
				if (SDL_JoystickGetButton(action.joy,
							action.number))
					return true;
				break;
			default:
				break;
		}
	}
	return false;
}
