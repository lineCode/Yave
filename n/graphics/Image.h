/*******************************
Copyright (C) 2013-2015 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/

#ifndef N_GRAPHICS_IMAGE_H
#define N_GRAPHICS_IMAGE_H

#include <n/assets/Asset.h>
#include "Color.h"

namespace n {
namespace graphics {

namespace internal {
struct Image : core::NonCopyable
{
	Image(const math::Vec2ui &s, ImageFormat f = ImageFormat::RGBA8, const void *c = 0, bool flip = true) : format(f), size(s), data(c ? new byte[s.mul() * f.bytesPerPixel()] : 0) {
		if(c) {
			if(flip) {
				uint bpp = format.bytesPerPixel();
				for(uint i = 0; i != size.y(); i++) {
					byte *dat = (byte *)data;
					const byte *bc = (const byte *)c;
					uint j = size.y() - (i + 1);
					memcpy(dat + i * size.x() * bpp, bc + j * size.x() * bpp, size.x() * bpp);
				}
			} else {
				memcpy((void *)data, c, s.mul() * format.bytesPerPixel());
			}
		}
	}

	~Image() {
		delete[] (byte *)data;
	}

	const ImageFormat format;
	const math::Vec2ui size;
	const void *data;

};
}



class Image : public assets::Asset<internal::Image>
{
	friend class ImageLoader;
	public:
		Image() {
		}

		Image(const Image &image) : Image((assets::Asset<internal::Image>)image) {
		}

		Image(internal::Image *i) : assets::Asset<internal::Image>(std::move(i)) {
		}

		Image(const math::Vec2ui &s, ImageFormat f = ImageFormat::RGBA8, void *c = 0) : Image(new internal::Image(s, f, c)) {
		}

		math::Vec2ui getSize() const {
			const internal::Image *in = getInternal();
			if(!in) {
				return math::Vec2ui(0);
			}
			return in->size;
		}

		const void *data() const {
			const internal::Image *in = getInternal();
			if(!in) {
				return 0;
			}
			return in->data;
		}

		ImageFormat getFormat() const {
			const internal::Image *in = getInternal();
			if(!in)  {
				return ImageFormat(ImageFormat::None);
			}
			return in->format;
		}

		template<typename T = float>
		const Color<T> getPixel(const math::Vec2ui &pos) const {
			const internal::Image *in = getInternal();
			uint offset = pos.x() * in->size.y() + pos.y();
			byte *colorData = (byte *)in->data;
			return Color<T>(colorData + offset * in->format.bytesPerPixel(), in->format);
		}

		bool operator==(const Image &i) const {
			return Asset<internal::Image>::operator==(i);
		}

		bool operator!=(const Image &i) const {
			return !operator==(i);
		}

		bool operator<(const Image &i) const {
			return operator->() < i.operator->();
		}


	private:
		Image(const assets::Asset<internal::Image> &t) : assets::Asset<internal::Image>(t) {
		}

		const internal::Image *getInternal() const {
			return isValid() ? this->operator->() : (const internal::Image *)0;
		}

};

}
}

#endif // N_GRAPHICS_IMAGE_H
