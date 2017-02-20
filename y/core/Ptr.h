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

#ifndef Y_CORE_RC_H
#define Y_CORE_RC_H

#include <y/utils.h>

namespace y {
namespace core {

namespace detail {

template<typename T>
class Ptr : NonCopyable {

	public:
		using pointer = std::remove_extent_t<T>*;
		using const_pointer = const std::remove_extent_t<T>*;

		const T& operator*() const {
			return *_ptr;
		}

		T& operator*() {
			return *_ptr;
		}

		const_pointer operator->() const {
			return _ptr;
		}

		pointer operator->() {
			return _ptr;
		}

		operator void*() {
			return _ptr;
		}

		operator void const*() const {
			return _ptr;
		}

		bool operator<(const_pointer t) const {
			return _ptr < t;
		}

		bool operator>(const_pointer t) const {
			return _ptr > t;
		}

		bool operator!() const {
			return is_null();
		}

		bool is_null() const {
			return !_ptr;
		}

		pointer as_ptr() {
			return _ptr;
		}

		const_pointer as_ptr() const {
			return _ptr;
		}

	protected:
		Ptr() = default;

		Ptr(Owner<pointer>&& p) : _ptr(p) {
		}

		Owner<pointer> _ptr = nullptr;

		void destroy() {
			destroy(std::is_array<T>());
		}

	private:
		void destroy(std::false_type) {
			delete _ptr;
		}

		void destroy(std::true_type) {
			delete[] _ptr;
		}
};

}

template<typename T>
class Unique : public detail::Ptr<T> {

	using Base = detail::Ptr<T>;
	using Base::_ptr;

	public:
		using pointer = typename Base::pointer;
		using const_pointer = typename Base::const_pointer;

		Unique() = default;

		Unique(Owner<pointer>&& p) : Base(std::move(p)) {
		}

		Unique(T&& p) : Unique(new T(std::move(p))) {
		}

		Unique(std::nullptr_t p) : Base(p) {
		}

		Unique(Unique&& p) : Base() {
			swap(p);
		}

		~Unique() {
			Base::destroy();
		}

		Unique& operator=(Unique&& p) {
			swap(p);
			return *this;
		}

		Unique& operator=(Owner<pointer>&& p) {
			Unique ptr(std::move(p));
			swap(ptr);
			return *this;
		}

		Unique& operator=(std::nullptr_t p) {
			Unique ptr(p);
			swap(ptr);
			return *this;
		}

		void swap(Unique& p) {
			std::swap(_ptr, p._ptr);
		}

};

template<typename T, typename C = u32>
class Rc : public detail::Ptr<T> {

	using Base = detail::Ptr<T>;
	using Base::_ptr;

	public:
		using pointer = typename Base::pointer;
		using const_pointer = typename Base::const_pointer;

		Rc() = default;

		Rc(std::nullptr_t) : Rc() {
		}

		explicit Rc(Owner<pointer>&& p) : Base(std::move(p)), _count(new C(1)) {
		}

		explicit Rc(T&& p) : Rc(new T(std::move(p))) {
		}

		Rc(const Rc& p) : Rc(p._ptr, p._count) {
		}

		template<typename U, typename = std::enable_if_t<std::is_base_of<T, U>::value && std::is_polymorphic<T>::value>>
		Rc(const Rc<U, C>& p) : Rc(p._ptr, p._count) {
		}

		Rc(Rc&& p) : Rc() {
			swap(p);
		}

		~Rc() {
			unref();
		}

		void swap(Rc& p) {
			std::swap(_ptr, p._ptr);
			std::swap(_count, p._count);
		}

		C ref_count() const {
			return *_count;
		}

		Rc& operator=(const Rc& p) {
			if(p._count != _count) {
				unref();
				ref(p);
			}
			return *this;
		}

		Rc& operator=(std::nullptr_t) {
			unref();
			return *this;
		}

		Rc& operator=(Rc&& p) {
			swap(p);
			return *this;
		}

	private:
		template<typename U, typename D>
		friend class Rc;

		// only called by other Rc's so we take ownedship as well
		Rc(T* p, C* c) : Base(std::move(p)), _count(c) {
			++(*_count);
		}

		void ref(const Rc& p) {
			if((_count = p._count)) {
				(*_count)++;
			}
			_ptr = p._ptr;
		}

		void unref() {
			if(_count && !--(*_count)) {
				Base::destroy();
				delete _count;
			}
			_ptr = nullptr;
			_count = nullptr;
		}

		C* _count = nullptr;
};

template<typename T>
inline auto unique(T&& t) {
	return Unique<typename std::remove_reference<T>::type>(std::forward<T>(t));
}

template<typename T>
inline auto rc(T&& t) {
	return Rc<typename std::remove_reference<T>::type>(std::forward<T>(t));
}

}
}


#endif // Y_CORE_RC_H

