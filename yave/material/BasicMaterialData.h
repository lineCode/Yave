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
#ifndef YAVE_MATERIAL_BASICMATERIALDATA_H
#define YAVE_MATERIAL_BASICMATERIALDATA_H

#include <yave/graphics/bindings/DescriptorSet.h>

#include <yave/assets/AssetPtr.h>
#include <yave/assets/AssetLoader.h>

namespace yave {

class BasicMaterialData {

	struct BasicMaterialHeader {
		y_serde(fs::magic_number, AssetType::Material, u32(1))
	};

	public:
		static constexpr usize texture_count = 4;

		BasicMaterialData() = default;
		BasicMaterialData(std::array<AssetPtr<Texture>, texture_count>&& textures);

		const std::array<AssetPtr<Texture>, texture_count>& textures() const;
		bool is_empty() const;


		y_serialize(BasicMaterialHeader(), texture_ids())
		static core::Result<BasicMaterialData> load(io::ReaderRef reader, AssetLoader& loader) noexcept;

	private:
		std::array<AssetId, texture_count> texture_ids() const;

		std::array<AssetPtr<Texture>, texture_count> _textures;
};

}

#endif // YAVE_MATERIAL_BASICMATERIALDATA_H
