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

#include "import.h"

#ifndef EDITOR_NO_ASSIMP

#include <yave/utils/FileSystemModel.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <unordered_set>

namespace editor {
namespace import {



static constexpr auto import_flags =
									 aiProcess_Triangulate |
									 aiProcess_FindInvalidData |
									 aiProcess_GenSmoothNormals |
									 aiProcess_CalcTangentSpace |
									 aiProcess_GenUVCoords |
									 aiProcess_ImproveCacheLocality |
									 aiProcess_OptimizeGraph |
									 aiProcess_OptimizeMeshes |
									 aiProcess_JoinIdenticalVertices |
									 aiProcess_ValidateDataStructure |
									 0;


core::Vector<Named<ImageData>> import_material_textures(core::ArrayView<aiMaterial*> materials, const core::String& filename) {
	const FileSystemModel* fs = FileSystemModel::local_filesystem();

	usize name_len = fs->filename(filename).size();
	core::String path(filename.data(), filename.size() - name_len);

	std::unordered_set<core::String> textures;
	for(aiMaterial* mat : materials) {
		if(mat->GetTextureCount(aiTextureType_DIFFUSE)) {
			aiString name;
			mat->GetTexture(aiTextureType_DIFFUSE, 0, &name);
			textures.insert(name.C_Str());
		}
	}

	core::Vector<Named<ImageData>> images;
	for(const auto& name : textures) {
		try {
			images.emplace_back(import_image(fs->join(path, name)));
		} catch(std::exception& e) {
			log_msg(fmt("Unable to load image: %, skipping.", e.what()), Log::Error);
		}
	}
	return images;
}

SceneData import_scene(const core::String& filename, SceneImportFlags flags) {
	y_profile();

	Assimp::Importer importer;
	auto scene = importer.ReadFile(filename, import_flags);

	if(!scene) {
		y_throw("Unable to load scene.");
	}

	if(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
		log_msg("Scene is incomplete!", Log::Warning);
	}

	auto meshes = core::ArrayView<aiMesh*>(scene->mMeshes, scene->mNumMeshes);
	auto animations = core::ArrayView<aiAnimation*>(scene->mAnimations, scene->mNumAnimations);
	auto materials = core::ArrayView<aiMaterial*>(scene->mMaterials, scene->mNumMaterials);

	SceneData data;

	if((flags & SceneImportFlags::ImportMeshes) != SceneImportFlags::None) {
		std::transform(meshes.begin(), meshes.end(), std::back_inserter(data.meshes), [=](aiMesh* mesh) {
			return Named(clean_asset_name(mesh->mName.C_Str()), import_mesh(mesh, scene));
		});
	}

	if((flags & SceneImportFlags::ImportAnims) != SceneImportFlags::None) {
		std::transform(animations.begin(), animations.end(), std::back_inserter(data.animations), [=](aiAnimation* anim) {
			return Named(clean_asset_name(anim->mName.C_Str()), import_animation(anim));
		});
	}

	if((flags & SceneImportFlags::ImportImages) != SceneImportFlags::None) {
		data.images = import_material_textures(materials, filename);
	}

	return data;
}

core::String supported_scene_extensions() {
	std::string extensions;
	Assimp::Importer importer;
	importer.GetExtensionList(extensions);
	return extensions;
}

}
}


#else

namespace editor {
namespace import {

SceneData import_scene(const core::String& filename) {
	unused(path);
	y_throw("Scene loading not supported.");
	SceneData data;
	return data;
}

core::String supported_scene_extensions() {
	return "";
}

}
}

#endif

