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
#include "TextManager.h"

/* includes */
#include <iostream>
#include <algorithm>
#include <set>

#include "tinyxml2.h"

#include "FileRead.h"

/* implementation */

TextManager::TextManager(std::string l): lang(std::move(l)) {
	mStrings.resize(COUNT);
	setDefault();

	std::string langfile = "lang_"+lang+".xml";

	bool loaded = false;
	try{
		loaded = loadFromXML(langfile);
	} catch(FileLoadException& fle) {
		std::cerr << fle.what() << std::endl;
	}

	if(!loaded){
		std::cerr << "error loading language " << langfile << "!" << std::endl;
		std::cerr << "\tfalling back to english" << std::endl;
	}
}

const std::string& TextManager::getString(STRING str) const{
	return mStrings[str];
}

std::string TextManager::getLang() const{
	return lang;
}

const int TextManager::getUTF8Length(const std::string& str)
{
	int len = 0;
	for(const char& character : str) {
		len += (character & 0xc0) != 0x80;
	}
	return len;
}

bool TextManager::loadFromXML(const std::string& filename){
	// read and parse file
	auto language_data = FileRead::readXMLDocument(filename);

	if (language_data->Error())
	{
		std::cerr << "Warning: Parse error in " << filename;
		std::cerr << "!" << std::endl;
	}

	const auto* language = language_data->FirstChildElement("language");
	if (!language)
		return false;

	int num_strings = mStrings.size();
	int found_count = 0;

	#ifdef DEBUG
	std::set<std::string> stringsToTranslate;
	for(std::string s: mStrings)
	{
		stringsToTranslate.insert( s );
	}
	#endif // DEBUG

	// this loop assumes that the strings in the xml file are in the correct order
	//  in each step, it reads the next string element and writes it to the next position in mStrings
	for (	const auto* stringel = language->FirstChildElement("string");
			stringel;
			stringel = stringel->NextSiblingElement("string"))

	{

		/// \todo we don't check for duplicate entries!
		const char* e = stringel->Attribute("english");
		const char* t = stringel->Attribute("translation");
		if (t && e)
		{
			#ifdef DEBUG
			// remove every found element
			stringsToTranslate.erase( std::string(e) );
			#endif // DEBUG
			// search the english string and replace it with the translation
			auto found = std::find(mStrings.begin(), mStrings.end(), e);
			if(found != mStrings.end())
			{
				found_count++;
				*found = t;
			}
			else
				std::cerr << "error in language file: entry " << e << " -> " << t << " invalid\n";

		}
		else if(t)
		{
			std::cerr << "error in language file: english not found for " << t << std::endl;
		}
		else if(e)
		{
			std::cerr << "error in language file: translation not found for " << e << std::endl;
		}

	}

	// do we check if we got all?
	if(num_strings != found_count)
	{
		std::cerr << "missing translations: got " << found_count <<
					" out of " << num_strings << " translation entries" << std::endl;

		#ifdef DEBUG
		for(auto e : stringsToTranslate)
		{
			std::cerr << " missing " << e << "\n";
		}
		#endif // DEBUG
	}

	return true;
}

