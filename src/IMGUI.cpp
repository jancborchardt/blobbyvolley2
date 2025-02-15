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
#include "IMGUI.h"

/* includes */
#include <queue>
#include <cassert>

#include <SDL2/SDL.h>

/* implementation */

enum ObjectType
{
	IMAGE,
	OVERLAY,
	TEXT,
	BUTTON, // Unused!
	SCROLLBAR,
	ACTIVESCROLLBAR,
	EDITBOX,
	ACTIVEEDITBOX,
	SELECTBOX,
	ACTIVESELECTBOX,
	BLOB,
	CHAT,
	ACTIVECHAT
};

struct QueueObject
{
	ObjectType type;
	int id;
	Vector2 pos1;
	Vector2 pos2;
	Color col;
	float alpha;
	std::string text;
	std::vector<std::string> entries;
	int selected;
	int length;
	unsigned int flags;
};

typedef std::queue<QueueObject> RenderQueue;

IMGUI* IMGUI::mSingleton = nullptr;
RenderQueue *mQueue;

IMGUI::IMGUI()
{
	mQueue = new RenderQueue;
	mActiveButton = -1;
	mHeldWidget = 0;
	mLastKeyAction = NONE;
	mLastWidget = 0;
	mButtonReset = false;
	mInactive = false;
	mIdCounter = 0;
}

IMGUI::~IMGUI()
{
	delete mQueue;
}

IMGUI& IMGUI::getSingleton()
{
	if (!mSingleton)
		mSingleton = new IMGUI;
	return *mSingleton;
}

void IMGUI::begin()
{
	mUsingCursor = false;
	mButtonReset = false;

	while (!mQueue->empty())
		mQueue->pop();


	mLastKeyAction = NONE;

	if (InputManager::getSingleton()->up())
		mLastKeyAction = UP;

	if (InputManager::getSingleton()->down())
		mLastKeyAction = DOWN;

	if (InputManager::getSingleton()->left())
		mLastKeyAction = LEFT;

	if (InputManager::getSingleton()->right())
		mLastKeyAction = RIGHT;

	if (InputManager::getSingleton()->select())
		mLastKeyAction = SELECT;

	if (InputManager::getSingleton()->exit())
		mLastKeyAction = BACK;

	mIdCounter = 0;
}

