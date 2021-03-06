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
#ifndef Y_IO_READER_H
#define Y_IO_READER_H

#include <y/utils.h>
#include <y/core/Vector.h>

namespace y {
namespace io {

class Reader : NonCopyable {
	public:
		virtual ~Reader() {
		}

		virtual bool at_end() const = 0;

		virtual usize read(void* data, usize bytes) = 0;
		virtual void read_all(core::Vector<u8>& data) = 0;


		template<typename T>
		void read_one(T& t) {
			static_assert(std::is_trivially_copyable_v<T>, "read_one only works on trivially copyable data");
			//static_assert(std::is_arithmetic_v<T> || std::has_unique_object_representations<T>::value, "read_one only works on packed types");
			check_len(read(&t, sizeof(T)), sizeof(T));
		}

		template<typename T>
		void read_array(T* t, usize size) {
			static_assert(std::is_trivially_copyable_v<T>, "read_array only works on trivially copyable data");
			//static_assert(std::is_arithmetic_v<T> || std::has_unique_object_representations<T>::value, "read_array only works on packed types");
			check_len(read(t, size * sizeof(T)), size * sizeof(T));
		}

		template<typename T>
		T read_one() {
			T t;
			read_one(t);
			return t;
		}

	protected:
		static void check_len(usize len, usize expected) {
			if(len != expected) {
				y_throw("End of file reached.");
			}
		}
};

}
}

#endif // Y_IO_READER_H