void TextManager::setDefault()
{
	// Hardcoded default language
	mStrings[LBL_OK] = "ok";
	mStrings[LBL_CANCEL] = "cancel";
	mStrings[LBL_YES] = "yes";
	mStrings[LBL_NO] = "no";
	mStrings[LBL_CONF_QUIT] = "really quit?";
	mStrings[LBL_CONTINUE] = "continue";

	mStrings[MNU_LABEL_ONLINE] = "online game";
	mStrings[MNU_LABEL_LAN] = "lan game";
	mStrings[MNU_LABEL_START] = "start";
	mStrings[MNU_LABEL_OPTIONS] = "options";
	mStrings[MNU_LABEL_REPLAY] = "watch replay";
	mStrings[MNU_LABEL_CREDITS] = "credits";
	mStrings[MNU_LABEL_EXIT] = "exit";

	mStrings[CRD_PROGRAMMERS] = "programmers:";
	mStrings[CRD_GRAPHICS] = "graphics:";
	mStrings[CRD_THX] = "special thanks at:";

	mStrings[RP_SHOW_AGAIN] = "show again";
	mStrings[RP_PLAY] = "play";
	mStrings[RP_DELETE] = "delete";
	mStrings[RP_INFO] = "info";
	mStrings[RP_DURATION] = "duration:";
	mStrings[RP_RESULT] = "result:";
	mStrings[RP_CHECKSUM] = "checksum error";
	mStrings[RP_FILE_CORRUPT] = "file is corrupt";
	mStrings[RP_VERSION] = "version error";
	mStrings[RP_FILE_OUTDATED] = "file is outdated";
	mStrings[RP_SAVE_NAME] = "name of the replay:";
	mStrings[RP_WAIT_REPLAY] = "receiving replay...";
	mStrings[RP_SAVE] = "save replay";

	mStrings[GAME_WIN] = "has won the game!";
	mStrings[GAME_TRY_AGAIN] = "try again";
	mStrings[GAME_WAITING] = "waiting for opponent...";
	mStrings[GAME_OPP_LEFT] = "opponent left the game";
	mStrings[GAME_PAUSED] = "game paused";
	mStrings[GAME_QUIT] = "quit";

	mStrings[NET_SERVER_SCAN] = "scan for servers";
	mStrings[NET_DIRECT_CONNECT] = "direct connect";
	mStrings[NET_SERVER_INFO] = "server info";
	mStrings[NET_ACTIVE_GAMES] = "games: ";
	mStrings[NET_WAITING_PLAYER] = "waiting player: ";
	mStrings[NET_HOST_GAME] = "host game";
	mStrings[NET_CONNECTING] = "connecting to server ...";
	mStrings[NET_DISCONNECT] = "disconnected from server";
	mStrings[NET_CON_FAILED] = "connection failed";
	mStrings[NET_SERVER_FULL] = "server full";
	mStrings[NET_STAY_ON_SERVER] = "stay on server";
	mStrings[NET_RANDOM_OPPONENT] = "random";
	mStrings[NET_OPEN_GAME] = "open game";
	mStrings[NET_JOIN] = "join game";
	mStrings[NET_LEAVE] = "leave game";
	mStrings[NET_SPEED] = "speed: ";
	mStrings[NET_POINTS] = "points: ";
	mStrings[NET_RULES_TITLE] = "rules: ";
	mStrings[NET_RULES_BY] = " by ";
	mStrings[NET_CHALLENGER] = "challenger: ";

	mStrings[OP_TOUCH_TYPE] = "touch input type:";
	mStrings[OP_TOUCH_ARROWS] = "arrow keys";
	mStrings[OP_TOUCH_DIRECT] = "blobby follows finger";
	mStrings[OP_INPUT_OP] = "input options";
	mStrings[OP_GFX_OP] = "graphic options";
	mStrings[OP_MISC] = "misc options";
	mStrings[OP_VIDEO] = "video settings";
	mStrings[OP_FULLSCREEN] = "fullscreen mode";
	mStrings[OP_WINDOW] = "window mode";
	mStrings[OP_RENDER_DEVICE] = "render device";
	mStrings[OP_SHOW_SHADOW] = "show shadow";
	mStrings[OP_BLOB_COLORS] = "blob colors";
	mStrings[OP_LEFT_PLAYER] = "left player";
	mStrings[OP_RIGHT_PLAYER] = "right player";
	mStrings[OP_RED] = "red";
	mStrings[OP_GREEN] = "green";
	mStrings[OP_BLUE] = "blue";
	mStrings[OP_MORPHING] = "morphing blob?";
	mStrings[OP_KEYBOARD] = "keyboard";
	mStrings[OP_MOUSE] = "mouse";
	mStrings[OP_JOYSTICK] = "joystick";
	mStrings[OP_JUMP_BUTTON] = "jump button";
	mStrings[OP_SET_ALL] = "set all";
	mStrings[OP_LEFT_KEY] = "left key";
	mStrings[OP_RIGHT_KEY] = "right key";
	mStrings[OP_JUMP_KEY] = "jump key";
	mStrings[OP_LEFT_BUTTON] = "left button";
	mStrings[OP_RIGHT_BUTTON] = "right button";
	mStrings[OP_PRESS_MOUSE_BUTTON] = "press mouse button for";
	mStrings[OP_PRESS_KEY_FOR] = "press key for";
	mStrings[OP_MOVING_LEFT] = "moving left";
	mStrings[OP_MOVING_RIGHT] = "moving right";
	mStrings[OP_JUMPING] = "jumping";
	mStrings[OP_PRESS_BUTTON_FOR] = "press button for";
	mStrings[OP_BACKGROUND] = "background:";
	mStrings[OP_VOLUME] = "volume:";
	mStrings[OP_MUTE] = "mute";
	mStrings[OP_FPS] = "show fps";
	mStrings[OP_BLOOD] = "show blood";
	mStrings[OP_NETWORK_SIDE] = "network side:";
	mStrings[OP_LEFT] = "left";
	mStrings[OP_RIGHT] = "right";
	mStrings[OP_SPEED] = "gamespeed:";
	mStrings[OP_VSLOW] = "very slow";
	mStrings[OP_SLOW] = "slow";
	mStrings[OP_DEFAULT] = "default";
	mStrings[OP_FAST] = "fast";
	mStrings[OP_VFAST] = "very fast";
	mStrings[OP_LANGUAGE] = "language";
	mStrings[OP_DIFFICULTY] = "bot strength";
	mStrings[OP_WEAK] = "weak";
	mStrings[OP_MEDIUM] = "medium";
	mStrings[OP_STRONG] = "strong";
	mStrings[OP_RULES] = "rules:";

	mStrings[UPDATE_NOTIFICATION] = "please visit http://blobby.sourceforge.net/ for a new version of blobby volley";
}

std::map<std::string, std::string> TextManager::language_names;

struct lang_init{
	lang_init(){
		TextManager::language_names["de"] = "deutsch";
		TextManager::language_names["en"] = "english";
		TextManager::language_names["fr"] = "francais";
		TextManager::language_names["es"] = "español";
		TextManager::language_names["it"] = "italiano";
		TextManager::language_names["cs"] = "česky";
	}
};
static lang_init init;


