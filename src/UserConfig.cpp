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
#include "UserConfig.h"

/* includes */
#include <iostream>
#include <map>

#include "tinyxml2.h"

#include "Global.h"
#include "FileRead.h"
#include "FileWrite.h"
#include "PlayerIdentity.h"
#include "LocalInputSource.h"


/* implementation */
std::map<std::string, std::shared_ptr<IUserConfigReader> >& userConfigCache()
{
     static std::map<std::string, std::shared_ptr<IUserConfigReader> > cache;
     return cache;
}

std::shared_ptr<IUserConfigReader> IUserConfigReader::createUserConfigReader(const std::string& file)
{
	// if we have this userconfig already cached, just return from cache
	auto cfg_cached = userConfigCache().find(file);
	if( cfg_cached != userConfigCache().end() )
	{
		return cfg_cached->second;
	}

	// otherwise, load user config...
	UserConfig* uc = new UserConfig();
	uc->loadFile(file);
	std::shared_ptr<IUserConfigReader> config(uc);

	// ... and add to cache
	userConfigCache()[file] = config;

	return config;
}

PlayerIdentity UserConfig::loadPlayerIdentity(PlayerSide side, bool force_human)
{
	std::string prefix = side == LEFT_PLAYER ? "left" : "right";
	std::string name;
	// init local input
	if(force_human)
	{
		name = getString(prefix + "_player_name");
	}
	else
	{
		name = getBool(prefix + "_player_human") ?
					getString(prefix + "_player_name") :
					getString(prefix + "_script_name") + ".lua";
	}

	PlayerIdentity player = PlayerIdentity(name);

	player.setStaticColor( Color(
							getInteger(prefix + "_blobby_color_r"),
							getInteger(prefix + "_blobby_color_g"),
							getInteger(prefix + "_blobby_color_b")
						) );

	player.setOscillating(getBool(prefix + "_blobby_oscillate"));
	player.setPreferredSide((PlayerSide)getInteger("network_side"));

	return player;
}

bool UserConfig::loadFile(const std::string& filename)
{
	auto configDoc = FileRead::readXMLDocument(filename);

	if (configDoc->Error())
	{
		std::cerr << "Warning: Parse error in " << filename;
		std::cerr << "!" << std::endl;
	}

	const auto* userConfigElem =
		configDoc->FirstChildElement("userconfig");
	if (!userConfigElem)
		return false;

	for (const auto* varElem = userConfigElem->FirstChildElement("var");
		varElem;
		varElem = varElem->NextSiblingElement("var"))
	{
		const char* c = varElem->Attribute("name");
		const char* v = varElem->Attribute("value");
		if(c && v) {
            createVar(c, v);
        } else {
		    std::cerr << "name of value missing for <var>" << std::endl;
		}
	}

	return true;
}

bool UserConfig::saveFile(const std::string& filename) const
{
	// this trows an exception if the file could not be opened for writing
	FileWrite file(filename);

	tinyxml2::XMLPrinter printer;
    printer.PushHeader(false, true);
	printer.OpenElement("userconfig");

	for (auto variable : mVars)
	{
		printer.OpenElement("var");
		printer.PushAttribute("name", variable->Name.c_str());
		printer.PushAttribute("value", variable->Value.c_str());
		printer.CloseElement();
	}
    printer.CloseElement();

	file.write(printer.CStr(), printer.CStrSize() - 1);  // do not write terminating \0 character
	file.close();

	// we have to make sure that we don't cache any outdated user configs
	auto cfg_cached = userConfigCache().find(filename);
	if( cfg_cached != userConfigCache().end() )
	{
		userConfigCache().erase(cfg_cached);
	}

	return true;
}

float UserConfig::getFloat(const std::string& name, float default_value) const
{
	auto var = checkVarByName(name);
	if (var)
		return std::atof( var->Value.c_str() );

	return default_value;
}

std::string UserConfig::getString(const std::string& name, const std::string& default_value) const
{
	auto var = checkVarByName(name);
	if (var)
		return var->Value;

	return default_value;
}

bool UserConfig::getBool(const std::string& name, bool default_value) const
{
	auto var = checkVarByName(name);
	if (var)
		return var->Value == "true";

	return default_value;
}

int UserConfig::getInteger(const std::string& name, int default_value) const
{
	auto var = checkVarByName(name);
	if (var)
		return std::atoi( var->Value.c_str() );

	return default_value;
}

void UserConfig::setFloat(const std::string& name, float var)
{
	char writeBuffer[256];
	snprintf(writeBuffer, 256, "%f", var);
	setValue(name, writeBuffer);
}

void UserConfig::setString(const std::string& name, const std::string& var)
{
	setValue(name, var);
}

void UserConfig::setBool(const std::string& name, bool var)
{
	setValue(name, var ? "true" : "false");
}

void UserConfig::setInteger(const std::string& name, int var)
{
	char writeBuffer[256];
	snprintf(writeBuffer, 256, "%d", var);
	setValue(name, writeBuffer);
}

UserConfigVar* UserConfig::createVar(const std::string& name, const std::string& value)
{
	if (findVarByName(name)) return nullptr;
	UserConfigVar *var = new UserConfigVar;
	var->Name = name;
	var->Value = value;
	mVars.push_back(var);
	return var;
}

void UserConfig::setValue(const std::string& name, const std::string& value)
{
	UserConfigVar *var = findVarByName(name);
	if (!var)
	{
		std::cerr << "Warning: impossible to set value of " <<
			"unknown configuration variable " << name <<
			"\n Creating new variable" << std::endl;
		var = createVar(name, value);
		mChangeFlag = true;
		return;
	}

	if (var->Value != value) mChangeFlag = true;
	var->Value = value;
}

UserConfig::~UserConfig()
{
	for (auto & var : mVars)
		delete var;
}

UserConfigVar* UserConfig::checkVarByName(const std::string& name) const
{
	auto var = findVarByName(name);
	if( !var )
	{
		std::cerr << "Warning: impossible to get value of " <<
			"unknown configuration variable " << name << std::endl;
	}
	return var;
}

UserConfigVar* UserConfig::findVarByName(const std::string& name) const
{
	for (auto var : mVars)
		if (var->Name == name) return var;

	return nullptr;
}

