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
#include "ColorCorrectionRenderer.h"
#include "RenderingPipeline.h"

#include <y/io/File.h>

namespace yave {

static ComputeShader create_correction_shader(DevicePtr dptr) {
	return ComputeShader(dptr, SpirVData::from_file(io::File::open("correction.comp.spv").expected("Unable to open SPIR-V file.")));
}


ColorCorrectionRenderer::ColorCorrectionRenderer(const Ptr<BufferRenderer>& renderer) :
		EndOfPipeline(renderer->device()),
		_renderer(renderer),
		_correction_shader(create_correction_shader(device())),
		_correction_program(_correction_shader) {
}


void ColorCorrectionRenderer::build_frame_graph(RenderingNode<result_type>& node, CmdBufferRecorder<>& recorder) {
	auto image = node.add_dependency(_renderer, recorder);

	node.set_func([=, &recorder, token = node.token()]() {
			auto size = token.image_view.size();
#warning barrier ?
			recorder.dispatch(_correction_program, math::Vec3ui(size / _correction_shader.local_size().to<2>(), 1), {create_descriptor_set(token.image_view, image.get())});
		});
}

const DescriptorSet& ColorCorrectionRenderer::create_descriptor_set(const StorageView& out, const TextureView& in) {
	if(out.size() != in.size()) {
		fatal("Invalid output image size.");
	}

	auto it = _output_sets.find(out.vk_image_view());
	if(it == _output_sets.end()) {
		it = _output_sets.insert(std::pair(out.vk_image_view(), DescriptorSet(_renderer->device(), {
				Binding(in),
				Binding(out)
			}))).first;
	}
	return it->second;
}

}