void IMGUI::end()
{
	int FontSize;
	RenderManager& rmanager = RenderManager::getSingleton();

	while (!mQueue->empty())
	{
		QueueObject& obj = mQueue->front();
		switch (obj.type)
		{
			case IMAGE:
				rmanager.drawImage(obj.text, obj.pos1, obj.pos2);
				break;

			case OVERLAY:
				rmanager.drawOverlay(obj.alpha, obj.pos1, obj.pos2, obj.col);
				break;

			case TEXT:
				rmanager.drawText(obj.text, obj.pos1, obj.flags);
				break;

			case SCROLLBAR:
				rmanager.drawOverlay(0.5, obj.pos1, obj.pos1 + Vector2(210.0, 26.0));
				rmanager.drawImage("gfx/scrollbar.bmp",obj.pos1 + Vector2(obj.pos2.x * 200.0 + 5 , 13));
				break;

			case ACTIVESCROLLBAR:
				rmanager.drawOverlay(0.4, obj.pos1, obj.pos1 + Vector2(210.0, 26.0));
				rmanager.drawImage("gfx/scrollbar.bmp",obj.pos1 + Vector2(obj.pos2.x * 200.0 + 5 , 13));
				break;

			case EDITBOX:
				FontSize = (obj.flags & TF_SMALL_FONT ? FONT_WIDTH_SMALL : FONT_WIDTH_NORMAL);
				rmanager.drawOverlay(0.5, obj.pos1, obj.pos1 + Vector2(10+obj.length*FontSize, 10+FontSize));
				rmanager.drawText(obj.text, obj.pos1+Vector2(5, 5), obj.flags);
				break;

			case ACTIVEEDITBOX:
				FontSize = (obj.flags & TF_SMALL_FONT ? FONT_WIDTH_SMALL : FONT_WIDTH_NORMAL);
				rmanager.drawOverlay(0.3, obj.pos1, obj.pos1 + Vector2(10+obj.length*FontSize, 10+FontSize));
				rmanager.drawText(obj.text, obj.pos1+Vector2(5, 5), obj.flags);
				if (obj.pos2.x >= 0)
					rmanager.drawOverlay(1.0, Vector2((obj.pos2.x)*FontSize+obj.pos1.x+5, obj.pos1.y+5), Vector2((obj.pos2.x)*FontSize+obj.pos1.x+5+3, obj.pos1.y+5+FontSize), Color(255,255,255));
				break;

			case SELECTBOX:
			case ACTIVESELECTBOX:
				FontSize = (obj.flags & TF_SMALL_FONT ? (FONT_WIDTH_SMALL+LINE_SPACER_SMALL) : (FONT_WIDTH_NORMAL+LINE_SPACER_NORMAL));
				rmanager.drawOverlay((obj.type == SELECTBOX ? 0.5 : 0.3), obj.pos1, obj.pos2);
				for (unsigned int c = 0; c < obj.entries.size(); c++)
				{
					if( c == static_cast<unsigned int>(obj.selected) )
						rmanager.drawText(obj.entries[c], Vector2(obj.pos1.x+5, obj.pos1.y+(c*FontSize)+5), obj.flags | TF_HIGHLIGHT);
					else
						rmanager.drawText(obj.entries[c], Vector2(obj.pos1.x+5, obj.pos1.y+(c*FontSize)+5), obj.flags);
				}
				break;

			case CHAT:
			case ACTIVECHAT:
				FontSize = (obj.flags & TF_SMALL_FONT ? (FONT_WIDTH_SMALL+LINE_SPACER_SMALL) : (FONT_WIDTH_NORMAL+LINE_SPACER_NORMAL));
				rmanager.drawOverlay((obj.type == CHAT ? 0.5 : 0.3), obj.pos1, obj.pos2);
				for (unsigned int c = 0; c < obj.entries.size(); c++)
				{
					if (obj.text[c] == 'R' )
						rmanager.drawText(obj.entries[c], Vector2(obj.pos1.x+5, obj.pos1.y+(c*FontSize)+5), obj.flags | TF_HIGHLIGHT);
					else
						rmanager.drawText(obj.entries[c], Vector2(obj.pos1.x+5, obj.pos1.y+(c*FontSize)+5), obj.flags);
				}
				break;

			case BLOB:
				rmanager.drawBlob(obj.pos1, obj.col);
				break;

			default:
				break;
		}
		mQueue->pop();
	}
#if BLOBBY_ON_DESKTOP
	if (mDrawCursor)
	{
		rmanager.drawImage("gfx/cursor.bmp", InputManager::getSingleton()->position() + Vector2(24.0, 24.0));
		mDrawCursor = false;
	}
#endif
	static bool lastCursor = false;
	bool relativeCursor = !mUsingCursor && InputManager::getSingleton()->isMouseCaptured();
	if( relativeCursor != lastCursor  )
	{
		lastCursor = relativeCursor;
		SDL_SetRelativeMouseMode( relativeCursor ? SDL_TRUE : SDL_FALSE );
	}
}

void IMGUI::doImage(int id, const Vector2& position, const std::string& name, const Vector2& size)
{
	QueueObject obj;
	obj.type = IMAGE;
	obj.id = id;
	obj.pos1 = position;
	obj.pos2 = size;
	obj.text = name;
	mQueue->push(obj);
}

