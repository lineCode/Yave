/*******************************
Copyright (c) 2016-2018 Gr�goire Angerand

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
#ifndef YAVE_RENDERERS_VARIANCERENDERER_H
#define YAVE_RENDERERS_VARIANCERENDERER_H

#include "DepthRenderer.h"

#include <yave/shaders/ComputeProgram.h>

namespace yave {

class VarianceRenderer : public BufferRenderer {

	public:
		static constexpr vk::Format variance_format = vk::Format::eR16G16Sfloat;

		VarianceRenderer(const Ptr<DepthRenderer>& depth, u32 half_kernel_size = 9);
		void build_frame_graph(RenderingNode<result_type>& node, CmdBufferRecorder<>& recorder) override;

		TextureView depth_variance() const override;

	private:
		core::Arc<DepthRenderer> _depth;

		ComputeProgram _h;
		ComputeProgram _v;

		StorageTexture _variance_h;
		StorageTexture _variance;

		DescriptorSet _descriptor_h;
		DescriptorSet _descriptor_v;

};
}

#endif // YAVE_RENDERERS_VARIANCERENDERER_H
