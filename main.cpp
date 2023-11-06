#include <iostream>
#include <thread>

#include <DigitalKeyboard.hpp>
#include <os.hpp>
#include <time.hpp>

using namespace soup;

[[nodiscard]] static std::string valueToString(uint8_t value)
{
	std::string str;
	str.push_back((value & 0b100) ? '#' : '_');
	str.push_back((value & 0b010) ? '#' : '_');
	str.push_back((value & 0b001) ? '#' : '_');
	return str;
}

[[nodiscard]] static Key sequenceToKey(uint16_t sequence)
{
	switch (sequence)
	{
	case 0b100'100: return KEY_A;         // #__ #__
	case 0b100'010: return KEY_B;         // #__ _#_
	case 0b100'001: return KEY_C;         // #__ __#
	case 0b100'110: return KEY_D;         // #__ ##_
	case 0b100'011: return KEY_E;         // #__ _##
	case 0b100'101: return KEY_F;         // #__ #_#
	case 0b100'111: return KEY_G;         // #__ ###
	case 0b010'100: return KEY_H;         // _#_ #__
	case 0b010'010: return KEY_I;         // _#_ _#_
	case 0b010'001: return KEY_J;         // _#_ __#
	case 0b010'110: return KEY_K;         // _#_ ##_
	case 0b010'011: return KEY_L;         // _#_ _##
	case 0b010'101: return KEY_M;         // _#_ #_#
	case 0b010'111: return KEY_N;         // _#_ ###
	case 0b001'100: return KEY_O;         // __# #__
	case 0b001'010: return KEY_P;         // __# _#_
	case 0b001'001: return KEY_Q;         // __# __#
	case 0b001'110: return KEY_R;         // __# ##_
	case 0b001'011: return KEY_S;         // __# _##
	case 0b001'101: return KEY_T;         // __# #_#
	case 0b001'111: return KEY_U;         // __# ###
	case 0b101'100: return KEY_V;         // #_# #__
	case 0b101'010: return KEY_W;         // #_# _#_
	case 0b101'001: return KEY_X;         // #_# __#
	case 0b101'110: return KEY_Y;         // #_# ##_
	case 0b101'011: return KEY_Z;         // #_# _##
	case 0b101'101: return KEY_SPACE;     // #_# #_#
	case 0b101'111: return KEY_PERIOD;    // #_# ###
	case 0b110'100: return KEY_1;         // ##_ #__
	case 0b110'010: return KEY_2;         // ##_ _#_
	case 0b110'001: return KEY_3;         // ##_ __#
	case 0b110'110: return KEY_4;         // ##_ ##_
	case 0b110'011: return KEY_5;         // ##_ _##
	case 0b110'101: return KEY_6;         // ##_ #_#
	case 0b110'111: return KEY_7;         // ##_ ###
	case 0b011'100: return KEY_8;         // _## #__
	case 0b011'010: return KEY_9;         // _## _#_
	case 0b011'001: return KEY_0;         // _## __#
	case 0b011'110: return KEY_BACKSPACE; // _## ##_
	case 0b011'011: return KEY_ENTER;     // _## _##
	case 0b011'101'100: return KEY_BRACKET_LEFT;  // _## #_# #__
	case 0b011'101'010: return KEY_BACKSLASH;     // _## #_# _#_
	case 0b011'101'001: return KEY_BRACKET_RIGHT; // _## #_# __#
	case 0b011'101'110: return KEY_SEMICOLON;     // _## #_# ##_
	case 0b011'101'011: return KEY_SLASH;         // _## #_# _##
	case 0b011'101'101: return KEY_QUOTE;         // _## #_# #_#
	case 0b011'101'111: return KEY_MINUS;         // _## #_# ###
	case 0b011'111'100: return KEY_EQUALS;        // _## ### #__
	case 0b011'111'010: return KEY_COMMA;         // _## ### _#_
	case 0b011'111'110: return KEY_ESCAPE;        // _## ### ##_
	case 0b011'111'011: return KEY_TAB;           // _## ### _##

	// Navigation keys
	case 0b011'111'111'100: return KEY_ARROW_LEFT;  // _## ### ### #__
	case 0b011'111'111'001: return KEY_ARROW_RIGHT; // _## ### ### __#
	case 0b011'111'111'110: return KEY_ARROW_UP;    // _## ### ### ##_
	case 0b011'111'111'011: return KEY_ARROW_DOWN;  // _## ### ### _##
	case 0b011'111'111'010: return KEY_HOME;        // _## ### ### _#_
	case 0b011'111'111'101: return KEY_END;         // _## ### ### #_#
	case 0b011'111'101'110: return KEY_PAGE_UP;     // _## ### #_# ##_
	case 0b011'111'101'011: return KEY_PAGE_DOWN;   // _## ### #_# _##

	// Dead end/unused sequences
	case 0b011'111'001: return KEY_OEM_1;     // _## ### __#
	case 0b011'111'111'111: return KEY_OEM_1; // _## ### ### ###
	case 0b011'111'101'100: return KEY_OEM_1; // _## ### #_# #__
	case 0b011'111'101'010: return KEY_OEM_1; // _## ### #_# _#_
	case 0b011'111'101'001: return KEY_OEM_1; // _## ### #_# __#
	case 0b011'111'101'101: return KEY_OEM_1; // _## ### #_# #_#
	case 0b011'111'101'111: return KEY_OEM_1; // _## ### #_# ###

	// Sequences that can only be reached with shift
	case 0b111'100: return KEY_LCTRL; // ### ### #__
	case 0b111'010: return KEY_LMETA; // ### ### _#_
	case 0b111'001: return KEY_LALT;  // ### ### __#
	case 0b111'110: return KEY_OEM_1; // ### ### ##_
	case 0b111'011: return KEY_OEM_1; // ### ### _##
	case 0b111'101: return KEY_OEM_1; // ### ### #_#
	case 0b111'111: return KEY_CAPS_LOCK; // ### ### ###
	}
	return KEY_NONE;
}