void IMGUI::doText(int id, const Vector2& position, const std::string& text, unsigned int flags)
{
	QueueObject obj;
	obj.type = TEXT;
	obj.id = id;
	obj.pos1 = position;

	int const fontSize = flags & TF_SMALL_FONT ? FONT_WIDTH_SMALL : FONT_WIDTH_NORMAL;
	// update position depending on alignment
	if( flags & TF_ALIGN_CENTER )
	{
		obj.pos1.x -= text.size() * fontSize / 2;
	}

	if( flags & TF_ALIGN_RIGHT )
	{
		obj.pos1.x -= text.size() * fontSize;
	}


	obj.text = text;
	obj.flags = flags;
	mQueue->push(obj);
}

void IMGUI::doText(int id, const Vector2& position, TextManager::STRING text, unsigned int flags)
{
	doText(id, position, getText(text), flags);
}

void IMGUI::doOverlay(int id, const Vector2& pos1, const Vector2& pos2, const Color& col, float alpha)
{
	QueueObject obj;
	obj.type = OVERLAY;
	obj.id = id;
	obj.pos1 = pos1;
	obj.pos2 = pos2;
	obj.col = col;
	obj.alpha = alpha;
	mQueue->push(obj);
	RenderManager::getSingleton().redraw();
}

bool IMGUI::doButton(int id, const Vector2& position, TextManager::STRING text, unsigned int flags)
{
	return doButton(id, position, getText(text), flags);
}

bool IMGUI::doButton(int id, const Vector2& position, const std::string& text, unsigned int flags)
{
	bool clicked = false;
	QueueObject obj;
	obj.id = id;
	obj.pos1 = position;
	obj.text = text;
	obj.type = TEXT;
	obj.flags = flags;

	int const textLength = TextManager::getUTF8Length(text);
	int const fontSize = flags & TF_SMALL_FONT ? FONT_WIDTH_SMALL : FONT_WIDTH_NORMAL;

	// update position depending on alignment
	if( flags & TF_ALIGN_CENTER )
	{
		obj.pos1.x -= textLength * fontSize / 2;
	}

	if( flags & TF_ALIGN_RIGHT )
	{
		obj.pos1.x -= textLength * fontSize;
	}

	if (!mInactive)
	{
		// M.W. : Activate cursorless object-highlighting once the up or down key is pressed.
		if (mActiveButton == -1)
		{
			switch (mLastKeyAction)
			{
				case DOWN:
					mActiveButton = 0;
					mLastKeyAction = NONE;
					break;

				case UP:
					mActiveButton = mLastWidget;
					mLastKeyAction = NONE;
					break;

				default:
					break;
			}
		}

		// Highlight first menu object for arrow key navigation.
		if (mActiveButton == 0 && !mButtonReset)
			mActiveButton = id;

		// React to keyboard input.
		if (id == mActiveButton)
		{
			obj.flags = obj.flags | TF_HIGHLIGHT;
			switch (mLastKeyAction)
			{
				case DOWN:
					mActiveButton = 0;
					mLastKeyAction = NONE;
					break;

				case UP:
					mActiveButton = mLastWidget;
					mLastKeyAction = NONE;
					break;

				case SELECT:
					clicked = true;
					mLastKeyAction = NONE;
					break;
				default:
					break;
			}
		}

		// React to back button
		if (mLastKeyAction == BACK)
		{
			if ((text == getText(TextManager::LBL_CANCEL)) ||
			    (text == getText(TextManager::LBL_NO)) ||
			    (text == getText(TextManager::MNU_LABEL_EXIT)))
			{
				//todo: Workaround to catch backkey
				clicked = true;
				mActiveButton = id;
			}
		}

		#if BLOBBY_ON_MOBILE
			const int tolerance = 3;
		#else
			const int tolerance = 0;
		#endif
		// React to mouse input.
		Vector2 mousepos = InputManager::getSingleton()->position();
		if (mousepos.x + tolerance >= obj.pos1.x &&
			mousepos.y + tolerance * 2 >= obj.pos1.y &&
			mousepos.x - tolerance <= obj.pos1.x + textLength * fontSize &&
			mousepos.y - tolerance * 2 <= obj.pos1.y + fontSize)
		{
			obj.flags = obj.flags
			#if BLOBBY_ON_DESKTOP
				| TF_HIGHLIGHT
			#endif
			;
			if (InputManager::getSingleton()->click())
			{
				clicked = true;
				mActiveButton = id;
			}
		}
	}

	mLastWidget = id;
	mQueue->push(obj);
	return clicked;
}

