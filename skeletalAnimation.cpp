#include <vulkan/vulkan.h>
#include <skeletalAnimation.h>

skeletalAnimation::skeletalAnimation(std::string name)
{
    asmpScene = importer.ReadFile(name,
    //aiProcess_CalcTangentSpace       |
    aiProcess_Triangulate            |
    aiProcess_JoinIdenticalVertices  |
    aiProcess_SortByPType);

    if(asmpScene == nullptr) {
        throw std::runtime_error("Failed to read file " + std::string(importer.GetErrorString()));
    }

    convertScene();
}

skeletalAnimation::~skeletalAnimation()
{
}

uint skeletalAnimation::FindPosition(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
        float t = (float)pNodeAnim->mPositionKeys[i + 1].mTime;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}

void skeletalAnimation::loadAnimation(std::string path) {
    asmpScene = importer.ReadFile(path,
    //aiProcess_CalcTangentSpace       |
    aiProcess_Triangulate            |
    aiProcess_JoinIdenticalVertices  |
    aiProcess_SortByPType);

    if(asmpScene == nullptr) {
        throw std::runtime_error("Failed to read file " + std::string(importer.GetErrorString()));
    }

    scene.bone_name_to_index_map.clear();
    scene.bones.resize(0, skeletalAnimation::Scene::BoneInfo(aiMatrix4x4()));
    //scene.globalTransforms.resize(0);
    scene.hierarchy.resize(0);
    scene.indices.resize(0);
    //scene.localTransforms.resize(0);
    scene.mesh_base_vertex.resize(0);
    scene.vertices.resize(0);

    convertScene();
}

void skeletalAnimation::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    uint PositionIndex = FindPosition(AnimationTimeTicks, pNodeAnim);
    uint NextPositionIndex = PositionIndex + 1;
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float t1 = (float)pNodeAnim->mPositionKeys[PositionIndex].mTime;
    float t2 = (float)pNodeAnim->mPositionKeys[NextPositionIndex].mTime;
    float DeltaTime = t2 - t1;
    float Factor = (AnimationTimeTicks - t1) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

void skeletalAnimation::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    uint RotationIndex = FindRotation(AnimationTime, pNodeAnim);
    uint NextRotationIndex = (RotationIndex + 1);
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float DeltaTime = pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime;
    float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}

uint skeletalAnimation::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (uint i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);
}

