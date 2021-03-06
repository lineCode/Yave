/*******************************
Copyright (c) 2016-2019 Grégoire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/
#ifndef YAVE_WINDOW_EVENTHANDLER_H
#define YAVE_WINDOW_EVENTHANDLER_H

#include <yave/yave.h>
#include <y/math/Vec.h>

namespace yave {

enum class Key {
	Unknown,
	Tab,
	Clear,
	Backspace,
	Enter,
	Escape,
	PageUp,
	PageDown,
	End,
	Home,
	Left,
	Up,
	Right,
	Down,
	Insert,
	Delete,
	Space = ' ',
	A = 'A', B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10_Reserved, F11, F12
};

static_assert(char(Key::Z) == 'Z');

class EventHandler {
	public:
		enum MouseButton {
			LeftButton,
			RightButton,
			MiddleButton
		};

		virtual ~EventHandler() {
		}

		virtual void mouse_moved(const math::Vec2i&)						{}
		virtual void mouse_pressed(const math::Vec2i&, MouseButton)			{}
		virtual void mouse_released(const math::Vec2i&, MouseButton)		{}

		virtual void mouse_wheel(i32)										{}

		virtual void char_input(u32 character)								{ unused(character); }

		virtual void key_pressed(Key)										{}
		virtual void key_released(Key)										{}

};

}

#endif // YAVE_WINDOW_EVENTHANDLER_H
