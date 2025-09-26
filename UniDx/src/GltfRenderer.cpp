#include "GltfRenderer.h"

#include <tiny_gltf.h>
#include <codecvt>

#include "UniDx.h"


namespace UniDx{

using namespace std;

namespace {

// tinygltf::Accessor から std::span<T> でデータを取得するヘルパー
template<typename T>
void ReadAccessorData(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor,
    vector<T>& out)
{
    if (accessor.bufferView < 0) return;
    const auto& bufferView = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[bufferView.buffer];
    size_t offset = bufferView.byteOffset + accessor.byteOffset;
    size_t count = accessor.count;
    const unsigned char* data = buffer.data.data() + offset;
    out.resize(count);

    // 型チェック
    if constexpr (is_same_v<T, Vector3>) {
        assert(accessor.type == TINYGLTF_TYPE_VEC3);
        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        for (size_t i = 0; i < count; ++i) {
            const float* v = reinterpret_cast<const float*>(data + i * bufferView.byteStride);
            out[i] = Vector3(v[0], v[1], v[2]);
        }
    }
    else if constexpr (is_same_v<T, Vector2>) {
        assert(accessor.type == TINYGLTF_TYPE_VEC2);
        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        for (size_t i = 0; i < count; ++i) {
            const float* v = reinterpret_cast<const float*>(data + i * bufferView.byteStride);
            out[i] = Vector2(v[0], v[1]);
        }
    }
    else if constexpr (is_same_v<T, Color>) {
        // glTFのCOLOR_0はfloat4またはubyte4
        if (accessor.type == TINYGLTF_TYPE_VEC3 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
            for (size_t i = 0; i < count; ++i) {
                const float* v = reinterpret_cast<const float*>(data + i * bufferView.byteStride);
                out[i] = Color(v[0], v[1], v[2], 1.0f);
            }
        }
        else if (accessor.type == TINYGLTF_TYPE_VEC4 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
            for (size_t i = 0; i < count; ++i) {
                const float* v = reinterpret_cast<const float*>(data + i * bufferView.byteStride);
                out[i] = Color(v[0], v[1], v[2], v[3]);
            }
        }
        else if (accessor.type == TINYGLTF_TYPE_VEC4 && accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
            for (size_t i = 0; i < count; ++i) {
                const uint8_t* v = reinterpret_cast<const uint8_t*>(data + i * bufferView.byteStride);
                out[i] = Color(
                    v[0] / 255.0f, v[1] / 255.0f, v[2] / 255.0f, v[3] / 255.0f);
            }
        }
    }
}

}


// -----------------------------------------------------------------------------
// gltfファイルを読み込み
// -----------------------------------------------------------------------------
bool GltfRenderer::load_(const wstring& filePath)
{
    Debug::Log(filePath);

    model = make_unique<tinygltf::Model>();
    tinygltf::TinyGLTF loader;
    string err, warn;

    auto path = ToUtf8(filePath);

    bool ok = loader.LoadBinaryFromFile(model.get(), &err, &warn, path.c_str());
    if (!warn.empty())
    {
        Debug::Log(warn);
    }
    if (!ok)
    {
        Debug::Log(err);
        return false;
    }

    // Meshの生成
    mesh.submesh.clear();

    for (const auto& gltfMesh : model->meshes)
    {
        for (const auto& primitive : gltfMesh.primitives)
        {
            auto sub = make_unique<OwnedSubMesh>();

            // POSITION
            if (auto it = primitive.attributes.find("POSITION"); it != primitive.attributes.end()) {
                const auto& accessor = model->accessors[it->second];
                sub->resizePositions(accessor.count);
                ReadAccessorData(*model, accessor, const_cast<vector<Vector3>&>(sub->mutablePositions()));
            }

            // NORMAL
            if (auto it = primitive.attributes.find("NORMAL"); it != primitive.attributes.end()) {
                const auto& accessor = model->accessors[it->second];
                sub->resizeNormals(accessor.count);
                ReadAccessorData(*model, accessor, const_cast<vector<Vector3>&>(sub->mutableNormals()));
            }

            // COLOR_0
            if (auto it = primitive.attributes.find("COLOR_0"); it != primitive.attributes.end()) {
                const auto& accessor = model->accessors[it->second];
                sub->resizeColors(accessor.count);
                ReadAccessorData(*model, accessor, const_cast<vector<Color>&>(sub->mutableColors()));
            }

            // TEXCOORD_0
            if (auto it = primitive.attributes.find("TEXCOORD_0"); it != primitive.attributes.end()) {
                const auto& accessor = model->accessors[it->second];
                sub->resizeUV(accessor.count);
                ReadAccessorData(*model, accessor, const_cast<vector<Vector2>&>(sub->mutableUV()));
            }
            // indices
            if (primitive.indices >= 0) {
                const auto& accessor = model->accessors[primitive.indices];
                sub->resizeIndices(accessor.count);
                auto& indices = const_cast<std::vector<uint32_t>&>(sub->mutableIndices());

                const auto& bufferView = model->bufferViews[accessor.bufferView];
                const auto& buffer = model->buffers[bufferView.buffer];
                size_t offset = bufferView.byteOffset + accessor.byteOffset;
                const unsigned char* data = buffer.data.data() + offset;

                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    // 32bit index
                    memcpy(indices.data(), data, accessor.count * sizeof(uint32_t));
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    // 16bit index → 32bitへ変換
                    for (size_t i = 0; i < accessor.count; ++i) {
                        indices[i] = reinterpret_cast<const uint16_t*>(data)[i];
                    }
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    // 8bit index → 32bitへ変換
                    for (size_t i = 0; i < accessor.count; ++i) {
                        indices[i] = data[i];
                    }
                }
            }

            sub->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            mesh.submesh.push_back(move(sub));
        }
    }

    // ノードから姿勢を取得。TODO:階層構造を作る
    int sceneIndex = model->defaultScene >= 0 ? model->defaultScene : 0;
    const auto& scene = model->scenes[sceneIndex];
    for (int nodeIndex : scene.nodes)
    {
        const auto& node = model->nodes[nodeIndex];

        // 行列を取得
        if (!node.matrix.empty())
        {
            // 4x4行列が直接指定されている場合は
            // どちらも列優先なので、順番にコピー
            Matrix matrix;
            for (int i = 0; i < 16; ++i)
            {
                reinterpret_cast<float*>(&matrix)[i] = static_cast<float>(node.matrix[i]);
            }
            matrix.Decompose(scale, rotation, position);
        }
        else {
            // translation/rotation/scaleから合成
            position = node.translation.size() == 3 ? Vector3((float)node.translation[0], (float)node.translation[1], (float)node.translation[2]) : Vector3::Zero;
            rotation = node.rotation.size() == 4 ? Quaternion((float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2], (float)node.rotation[3]) : Quaternion::Identity;
            scale = node.scale.size() == 3 ? Vector3((float)node.scale[0], (float)node.scale[1], (float)node.scale[2]) : Vector3::One;
        }
    }
    return true;
}

}