uint skeletalAnimation::FindScaling(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (uint i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
        float t = (float)pNodeAnim->mScalingKeys[i + 1].mTime;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void skeletalAnimation::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumScalingKeys == 1) {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    uint ScalingIndex = FindScaling(AnimationTimeTicks, pNodeAnim);
    uint NextScalingIndex = ScalingIndex + 1;
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
    float t1 = (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime;
    float t2 = (float)pNodeAnim->mScalingKeys[NextScalingIndex].mTime;
    float DeltaTime = t2 - t1;
    float Factor = (AnimationTimeTicks - (float)t1) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

void skeletalAnimation::loadBone(const aiBone* bone, uint meshI) {
    uint boneId = scene.getBoneId(bone);
    for(uint idWeightPairI = 0; idWeightPairI < bone->mNumWeights; idWeightPairI++) {
        aiVertexWeight idWeightPair = bone->mWeights[idWeightPairI];
        uint globalVertexId = scene.mesh_base_vertex[meshI] + idWeightPair.mVertexId;
        if(globalVertexId >= scene.vertices.size()) {
            throw std::runtime_error("global vertex id is greater than the number of vertices");
        }
        scene.vertices[globalVertexId].addBoneData(boneId, idWeightPair.mWeight);
    }
}

void skeletalAnimation::loadMeshes() {
    uint total_vertices = 0;
    scene.mesh_base_vertex.resize(asmpScene->mNumMeshes);
    for(uint meshI = 0; meshI < asmpScene->mNumMeshes; meshI++) {
        aiMesh* mesh = asmpScene->mMeshes[meshI];
        scene.mesh_base_vertex[meshI] = total_vertices;
        for(uint vertI = 0; vertI < mesh->mNumVertices; vertI++) {
            scene.vertices.push_back({});
            scene.vertices[scene.vertices.size()-1].pos = glm::vec3(mesh->mVertices[vertI].x, mesh->mVertices[vertI].y, mesh->mVertices[vertI].z);
        }
        for(uint faceI = 0; faceI < mesh->mNumFaces; faceI++) {
            scene.indices.push_back(mesh->mFaces[faceI].mIndices[0] + scene.mesh_base_vertex[meshI]);
            scene.indices.push_back(mesh->mFaces[faceI].mIndices[1] + scene.mesh_base_vertex[meshI]);
            scene.indices.push_back(mesh->mFaces[faceI].mIndices[2] + scene.mesh_base_vertex[meshI]);
        }
        for(uint boneI = 0; boneI < mesh->mNumBones; boneI++) {
            loadBone(mesh->mBones[boneI], meshI);
        }
        total_vertices += mesh->mNumVertices;
    }
}

aiNodeAnim* skeletalAnimation::findNodeAnim(const char* nodeName) {
    if(scene.animationIndex < 0) return nullptr;
    //std::cout << "Node name pointer: " << (void*)nodeName << std::endl;
    aiAnimation& anim = *(asmpScene->mAnimations[scene.animationIndex]);
    for(uint i = 0; i < anim.mNumChannels; i++) {
        //std::cout << std::string(nodeName) << ", " << std::string(anim.mChannels[i]->mNodeName.C_Str()) << std::endl;
        if(strcmp(anim.mChannels[i]->mNodeName.C_Str(), nodeName) == 0) {
            return anim.mChannels[i];
        }
    }
    return nullptr;
}

// (1) convert N-ary tree to left child right sibling
// (2) move bone-vertex information into a linear array
void skeletalAnimation::convertScene() {
    if(asmpScene->mNumAnimations > 0) {
        scene.animationIndex = 0;
        scene.animationDuration = asmpScene->mAnimations[scene.animationIndex]->mDuration;
    }
    scene.globalInverseTransform = asmpScene->mRootNode->mTransformation;
    scene.globalInverseTransform.Inverse();
    std::stack<aiNode*> s;
    s.push(asmpScene->mRootNode);
    struct Node root;
    root.name = std::string(asmpScene->mRootNode->mName.C_Str());
    root.firstChild = asmpScene->mRootNode->mNumChildren ? 1 : -1;
    root.nextSibling = -1;
    root.transformation = asmpScene->mRootNode->mTransformation;
    root.parent = -1;
    scene.hierarchy.push_back(root);
    while(!s.empty()) {
        aiNode* top = s.top();
        s.pop();
        size_t currentIndex = scene.hierarchy.size();
        for(uint i = 0; i < top->mNumChildren; i++) {
            s.push(top->mChildren[i]);
            struct Node someChild;
            someChild.name = std::string(top->mChildren[i]->mName.C_Str());
            someChild.parent = currentIndex-1;
            someChild.transformation = top->mTransformation;
            someChild.nextSibling = i < top->mNumChildren-1 ? currentIndex+i+1 : -1; // Checks if there is a next sibling
            someChild.firstChild = -1; // if someChild has no children this will stay at -1
            scene.hierarchy[someChild.parent].firstChild = currentIndex; // set's the first child of the parent to the first of mChildren
            scene.hierarchy.push_back(someChild);
        }
    }
    
    /*scene.vertices = {
        {{-0.5f, -1.5f, 0.1f}},
        {{0.5f, -1.5f, 0.1f}},
        {{0.5f, 0.5f, 0.1f}},
        {{-0.5f, 0.5f, 0.1f}}
    };

    scene.indices = {
        0, 1, 2, 2, 3, 0
    };*/
    loadMeshes();
}

void skeletalAnimation::traverse(float animationTime) {
    std::stack<uint> s;
    s.push(0); // index of the root node
    while (!s.empty())
    {
        uint top = s.top();
        s.pop();
        //std::cout << "Node name: " << top->name << std::endl;
        /*aiNodeAnim* nodeAnim = findNodeAnim(scene.hierarchy[top].name.c_str());
        if(nodeAnim) {
            std::cout << "Found a node animation" << std::endl;
            aiVector3D Scaling;
            CalcInterpolatedScaling(Scaling, animationTime, nodeAnim);
            aiMatrix4x4 ScalingM;
            ScalingM.Scaling(Scaling, ScalingM);

            // Interpolate rotation and generate rotation transformation matrix
            aiQuaternion RotationQ;
            CalcInterpolatedRotation(RotationQ, animationTime, nodeAnim);
            aiMatrix4x4 RotationM = aiMatrix4x4(RotationQ.GetMatrix());

            // Interpolate translation and generate translation transformation matrix
            aiVector3D Translation;
            CalcInterpolatedPosition(Translation, animationTime, nodeAnim);
            aiMatrix4x4 TranslationM;
            TranslationM.Translation(Translation, TranslationM);

            // Combine the above transformations
            scene.hierarchy[top].transformation = TranslationM * RotationM * ScalingM;
        }*/
        if(scene.hierarchy[top].parent > -1) {
            scene.hierarchy[top].transformation *= scene.hierarchy[scene.hierarchy[top].parent].transformation;
        }
        if(scene.bone_name_to_index_map.find(scene.hierarchy[top].name) != scene.bone_name_to_index_map.end()) {
            uint boneIndex = scene.bone_name_to_index_map[scene.hierarchy[top].name];
            scene.finalTransforms[boneIndex] = scene.globalInverseTransform * scene.hierarchy[top].transformation * scene.bones[boneIndex].OffsetMatrix;
        }
        if (scene.hierarchy[top].nextSibling > -1) s.push(scene.hierarchy[top].nextSibling);
        if (scene.hierarchy[top].firstChild > -1) s.push(scene.hierarchy[top].firstChild);
    }
}