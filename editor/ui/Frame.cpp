/*******************************
Copyright (c) 2016-2019 Gr�goire Angerand

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

#include "Frame.h"

#include <imgui/imgui.h>

namespace editor {

Frame::Frame(std::string_view title, u32 flags) : UiElement(title), _flags(flags) {
}

void Frame::paint(CmdBufferRecorder& recorder, const FrameToken& token) {
	if(!is_visible()) {
		return;
	}

	// this breaks everthing that relies on getting focus (like popups)
	// ImGui::SetNextWindowFocus();

	ImGui::SetNextWindowBgAlpha(0.0f);
	if(ImGui::BeginChild(_title_with_id.begin(), ImVec2(0, 0), false, _flags)) {
		paint_ui(recorder, token);
	}
	ImGui::EndChild();

}

}
