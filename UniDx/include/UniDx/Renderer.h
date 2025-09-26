#pragma once

#include <memory>
#include <map>

#include <SimpleMath.h>

#include "Component.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"


namespace UniDx {

class Camera;
class Material;


// --------------------
// Rendererクラス
// --------------------
class Renderer : public Component
{
public:
    std::vector< std::shared_ptr<Material> > materials;

    virtual void Render(const Camera& camera) const {}

    // マテリアルを追加（共有）
    void addMaterial(std::shared_ptr<Material> material)
    {
        materials.push_back(material);
    }

    // シェーダー読み込んでマテリアルを追加
    template<typename TVertex>
    void addMaterial(const std::wstring& shaderPath)
    {
        // マテリアルを追加
        materials.push_back(std::make_shared<Material>());

        // マテリアルをシェーダーを読み込んで初期化
        materials.back()->shader.compile<TVertex>(shaderPath);
    }

    // シェーダーとテクスチャを読み込んでマテリアルを追加
    template<typename TVertex>
    void addMaterial(const std::wstring& shaderPath, const std::wstring& textuePath)
    {
        addMaterial<TVertex>(shaderPath);

        // テクスチャを読み込んでマテリアルに追加
        std::unique_ptr<Texture> t = std::make_unique<Texture>();
        t->load(textuePath);
        materials.back()->addTexture(std::move(t));
    }

protected:
    ComPtr<ID3D11Buffer> constantBuffer0;

    virtual void OnEnable() override;
    virtual void updatePositionCameraCBuffer(const UniDx::Camera& camera) const;
    virtual void setShaderForRender() const;
};


// --------------------
// MeshRendererクラス
// --------------------
class MeshRenderer : public Renderer
{
public:
    Mesh mesh;

    // メッシュを使って描画
    virtual void Render(const Camera& camera) const override;
};


} // namespace UniDx
