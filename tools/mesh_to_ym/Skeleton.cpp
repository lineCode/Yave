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

#include "Skeleton.h"

#include <unordered_map>
#include <optional>

struct BoneRef {
	u32 index;
	float weight;

	bool operator<(const BoneRef& other) const {
		return std::tie(weight, index) > std::tie(other.weight, other.index);
	}
};

static void add_bone_refs(aiBone* bone, u32 index, Vector<Vector<BoneRef>>& refs) {
	for(usize i = 0; i != bone->mNumWeights; ++i) {
		auto w = bone->mWeights[i];
		refs[w.mVertexId] << BoneRef{index, w.mWeight};
	}
}

static SkinWeights compute_skin(Vector<BoneRef>& refs) {
	sort(refs.begin(), refs.end());
	SkinWeights skin;

	usize max = std::min(refs.size(), skin.size);

	float total = 0.0f;
	for(usize i = 0; i != max; ++i) {
		total += refs[i].weight;
	}

	for(usize i = 0; i != max; ++i) {
		skin.indexes[i] = refs[i].index;
		skin.weights[i] = refs[i].weight / total;
	}

	return skin;
}


const Vector<Bone>& Skeleton::bones() const {
	return _bones;
}

const Vector<SkinWeights>& Skeleton::skin() const {
	return _skin;
}

Result<Skeleton> Skeleton::from_assimp(aiMesh* mesh, const aiScene* scene) {
	static_assert(sizeof(aiMatrix4x4) == sizeof(math::Transform<>), "aiMatrix4x4 should be 16 floats");

	if(!mesh || !mesh->mNumBones) {
		return Err();
	}


	std::unordered_map<std::string, std::pair<usize, aiBone*>> bone_map;
	for(usize i = 0; i != mesh->mNumBones; ++i) {
		aiBone* bone = mesh->mBones[i];
		bone_map[std::string(bone->mName.C_Str())] = std::pair(i, bone);
	}



	const aiNode* root = nullptr;
	auto bones = vector_with_capacity<Bone>(mesh->mNumBones);
	for(usize i = 0; i != mesh->mNumBones; ++i) {
		aiBone* bone = mesh->mBones[i];
		const aiNode* node = scene->mRootNode->FindNode(bone->mName);

		if(!node) {
			log_msg("Node not found.", Log::Error);
			return Err();
		}

		u32 parent_index = 0;
		std::string parent_name(node->mParent->mName.C_Str());
		auto parent = bone_map.find(parent_name);
		if(parent == bone_map.end()) {
			if(!root) {
				log_msg("Skeleton root: \"" + str(parent_name) + "\"");
				root = node->mParent;
				parent_index = u32(-1);
			} else {
				return Err();
			}
		} else {
			parent_index = u32(parent->second.first);
		}

		bones << Bone {
				str(bone->mName.C_Str()),
				parent_index,
				reinterpret_cast<const math::Transform<>&>(bone->mOffsetMatrix)
			};
	}
	if(!root) {
		log_msg("Skeleton is not rooted.", Log::Error);
		return Err();
	}



	Vector<Vector<BoneRef>> bone_per_vertex(mesh->mNumVertices, Vector<BoneRef>());
	for(usize i = 0; i != mesh->mNumBones; ++i) {
		aiBone* bone = mesh->mBones[i];
		add_bone_refs(bone, i, bone_per_vertex);
	}




	auto skin_points = vector_with_capacity<SkinWeights>(mesh->mNumVertices);
	std::transform(bone_per_vertex.begin(), bone_per_vertex.end(), std::back_inserter(skin_points), [](auto& bones) { return compute_skin(bones); });



	Skeleton skeleton;
	skeleton._bones = std::move(bones);
	skeleton._skin = std::move(skin_points);
	return Ok(std::move(skeleton));
}

