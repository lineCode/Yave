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
#ifndef YAVE_FRAMEGRAPH_FRAMEGRAPH_H
#define YAVE_FRAMEGRAPH_FRAMEGRAPH_H

#include "FrameGraphResourcePool.h"
#include "FrameGraphPassBuilder.h"

#include <yave/graphics/barriers/Barrier.h>

namespace yave {

class FrameGraph : NonCopyable {

	struct ImageCreateInfo {
		math::Vec2ui size;
		ImageFormat format;
		ImageUsage usage = ImageUsage::None;
	};

	struct BufferCreateInfo {
		usize byte_size;
		BufferUsage usage = BufferUsage::None;
		MemoryType memory_type = MemoryType::DontCare;
	};

	public:
		FrameGraph(const std::shared_ptr<FrameGraphResourcePool>& pool);

		DevicePtr device() const;
		const FrameGraphResourcePool* resources() const;


		void render(CmdBufferRecorder& recorder) &&;


		FrameGraphPassBuilder add_pass(std::string_view name);

		FrameGraphMutableImageId declare_image(ImageFormat format, const math::Vec2ui& size);
		FrameGraphMutableBufferId declare_buffer(usize byte_size);

		template<typename T>
		FrameGraphMutableTypedBufferId<T> declare_typed_buffer(usize size = 1) {
			return FrameGraphMutableTypedBufferId<T>::from_untyped(declare_buffer(sizeof(T) * size));
		}


		void add_usage(FrameGraphImageId res, ImageUsage usage);
		void add_usage(FrameGraphBufferId res, BufferUsage usage);
		void set_cpu_visible(FrameGraphMutableBufferId res);


		math::Vec2ui image_size(FrameGraphImageId res) const;

	private:
		friend class FrameGraphPassBuilder;

		void alloc_resources();
		void release_resources(CmdBufferRecorder& recorder);

		std::shared_ptr<FrameGraphResourcePool> _pool;

		core::Vector<std::unique_ptr<FrameGraphPass>> _passes;

		using hash_t = std::hash<FrameGraphResourceId>;
		std::unordered_map<FrameGraphImageId, ImageCreateInfo, hash_t> _images;
		std::unordered_map<FrameGraphBufferId, BufferCreateInfo, hash_t> _buffers;

};

}

#endif // YAVE_FRAMEGRAPH_FRAMEGRAPH_H