bool IMGUI::doImageButton(int id, const Vector2& position, const Vector2& size, const std::string& image)
{
	doImage(id, position, image);

	// React to mouse input.
	if (InputManager::getSingleton()->click())
	{
		Vector2 mousepos = InputManager::getSingleton()->position();
		Vector2 btnpos = position - size * 0.5;
		if (mousepos.x > btnpos.x && mousepos.y > btnpos.y &&
				mousepos.x < btnpos.x + size.x &&	mousepos.y < btnpos.y + size.y)
		{
			return true;
		}
	}

	return false;
}

bool IMGUI::doScrollbar(int id, const Vector2& position, float& value)
{
	QueueObject obj;
	obj.id = id;
	obj.pos1 = position;
	obj.type = SCROLLBAR;

	bool deselected = false;

	if (InputManager::getSingleton()->unclick())
	{
		if (id == mHeldWidget)
			deselected = true;

		mHeldWidget = 0;
	}

	if (!mInactive)
	{
		// M.W. : Activate cursorless object-highlighting once the up or down key is pressed.
		if (mActiveButton == -1)
		{
			switch (mLastKeyAction)
			{
				case DOWN:
					mActiveButton = 0;
					mLastKeyAction = NONE;
					break;

				case UP:
					mActiveButton = mLastWidget;
					mLastKeyAction = NONE;
					break;

				default:
					break;
			}
		}

		// Highlight first menu object for arrow key navigation.
		if (mActiveButton == 0 && !mButtonReset)
			mActiveButton = id;

		// React to keyboard input.
		if (id == mActiveButton)
		{
			obj.type = ACTIVESCROLLBAR;
			switch (mLastKeyAction)
			{
				case DOWN:
					mActiveButton = 0;
					mLastKeyAction = NONE;
					break;

				case UP:
					mActiveButton = mLastWidget;
					mLastKeyAction = NONE;
					break;

				case LEFT:
					value -= 0.1;
					mLastKeyAction = NONE;
					break;

				case RIGHT:
					value += 0.1;
					mLastKeyAction = NONE;
					break;

				default:
					break;
			}
		}

		#if BLOBBY_ON_MOBILE
			const int tolerance = 3;
		#else
			const int tolerance = 0;
		#endif

		// React to mouse input.
		Vector2 mousepos = InputManager::getSingleton()->position();
		if (mousepos.x + 5 > position.x &&
			mousepos.y + tolerance * 2 > position.y &&
			mousepos.x < position.x + 205 &&
			mousepos.y - tolerance < position.y + 24.0)
		{
			obj.type = ACTIVESCROLLBAR;

			if (InputManager::getSingleton()->click())
			{
				mHeldWidget = id;
			}

			if (mHeldWidget == id)
			{
				value = (mousepos.x - position.x) / 200.0;
				mActiveButton = id;
			}

			if(InputManager::getSingleton()->mouseWheelUp())
				value += 0.1;

			if(InputManager::getSingleton()->mouseWheelDown())
				value -= 0.1;
		}
	}

	value = value > 0.f ? (value < 1.f ? value : 1.f) : 0.f;
	obj.pos2.x = value;

	mLastWidget = id;
	mQueue->push(obj);

	return deselected;
}

void IMGUI::resetSelection()
{
	mInactive = false;
	mActiveButton = -1;
	mButtonReset = true;
}

