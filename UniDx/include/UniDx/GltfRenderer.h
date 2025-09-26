#pragma once

#include <tiny_gltf.h>

#include "Renderer.h"


namespace UniDx {

// --------------------
// GltfRendererクラス
// --------------------
class GltfRenderer : public MeshRenderer
{
public:
    // 画像ファイルを読み込む
    template<typename TVertex>
    bool load(const std::wstring& filePath)
    {
        if (load_(filePath))
        {
            for (auto& sub : mesh.submesh)
            {
                sub->createBuffer<TVertex>();
            }
            return true;
        }
        return false;
    }

    Vector3 position;
    Vector3 scale;
    Quaternion rotation;

protected:
    std::unique_ptr< tinygltf::Model> model;

    bool load_(const std::wstring& filePath);
};


} // namespace UniDx
