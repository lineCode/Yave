/*******************************
Copyright (c) 2016-2017 Grégoire Angerand

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
#ifndef YAVE_COMMANDS_CMDBUFFERRECORDER_H
#define YAVE_COMMANDS_CMDBUFFERRECORDER_H

#include <yave/yave.h>
#include <yave/Framebuffer.h>

#include "RecordedCmdBuffer.h"

#include <yave/mesh/StaticMeshInstance.h>
#include <yave/material/GraphicPipeline.h>
#include <yave/shaders/ComputeProgram.h>
#include <yave/descriptors/DescriptorSet.h>
#include <yave/Viewport.h>

namespace yave {

class CmdBufferRecorder : NonCopyable {

	public:
		CmdBufferRecorder(CmdBuffer&& buffer);

		CmdBufferRecorder(CmdBufferRecorder&& other);
		CmdBufferRecorder& operator=(CmdBufferRecorder&& other);

		RecordedCmdBuffer end();
		~CmdBufferRecorder();

		vk::CommandBuffer vk_cmd_buffer() const;

		const RenderPass& current_pass() const;
		const Viewport& viewport() const;

		CmdBufferRecorder& set_viewport(const Viewport& view);
		CmdBufferRecorder& bind_framebuffer(const RenderPass& render_pass, const Framebuffer& framebuffer);
		CmdBufferRecorder& bind_pipeline(const GraphicPipeline& pipeline, const DescriptorSet& vp);
		CmdBufferRecorder& draw(const StaticMeshInstance& mesh_instance);
		CmdBufferRecorder& dispatch(const ComputeProgram& program, const math::Vec3ui& size, const DescriptorSet& descriptor_set);

	private:
		void swap(CmdBufferRecorder& other);

		void end_render_pass();

		CmdBuffer _cmd_buffer;
		const RenderPass* _render_pass;
		Viewport _viewport;
};

}

#endif // YAVE_COMMANDS_CMDBUFFERRECORDER_H
