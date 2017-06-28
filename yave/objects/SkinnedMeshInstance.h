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
#ifndef YAVE_OBJECTS_SKINNEDMESHINSTANCE_H
#define YAVE_OBJECTS_SKINNEDMESHINSTANCE_H

#include <yave/assets/AssetPtr.h>
#include <yave/material/Material.h>
#include <yave/meshs/SkinnedMesh.h>
#include <yave/animations/SkeletonInstance.h>

#include "Transformable.h"
#include "Renderable.h"

namespace yave {

class SkinnedMeshInstance : public Renderable {

	public:
		SkinnedMeshInstance(const AssetPtr<SkinnedMesh>& mesh, const AssetPtr<Material>& material);

		SkinnedMeshInstance(SkinnedMeshInstance&& other);
		SkinnedMeshInstance& operator=(SkinnedMeshInstance&& other) = delete;

		void render(const FrameToken&, CmdBufferRecorderBase& recorder, const SceneData& scene_data) const override;

		void animate(const AssetPtr<Animation>& anim) {
			_skeleton.animate(anim);
		}

	private:
		AssetPtr<SkinnedMesh> _mesh;

		mutable SkeletonInstance _skeleton;
		mutable AssetPtr<Material> _material;
};
}

#endif // YAVE_OBJECTS_SKINNEDMESHINSTANCE_H