bool IMGUI::doEditbox(int id, const Vector2& position, unsigned int length, std::string& text, unsigned& cpos, unsigned int flags, bool force_active)
{
	int FontSize = (flags & TF_SMALL_FONT ? FONT_WIDTH_SMALL : FONT_WIDTH_NORMAL);
	bool changed = false;
	QueueObject obj;
	obj.id = id;
	obj.pos1 = position;
	obj.type = EDITBOX;
	obj.length = length; // length does not actually work!
	obj.flags = flags;

	// Width and height including border
	int width = length * FontSize + 10;
	int height = FontSize + 10;

		// update position depending on alignment
	if( flags & TF_ALIGN_CENTER )
	{
		obj.pos1.x -= width / 2;
	}

	// React to mouse input.
	Vector2 mousepos = InputManager::getSingleton()->position();
	if (mousepos.x > obj.pos1.x &&
		mousepos.y > obj.pos1.y &&
		mousepos.x < obj.pos1.x + width &&
		mousepos.y < obj.pos1.y + height)
	{
		obj.flags = obj.flags | TF_HIGHLIGHT;
		if (InputManager::getSingleton()->click())
		{
			// Handle click on the text.
			if (mousepos.x < obj.pos1.x + text.length() * FontSize)
				cpos = (int) ((mousepos.x-obj.pos1.x-5+(FontSize/2)) / FontSize);
			// Handle click behind the text.
			else if (mousepos.x < obj.pos1.x + width)
				cpos = (int) text.length();

			mActiveButton = id;

			// Show keyboard
			SDL_StartTextInput();
		}
	}

	if (!mInactive)
	{
		// M.W. : Activate cursorless object-highlighting once the up or down key is pressed.
		if (mActiveButton == -1)
		{
			switch (mLastKeyAction)
			{
				case DOWN:
					mActiveButton = 0;
					mLastKeyAction = NONE;
					break;

				case UP:
					mActiveButton = mLastWidget;
					mLastKeyAction = NONE;
					break;

				default:
					break;
			}

			// M.W. : Initialize the cursor position at the end of the string.
			// IMPORTANT: If you make changes to EditBox text that alter the length
			//				of the text, either call resetSelection() to come back
			//				to this area of code or update cpos manually to prevent
			//				crashes due to a misplaced cursor.
			cpos = text.length();
		}

		// Highlight first menu object for arrow key navigation.
		if (mActiveButton == 0 && !mButtonReset)
			mActiveButton = id;

		// React to keyboard input.
		if (id == mActiveButton || force_active)
		{
			obj.type = ACTIVEEDITBOX;
			switch (mLastKeyAction)
			{
				case DOWN:
					mActiveButton = 0;
					mLastKeyAction = NONE;
					break;

				case UP:
					mActiveButton = mLastWidget;
					mLastKeyAction = NONE;
					break;

				case LEFT:
					if (cpos > 0)
						cpos--;
					mLastKeyAction = NONE;
					break;

				case RIGHT:
					if (cpos < text.length())
						cpos++;
					mLastKeyAction = NONE;
					break;

				default:
					break;
			}
			std::string input = InputManager::getSingleton()->getLastTextKey();

			if (input == "backspace" && text.length() > 0 && cpos > 0)
			{
				text.erase(cpos - 1, 1);
				cpos--;
			}
			else if (input == "del" && text.length() > cpos)
			{
				text.erase(cpos, 1);
			}
			else if (input == "return")
			{
				// Workarround for chatwindow! Delete this after GUI-Rework
				changed = true;
			}
			// This is a temporary solution until the new
			// UTF-8 class can tell the real length!!!
			else if (text.length() < length)
			{
				if (input == "paste")
				{
					char * clipboardText = SDL_GetClipboardText();

					if (clipboardText != nullptr) {
						for (int i = 0; text.length() < length; i++)
						{
							const char c = clipboardText[i];

							// Only ASCII is currently supported
							if ((c == '\0') || (c < 0x20) || (c > 0xf7))
								break;

							text.insert(cpos, &c, 1);
							cpos++;
							changed = true;
						}

						SDL_free(clipboardText);
					}
				}
				if (input == "space")
				{
					text.insert(cpos, " ");
					cpos++;
					changed = true;
				}
				else if (input == "keypad0")
				{
					text.insert(cpos, "0");
					cpos++;
					changed = true;
				}
				else if (input == "keypad1")
				{
					text.insert(cpos, "1");
					cpos++;
					changed = true;
				}
				else if (input == "keypad2")
				{
					text.insert(cpos, "2");
					cpos++;
					changed = true;
				}
				else if (input == "keypad3")
				{
					text.insert(cpos, "3");
					cpos++;
					changed = true;
				}
				else if (input == "keypad4")
				{
					text.insert(cpos, "4");
					cpos++;
					changed = true;
				}
				else if (input == "keypad5")
				{
					text.insert(cpos, "5");
					cpos++;
					changed = true;
				}
				else if (input == "keypad6")
				{
					text.insert(cpos, "6");
					cpos++;
					changed = true;
				}
				else if (input == "keypad7")
				{
					text.insert(cpos, "7");
					cpos++;
					changed = true;
				}
				else if (input == "keypad8")
				{
					text.insert(cpos, "8");
					cpos++;
					changed = true;
				}
				else if (input == "keypad9")
				{
					text.insert(cpos, "9");
					cpos++;
					changed = true;
				}
				else if (input.length() == 1)
				{
					text.insert(cpos, input);
					cpos++;
					changed = true;
				}
			}
		}
	}

	obj.pos2.x = SDL_GetTicks() % 1000 >= 500 ? cpos : -1.0;
	obj.text = text;

	mLastWidget = id;
	mQueue->push(obj);

	// when content changed, it is active
	// part of chat window hack
	if( changed && force_active )
		mActiveButton = id;

	return changed;
}

