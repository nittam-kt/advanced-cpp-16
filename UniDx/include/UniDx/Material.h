#pragma once

#include <memory>
#include <map>

#include <SimpleMath.h>

#include "Component.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"

namespace UniDx {

class Camera;
class Texture;

// --------------------
// Materialクラス
// --------------------
class Material : public Object
{
public:
    Shader shader;
    ReadOnlyProperty<Texture*> mainTexture;
    bool zTest;
    D3D11_DEPTH_WRITE_MASK depthWrite;
    D3D11_COMPARISON_FUNC ztest;

    // コンストラクタ
    Material();

    // マテリアル情報設定。Render()内で呼び出す
    void setForRender() const;

    // テクスチャ追加
    void addTexture(std::unique_ptr<Texture> tex);

    // 有効化
    virtual void OnEnable();

protected:
    ComPtr<ID3D11Buffer> constantBuffer;
    ComPtr<ID3D11DepthStencilState> depthStencilState;

    std::vector<std::unique_ptr<Texture>> textures;
};


} // namespace UniDx
