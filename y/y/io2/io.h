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
#ifndef Y_IO2_IO_H
#define Y_IO2_IO_H

#include <memory>
#include <y/core/String.h>
#include <y/core/Vector.h>
#include <y/core/Result.h>

namespace y {
namespace io2 {

using Result = core::Result<void, usize>;


class Reader {
	struct InnerBase {
		virtual ~InnerBase() = default;

		virtual bool at_end() const = 0;
		virtual Result read(u8* data, usize max_bytes) = 0;
		virtual Result read_all(core::Vector<u8>& data) = 0;
	};

	template<typename T>
	struct Inner : InnerBase {
		Inner(T&& t) : _inner(std::move(t)) {
		}

		bool at_end() const override {
			return _inner.at_end();
		}

		Result read(u8* data, usize max_bytes) override {
			return _inner.read(data, max_bytes);
		}

		Result read_all(core::Vector<u8>& data) override {
			return _inner.read_all(data);
		}

		T _inner;
	};

	template<typename T>
	struct NotOwner : InnerBase {
		NotOwner(T& t) : _inner(t) {
		}

		bool at_end() const override {
			return _inner.at_end();
		}

		Result read(u8* data, usize max_bytes) override {
			return _inner.read(data, max_bytes);
		}

		Result read_all(core::Vector<u8>& data) override {
			return _inner.read_all(data);
		}

		T& _inner;
	};


	public:
		template<typename T>
		Reader(T&& t) : _inner(std::make_unique<Inner<T>>(y_fwd(t)))  {
			static_assert(!std::is_void_v<decltype(t.at_end())>);
		}

		template<typename T>
		explicit Reader(T& t) : _inner(std::make_unique<NotOwner<T>>(t))  {
			static_assert(!std::is_void_v<decltype(t.at_end())>);
		}

		Reader(Reader&&) = default;
		Reader& operator=(Reader&&) = default;

		bool at_end() const {
			return _inner->at_end();
		}

		Result read(u8* data, usize max_bytes) {
			return _inner->read(data, max_bytes);
		}

		Result read_all(core::Vector<u8>& data) {
			return _inner->read_all(data);
		}

	private:
		std::unique_ptr<InnerBase> _inner;
};


class Writer {
	struct InnerBase {
		virtual ~InnerBase() = default;

		virtual void flush() = 0;
		virtual Result write(const u8* data, usize bytes) = 0;
	};

	template<typename T>
	struct Inner : InnerBase {
		Inner(T&& t) : _inner(std::move(t)) {
		}

		void flush() override {
			_inner.flush();
		}

		Result write(const u8* data, usize bytes) override {
			return _inner.write(data, bytes);
		}

		T _inner;
	};

	template<typename T>
	struct NotOwner : InnerBase {
		NotOwner(T& t) : _inner(t) {
		}

		void flush() override {
			_inner.flush();
		}

		Result write(const u8* data, usize bytes) override {
			return _inner.write(data, bytes);
		}

		T& _inner;
	};

	public:
		template<typename T>
		Writer(T&& t) : _inner(std::make_unique<Inner<T>>(y_fwd(t))) {
		}

		template<typename T>
		Writer(T& t) : _inner(std::make_unique<NotOwner<T>>(t))  {
		}

		void flush() {
			_inner->flush();
		}

		Result write(const u8* data, usize bytes) {
			return _inner->write(data, bytes);
		}

	private:
		std::unique_ptr<InnerBase> _inner;
};

}
}

#endif // Y_IO2_IO_H
