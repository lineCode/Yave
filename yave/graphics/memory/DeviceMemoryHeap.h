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
#ifndef YAVE_GRAPHICS_MEMORY_DEVICEMEMORYHEAP_H
#define YAVE_GRAPHICS_MEMORY_DEVICEMEMORYHEAP_H

#include <y/concurrent/SpinLock.h>

#include "DeviceMemoryHeapBase.h"

namespace yave {

// For DeviceAllocator, should not be used directly
class DeviceMemoryHeap : public DeviceMemoryHeapBase {

	struct FreeBlock {
		usize offset;
		usize size;

		usize end_offset() const;
		bool contiguous(const FreeBlock& blck) const;
		void merge(const FreeBlock& block);
	};

	public:
		static constexpr usize alignment = 256;

		static constexpr usize heap_size = 1024 * 1024 * 128;

		static_assert(heap_size % alignment == 0, "Heap size is not a multiple of alignment");


		DeviceMemoryHeap(DevicePtr dptr, u32 type_bits, MemoryType type);
		~DeviceMemoryHeap() override;

		core::Result<DeviceMemory> alloc(vk::MemoryRequirements reqs) override;
		void free(const DeviceMemory& memory) override;

		void* map(const DeviceMemoryView& view) override;
		void unmap(const DeviceMemoryView&) override;

		usize available() const; // slow!
		bool mapped() const;

	private:
		void swap(DeviceMemoryHeap& other);

		DeviceMemory create(usize offset, usize size);
		void free(const FreeBlock& block);
		void compact_block(FreeBlock block);

		vk::DeviceMemory _memory;
		core::Vector<FreeBlock> _blocks;
		u8* _mapping = nullptr;

};

}

#endif // YAVE_GRAPHICS_MEMORY_DEVICEMEMORYHEAP_H
