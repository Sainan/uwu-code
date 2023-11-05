#include <iostream>
#include <thread>

#include <DigitalKeyboard.hpp>
#include <macros.hpp>
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

int main(int argc, const char** argv)
{
	Key key_map[0b111'111'111 + 1];
	for (auto& key : key_map)
	{
		key = KEY_NONE;
	}
	key_map[0b100'100] = KEY_A;         // #__ #__
	key_map[0b100'010] = KEY_B;         // #__ _#_
	key_map[0b100'001] = KEY_C;         // #__ __#
	key_map[0b100'110] = KEY_D;         // #__ ##_
	key_map[0b100'011] = KEY_E;         // #__ _##
	key_map[0b100'101] = KEY_F;         // #__ #_#
	key_map[0b100'111] = KEY_G;         // #__ ###
	key_map[0b010'100] = KEY_H;         // _#_ #__
	key_map[0b010'010] = KEY_I;         // _#_ _#_
	key_map[0b010'001] = KEY_J;         // _#_ __#
	key_map[0b010'110] = KEY_K;         // _#_ ##_
	key_map[0b010'011] = KEY_L;         // _#_ _##
	key_map[0b010'101] = KEY_M;         // _#_ #_#
	key_map[0b010'111] = KEY_N;         // _#_ ###
	key_map[0b001'100] = KEY_O;         // __# #__
	key_map[0b001'010] = KEY_P;         // __# _#_
	key_map[0b001'001] = KEY_Q;         // __# __#
	key_map[0b001'110] = KEY_R;         // __# ##_
	key_map[0b001'011] = KEY_S;         // __# _##
	key_map[0b001'101] = KEY_T;         // __# #_#
	key_map[0b001'111] = KEY_U;         // __# ###
	key_map[0b101'100] = KEY_V;         // #_# #__
	key_map[0b101'010] = KEY_W;         // #_# _#_
	key_map[0b101'001] = KEY_X;         // #_# __#
	key_map[0b101'110] = KEY_Y;         // #_# ##_
	key_map[0b101'011] = KEY_Z;         // #_# _##
	key_map[0b101'101] = KEY_SPACE;     // #_# #_#
	key_map[0b101'111] = KEY_PERIOD;    // #_# ###
	key_map[0b110'100] = KEY_1;         // ##_ #__
	key_map[0b110'010] = KEY_2;         // ##_ _#_
	key_map[0b110'001] = KEY_3;         // ##_ __#
	key_map[0b110'110] = KEY_4;         // ##_ ##_
	key_map[0b110'011] = KEY_5;         // ##_ _##
	key_map[0b110'101] = KEY_6;         // ##_ #_#
	key_map[0b110'111] = KEY_7;         // ##_ ###
	key_map[0b011'100] = KEY_8;         // _## #__
	key_map[0b011'010] = KEY_9;         // _## _#_
	key_map[0b011'001] = KEY_0;         // _## __#
	key_map[0b011'110] = KEY_BACKSPACE; // _## ##_
	key_map[0b011'011] = KEY_ENTER;     // _## _##
	key_map[0b011'101'100] = KEY_BRACKET_LEFT; // _## #_# #__
	key_map[0b011'101'010] = KEY_BACKSLASH;    // _## #_# _#_
	key_map[0b011'101'001] = KEY_BRACKET_RIGHT; // _## #_# __#
	key_map[0b011'101'110] = KEY_SEMICOLON;    // _## #_# ##_
	key_map[0b011'101'011] = KEY_SLASH;        // _## #_# _##
	key_map[0b011'101'101] = KEY_QUOTE;        // _## #_# #_#
	key_map[0b011'101'111] = KEY_MINUS;        // _## #_# ###
	key_map[0b011'111'100] = KEY_EQUALS;       // _## ### #__
	key_map[0b011'111'010] = KEY_COMMA;        // _## ### _#_

	// Dead end/unused sequences
	key_map[0b011'111'001] = KEY_OEM_1; // _## ### __#
	key_map[0b011'111'110] = KEY_OEM_1; // _## ### ##_
	key_map[0b011'111'011] = KEY_OEM_1; // _## ### _##
	key_map[0b011'111'101] = KEY_OEM_1; // _## ### #_#
	key_map[0b011'111'111] = KEY_OEM_1; // _## ### ###

	// Sequences that can only be reached with shift
	key_map[0b111'100] = KEY_LCTRL; // ### ### #__
	key_map[0b111'010] = KEY_OEM_1; // ### ### _#_
	key_map[0b111'001] = KEY_OEM_1; // ### ### __#
	key_map[0b111'110] = KEY_OEM_1; // ### ### ##_
	key_map[0b111'011] = KEY_OEM_1; // ### ### _##
	key_map[0b111'101] = KEY_OEM_1; // ### ### #_#
	key_map[0b111'111] = KEY_CAPS_LOCK; // ### ### ###

	// Also consider that the following sequences could be given special meanings:
	// - Shift + Space     (### #_# #_#)
	// - Shift + Backspace (### _## ##_)
	// - Shift + Enter     (### _## _##)

	DigitalKeyboard kbd;
	bool shift = false;
	bool ctrl = false;
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

				if (sequence == 0 && !shift && !ctrl // First key in sequence?
					&& current == 0b111
					)
				{
					shift = true;
				}
				else
				{
					sequence <<= 3;
					sequence |= current;
				}
				current = 0;

				SOUP_IF_UNLIKELY (sequence >= COUNT(key_map))
				{
					std::cout << "[ERROR: Dead end that was not mapped]";
					goto _reset_sequence_data;
				}
				if (auto key = key_map[sequence]; key != KEY_NONE) // End of sequence?
				{
					if (key == KEY_LCTRL)
					{
						shift = false;
						ctrl = true;
						sequence = 0;
						key = KEY_NONE;
						deadline = time::millis() + 1000;
					}
					else
					{
						SOUP_IF_LIKELY (key != KEY_OEM_1)
						{
							SOUP_IF_UNLIKELY (key == KEY_CAPS_LOCK)
							{
								shift = false;
							}
							os::simulateKeyPress(ctrl, shift, false, key);
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
					std::cout << "[Timeout]";
				_reset_sequence_data:
					std::cout << "\n";
					shift = false;
					ctrl = false;
					sequence = 0;
					deadline = 0;
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(7));
	}
	return 0;
}