int main(int argc, const char** argv)
{
	DigitalKeyboard kbd;
	bool shift = false;
	bool ctrl = false;
	bool meta = false;
	bool alt = false;
	uint8_t current = 0;
	uint16_t sequence = 0;
	time_t deadline = 0;
	while (true)
	{
		kbd.update();

		uint8_t value = 0;
		value |= (0b100 * kbd.keys[KEY_F13]);
		value |= (0b010 * kbd.keys[KEY_F14]);
		value |= (0b001 * kbd.keys[KEY_F15]);

		current |= value;

		if (value == 0) // All keys released now?
		{
			if (current != 0) // Any key was pressed?
			{
				std::cout << valueToString(current) << " ";

				if (sequence == 0 && !shift && current == 0b111)
				{
					shift = true;
				}
				else
				{
					sequence <<= 3;
					sequence |= current;
				}
				current = 0;

				if (auto key = sequenceToKey(sequence); key != KEY_NONE) // End of sequence?
				{
					if (key == KEY_LCTRL)
					{
						ctrl = true;
					_enabled_modifier_key:
						shift = false;
						sequence = 0;
						key = KEY_NONE;
						deadline = time::millis() + 1000;
					}
					else if (key == KEY_LMETA)
					{
						meta = true;
						goto _enabled_modifier_key;
					}
					else if (key == KEY_LALT)
					{
						alt = true;
						goto _enabled_modifier_key;
					}
					else
					{
						SOUP_IF_LIKELY (key != KEY_OEM_1)
						{
							SOUP_IF_UNLIKELY (key == KEY_CAPS_LOCK)
							{
								shift = false;
							}
							os::simulateKeyPress(meta, ctrl, shift, alt, key);
							std::cout << "[Done]";
						}
						else
						{
							std::cout << "[Dead end]";
						}
						goto _reset_sequence_data;
					}
				}
				else
				{
					deadline = time::millis() + 500;
				}
			}
			else if (deadline != 0) // Waiting for further input?
			{
				if (time::millis() > deadline)
				{
					if (meta)
					{
						os::simulateKeyPress(KEY_LMETA);
					}
					std::cout << "[Timeout]";
				_reset_sequence_data:
					std::cout << "\n";
					shift = false;
					ctrl = false;
					meta = false;
					alt = false;
					sequence = 0;
					deadline = 0;
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(7));
	}
	return 0;
}