SelectBoxAction IMGUI::doSelectbox(int id, const Vector2& pos1, const Vector2& pos2, const std::vector<std::string>& entries, unsigned int& selected, unsigned int flags)
{
	int FontSize = (flags & TF_SMALL_FONT ? (FONT_WIDTH_SMALL+LINE_SPACER_SMALL) : (FONT_WIDTH_NORMAL+LINE_SPACER_NORMAL));
	SelectBoxAction changed = SBA_NONE;
	QueueObject obj;
	obj.id = id;
	obj.pos1 = pos1;
	obj.pos2 = pos2;
	obj.type = SELECTBOX;
	obj.flags = flags;

	const int itemsPerPage = int(pos2.y - pos1.y - 10) / FontSize;
	int first = (int)(selected / itemsPerPage)*itemsPerPage; //the first visible element in the list

	if (!mInactive)
	{
		// M.W. : Activate cursorless object-highlighting once the up or down key is pressed.
		if (mActiveButton == -1)
		{
			switch (mLastKeyAction)
			{
				case DOWN:
					mActiveButton = 0;
					mLastKeyAction = NONE;
					break;

				case UP:
					mActiveButton = mLastWidget;
					mLastKeyAction = NONE;
					break;

				default:
					break;
			}
		}

		// Highlight first menu object for arrow key navigation.
		if (mActiveButton == 0 && !mButtonReset)
			mActiveButton = id;

		// React to keyboard input.
		if (id == mActiveButton)
		{
			obj.type = ACTIVESELECTBOX;
			switch (mLastKeyAction)
			{
				case DOWN:
					mActiveButton = 0;
					mLastKeyAction = NONE;
					break;

				case UP:
					mActiveButton = mLastWidget;
					mLastKeyAction = NONE;
					break;

				case LEFT:
					if (selected > 0)
					{
						selected--;
						changed = SBA_SELECT;
					}
					mLastKeyAction = NONE;
					break;

				case RIGHT:
					if (selected + 1 < entries.size())
					{
						selected++;
						changed = SBA_SELECT;
					}
					mLastKeyAction = NONE;
					break;

				default:
					break;
			}
		}

		// React to mouse input.
		Vector2 mousepos = InputManager::getSingleton()->position();
		if (mousepos.x > pos1.x && mousepos.y > pos1.y && mousepos.x < pos2.x && mousepos.y < pos2.y)
		{
			obj.type = ACTIVESELECTBOX;
			if (InputManager::getSingleton()->click())
				mActiveButton = id;
		}
		//entries mouseclick:
		if (mousepos.x > pos1.x &&
			mousepos.y > pos1.y+5 &&
			mousepos.x < pos2.x-35 &&
			mousepos.y < pos1.y+5+FontSize*itemsPerPage)
		{
			if (InputManager::getSingleton()->click())
			{
				int tmp = (int)((mousepos.y - pos1.y - 5) / FontSize) + first;
				/// \todo well, it's not really a doulbe click...
				/// we need to do this in inputmanager
				if( selected == static_cast<unsigned int>(tmp) && InputManager::getSingleton()->doubleClick() )
					changed = SBA_DBL_CLICK;

				if (tmp >= 0 && static_cast<unsigned int>(tmp) < entries.size())
					selected = tmp;

				mActiveButton = id;
			}
			if ((InputManager::getSingleton()->mouseWheelUp()) && (selected > 0))
			{
				selected--;
				changed = SBA_SELECT;
			}
			if ((InputManager::getSingleton()->mouseWheelDown()) && (selected + 1 < entries.size()))
			{
				selected++;
				changed = SBA_SELECT;
			}
		}
		//arrows mouseclick:
		if (mousepos.x > pos2.x-30 && mousepos.x < pos2.x-30+24 && InputManager::getSingleton()->click())
		{
			if (mousepos.y > pos1.y+3 && mousepos.y < pos1.y+3+24 && selected > 0)
			{
				selected--;
				changed = SBA_SELECT;
			}

			if (mousepos.y > pos2.y-27 && mousepos.y < pos2.y-27+24 && selected + 1 < entries.size())
			{
				selected++;
				changed = SBA_SELECT;
			}
		}
	}
	doImage(GEN_ID, Vector2(pos2.x-15, pos1.y+15), "gfx/pfeil_oben.bmp");
	doImage(GEN_ID, Vector2(pos2.x-15, pos2.y-15), "gfx/pfeil_unten.bmp");

	first = (selected / itemsPerPage)*itemsPerPage; //recalc first
	if ( !entries.empty() )
	{
		unsigned int last = first + itemsPerPage;
		if (last > entries.size())
			last = entries.size();

		obj.entries = std::vector<std::string>(entries.begin()+first, entries.begin()+last);
	}
	else
		obj.entries = std::vector<std::string>();

	obj.selected = selected-first;

	mLastWidget = id;
	mQueue->push(obj);

	return changed;
}

