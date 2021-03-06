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
#ifndef Y_SERDE2_FORMATS_H
#define Y_SERDE2_FORMATS_H

#include "serde.h"

namespace y {

namespace io2 {
class Reader;
class Writer;
}

namespace serde2 {

struct BinaryFormat {
	template<typename R, typename T>
	Result read_array(R& reader, T* t, usize n) {
		static_assert(std::is_trivially_copyable_v<T>);
		return reader.read(reinterpret_cast<u8*>(t), sizeof(T) * n);
	}

	template<typename R, typename T>
	Result read_one(R& reader, T& t) {
		static_assert(std::is_trivially_copyable_v<T>);
		return reader.read(reinterpret_cast<u8*>(&t), sizeof(T));
	}


	template<typename W, typename T>
	Result write_array(W& writer, const T* t, usize n) {
		static_assert(std::is_trivially_copyable_v<T>);
		return writer.write(reinterpret_cast<const u8*>(t), sizeof(T) * n);
	}

	template<typename W, typename T>
	Result write_one(W& writer, const T& t) {
		static_assert(std::is_trivially_copyable_v<T>);
		return writer.write(reinterpret_cast<const u8*>(&t), sizeof(T));
	}
};

template<typename Format = serde2::BinaryFormat>
class FormattedReader : Format {
	public:
		FormattedReader(io2::Reader& reader) : _reader(reader) {
		}

		template<typename T>
		Result read_one(T& t) {
			static_assert(std::is_trivially_copyable_v<T>);
			return Format::read_one(_reader, t);
		}

		template<typename T>
		Result read_array(T* t, usize n) {
			static_assert(std::is_trivially_copyable_v<T>);
			return Format::read_array(_reader, t, n);
		}

	private:
		io2::Reader& _reader;
};


template<typename Format = serde2::BinaryFormat>
class FormattedWriter : Format {
	public:
		FormattedWriter(io2::Writer& writer) : _writer(writer) {
		}

		template<typename T>
		Result write_one(const T& t) {
			static_assert(std::is_trivially_copyable_v<T>);
			return Format::write_one(_writer, t);
		}

		template<typename T>
		Result write_array(const T* t, usize n) {
			static_assert(std::is_trivially_copyable_v<T>);
			return Format::write_array(_writer, t, n);
		}

	private:
		io2::Writer& _writer;
};

}
}

#endif // Y_SERDE2_FORMATS_H
