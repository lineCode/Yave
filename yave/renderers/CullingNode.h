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
#ifndef YAVE_RENDERERS_CULLINGNODE_H
#define YAVE_RENDERERS_CULLINGNODE_H

#include <yave/scene/SceneView.h>
#include "renderers.h"

namespace yave {

class CullingNode : public Node {

	public:
		struct CullingResults {
			core::Vector<const StaticMesh*> static_meshes;
			core::Vector<const Renderable*> renderables;
			core::Vector<const Light*> lights;
		};

		using result_type = CullingResults;


		CullingNode(SceneView& view);
		void build_frame_graph(RenderingNode<result_type>& node);


		const SceneView& scene_view() const {
			return _view;
		}

	private:
		SceneView& _view;
};

}

#endif // YAVE_RENDERERS_CULLINGNODE_H
