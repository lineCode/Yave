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
#ifndef YAVE_GRAPHICS_MEMORY_DEVICEALLOCATOR_H
#define YAVE_GRAPHICS_MEMORY_DEVICEALLOCATOR_H

#include "DeviceMemoryHeap.h"
#include "DedicatedDeviceMemoryAllocator.h"

#include <unordered_map>
#include <mutex>

namespace yave {

class DeviceAllocator : NonCopyable, public DeviceLinked {

	using HeapType = std::pair<u32, MemoryType>;

	static constexpr usize dedicated_threshold = DeviceMemoryHeap::heap_size / 2;

	public:
		DeviceAllocator() = default;
		DeviceAllocator(DevicePtr dptr);

		DeviceMemory alloc(vk::Image image);
		DeviceMemory alloc(vk::Buffer buffer, MemoryType type);

		core::String dump_info() const;

	private:
		DeviceMemory alloc(vk::MemoryRequirements reqs, MemoryType type);
		DeviceMemory dedicated_alloc(vk::MemoryRequirements reqs, MemoryType type);

		std::unordered_map<HeapType, core::Vector<std::unique_ptr<DeviceMemoryHeap>>> _heaps;
		std::unordered_map<MemoryType, std::unique_ptr<DedicatedDeviceMemoryAllocator>> _dedicated_heaps;

		usize _max_allocs = 0;
		mutable std::mutex _lock;
};

}

#endif // YAVE_GRAPHICS_MEMORY_DEVICEALLOCATOR_H