void IMGUI::doChatbox(int id, const Vector2& pos1, const Vector2& pos2, const std::vector<std::string>& entries, unsigned int& selected, const std::vector<bool>& local, unsigned int flags)
{
	assert( entries.size() == local.size() );
	int FontSize = (flags & TF_SMALL_FONT ? (FONT_WIDTH_SMALL+LINE_SPACER_SMALL) : (FONT_WIDTH_NORMAL+LINE_SPACER_NORMAL));
	QueueObject obj;
	obj.id = id;
	obj.pos1 = pos1;
	obj.pos2 = pos2;
	obj.type = CHAT;
	obj.flags = flags;

	const unsigned int itemsPerPage = int(pos2.y - pos1.y - 10) / FontSize;

	if (!mInactive)
	{
		// M.W. : Activate cursorless object-highlighting once the up or down key is pressed.
		if (mActiveButton == -1)
		{
			switch (mLastKeyAction)
			{
				case DOWN:
					mActiveButton = 0;
					mLastKeyAction = NONE;
					break;

				case UP:
					mActiveButton = mLastWidget;
					mLastKeyAction = NONE;
					break;

				default:
					break;
			}
		}

		// Highlight first menu object for arrow key navigation.
		if (mActiveButton == 0 && !mButtonReset)
			mActiveButton = id;

		// React to keyboard input.
		if (id == mActiveButton)
		{
			obj.type = ACTIVECHAT;
			switch (mLastKeyAction)
			{
				case DOWN:
					mActiveButton = 0;
					mLastKeyAction = NONE;
					break;

				case UP:
					mActiveButton = mLastWidget;
					mLastKeyAction = NONE;
					break;

				case LEFT:
					if (selected > 0)
					{
						selected--;
					}
					mLastKeyAction = NONE;
					break;

				case RIGHT:
					if (selected + 1 < entries.size())
					{
						selected++;
					}
					mLastKeyAction = NONE;
					break;

				default:
					break;
			}
		}

		// React to mouse input.
		Vector2 mousepos = InputManager::getSingleton()->position();
		if (mousepos.x > pos1.x && mousepos.y > pos1.y && mousepos.x < pos2.x && mousepos.y < pos2.y)
		{
			if (InputManager::getSingleton()->click())
				mActiveButton = id;
		}
		//entries mouseclick:
		if (mousepos.x > pos1.x &&
			mousepos.y > pos1.y+5 &&
			mousepos.x < pos2.x-35 &&
			mousepos.y < pos1.y+5+FontSize*itemsPerPage)
		{
			if ((InputManager::getSingleton()->mouseWheelUp()) && (selected > 0))
			{
				selected--;
			}

			if ((InputManager::getSingleton()->mouseWheelDown()) && (selected + 1 < entries.size()))
			{
				selected++;
			}
		}
		//arrows mouseclick:
		if (mousepos.x > pos2.x-30 && mousepos.x < pos2.x-30+24 && InputManager::getSingleton()->click())
		{
			if (mousepos.y > pos1.y+3 && mousepos.y < pos1.y+3+24 && selected > 0)
			{
				selected--;
			}

			if (mousepos.y > pos2.y-27 && mousepos.y < pos2.y-27+24 && selected + 1 < entries.size())
			{
				selected++;
			}
		}
	}
	doImage(GEN_ID, Vector2(pos2.x-15, pos1.y+15), "gfx/pfeil_oben.bmp");
	doImage(GEN_ID, Vector2(pos2.x-15, pos2.y-15), "gfx/pfeil_unten.bmp");

	unsigned int first = (selected / itemsPerPage) * itemsPerPage; //recalc first
	if ( !entries.empty() )
	{
		unsigned int last = selected + 1;
		/// \todo maybe we should adapt selected so we even can't scroll up further!
		// we don't want negative chatlog, so we just scroll upward without coming to negative
		// elements.
		if (last >= itemsPerPage)
			first = last - itemsPerPage;
		else
			first = 0;

		// check that we don't create out of bounds problems
		if(last > entries.size())
		{
			last = entries.size();
		}

		obj.entries = std::vector<std::string>(entries.begin()+first, entries.begin()+last);
		// HACK: we use taxt to store information which text is from local player and which from
		//			remote player.
		obj.text = "";
		for(unsigned int i = first; i < last; ++i)
		{
			obj.text += local[i] ? 'L' : 'R';
		}
	}
	else
		obj.entries = std::vector<std::string>();

	obj.selected = selected-first;

	mLastWidget = id;
	mQueue->push(obj);
}


bool IMGUI::doBlob(int id, const Vector2& position, const Color& col)
{
	QueueObject obj;
	obj.id = id;
	obj.pos1 = position;
	obj.type = BLOB;
	obj.col = col;
	mQueue->push(obj);
	return false;
}

void IMGUI::doCursor(bool draw)
{
	mDrawCursor = draw;
	mUsingCursor = true;
}

bool IMGUI::usingCursor() const
{
	return mUsingCursor;
}

const TextManager& IMGUI::textMgr() const {
	return *mTextManager;
}

const std::string& IMGUI::getText(TextManager::STRING id) const {
	return textMgr().getString(id);
}

void IMGUI::setTextMgr(std::string lang) {
	mTextManager.reset(new TextManager(std::move(lang)));
}