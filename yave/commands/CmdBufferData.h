/*******************************
Copyright (c) 2016-2017 Gr�goire Angerand

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
#ifndef YAVE_COMMANDS_CMDBUFFERDATA_H
#define YAVE_COMMANDS_CMDBUFFERDATA_H

#include "CmdBufferUsage.h"

namespace yave {

class CmdBufferDataProxy;

struct CmdBufferData : NonCopyable {
	vk::CommandBuffer cmd_buffer;
	vk::Fence fence;

	core::Vector<core::Arc<CmdBufferDataProxy>> keep_alive;
	CmdBufferPoolBase* pool = nullptr;



	CmdBufferData(vk::CommandBuffer buf, vk::Fence fen, CmdBufferPoolBase* p);

	CmdBufferData() = default;

	CmdBufferData(CmdBufferData&& other);
	CmdBufferData& operator=(CmdBufferData&& other);

	void swap(CmdBufferData& other);
};

}

#endif // YAVE_COMMANDS_CMDBUFFERDATA_H