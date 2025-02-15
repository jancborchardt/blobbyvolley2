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
#include "LocalGameState.h"

/* includes */
#include "DuelMatch.h"
#include "InputManager.h"
#include "IMGUI.h"
#include "replays/ReplayRecorder.h"
#include "SoundManager.h"
#include "SpeedController.h"
#include "IUserConfigReader.h"
#include "InputSourceFactory.h"

/* implementation */
LocalGameState::~LocalGameState() = default;

LocalGameState::LocalGameState()
	: mWinner(false), mRecorder(new ReplayRecorder())
{
	std::shared_ptr<IUserConfigReader> config = IUserConfigReader::createUserConfigReader("config.xml");
	PlayerIdentity leftPlayer = config->loadPlayerIdentity(LEFT_PLAYER, false);
	PlayerIdentity rightPlayer = config->loadPlayerIdentity(RIGHT_PLAYER, false);

	std::shared_ptr<InputSource> leftInput = InputSourceFactory::createInputSource( *config, LEFT_PLAYER);
	std::shared_ptr<InputSource> rightInput = InputSourceFactory::createInputSource( *config, RIGHT_PLAYER);

	// create default replay name
	setDefaultReplayName(leftPlayer.getName(), rightPlayer.getName());

	// set speed
	SpeedController::getMainInstance()->setGameSpeed( (float)config->getInteger("gamefps") );


	SoundManager::getSingleton().playSound("sounds/pfiff.wav", ROUND_START_SOUND_VOLUME);

	mMatch.reset(new DuelMatch( false, config->getString("rules")));
	mMatch->setPlayers(leftPlayer, rightPlayer);
	mMatch->setInputSources(leftInput, rightInput);

	mRecorder->setPlayerNames(leftPlayer.getName(), rightPlayer.getName());
	mRecorder->setPlayerColors( leftPlayer.getStaticColor(), rightPlayer.getStaticColor() );
	mRecorder->setGameSpeed((float)config->getInteger("gamefps"));
	mRecorder->setGameRules( config->getString("rules") );
}

void LocalGameState::step_impl()
{
	IMGUI& imgui = IMGUI::getSingleton();
	if(!mErrorMessage.empty())
	{
		displayErrorMessageBox();
	}
	else if (mSaveReplay)
	{
		if ( displaySaveReplayPrompt() )
		{
			saveReplay( *mRecorder );
		}
	}
	else if (mMatch->isPaused())
	{
		displayQueryPrompt(200,
			TextManager::LBL_CONF_QUIT,
			std::make_tuple(TextManager::LBL_YES, [&](){ switchState(new MainMenuState); }),
			std::make_tuple(TextManager::LBL_NO,  [&](){ mMatch->unpause(); }),
			std::make_tuple(TextManager::RP_SAVE, [&](){ mSaveReplay = true; imgui.resetSelection(); }));

		imgui.doCursor();
	}
	else if (mWinner)
	{
		displayWinningPlayerScreen( mMatch->winningPlayer() );
		if (imgui.doButton(GEN_ID, Vector2(310, 340), TextManager::LBL_OK))
		{
			switchState(new MainMenuState());
		}
		if (imgui.doButton(GEN_ID, Vector2(420, 340), TextManager::GAME_TRY_AGAIN))
		{
			switchState(new LocalGameState());
		}
		if (imgui.doButton(GEN_ID, Vector2(500, 390), TextManager::RP_SAVE, TF_ALIGN_CENTER))
		{
			mSaveReplay = true;
			imgui.resetSelection();
		}
	}
	else if (InputManager::getSingleton()->exit())
	{
		if (mSaveReplay)
		{
			mSaveReplay = false;
			IMGUI::getSingleton().resetSelection();
		}
		else if (mMatch->isPaused())
		{
			switchState(new MainMenuState);
		}
		else
		{
			RenderManager::getSingleton().redraw();
			mMatch->pause();
		}
	}
	else
	{
		mRecorder->record(mMatch->getState());
		mMatch->step();

		if (mMatch->winningPlayer() != NO_PLAYER)
		{
			mWinner = true;
			mRecorder->record(mMatch->getState());
			mRecorder->finalize( mMatch->getScore(LEFT_PLAYER), mMatch->getScore(RIGHT_PLAYER) );
		}

		presentGame();
	}

	presentGameUI();
}

const char* LocalGameState::getStateName() const
{
	return "LocalGameState";
}

