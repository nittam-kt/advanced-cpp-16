// CreateDefaultScene.cpp
// デフォルトのシーンを生成します

#include <numbers>

#include <UniDx.h>
#include <UniDx/Scene.h>
#include <UniDx/Camera.h>
#include <UniDx/PrimitiveRenderer.h>
#include <UniDx/GltfModel.h>
#include <UniDx/Rigidbody.h>
#include <UniDx/Random.h>
#include <UniDx/Collider.h>
#include <UniDx/Light.h>

#include "CameraBehaviour.h"
#include "Player.h"

using namespace std;
using namespace UniDx;


unique_ptr<Scene> CreateDefaultScene()
{
    // -- プレイヤー --
    auto player = make_unique<GameObject>(L"プレイヤー",
        make_unique<GltfModel>(),
        make_unique<Player>(),
        make_unique<Rigidbody>(),
        make_unique<SphereCollider>()
        );
    auto model = player->GetComponent<GltfModel>(true);
    model->load<VertexPNT>(
        L"Resource/ModularCharacterPBR.glb",
        L"Resource/AlbedoShade.hlsl",
        L"Resource/Albedo.png");

    player->transform->localPosition = Vector3(0,1,0);
    player->transform->localRotation = Quaternion::CreateFromYawPitchRoll(XM_PI, 0, 0);

    // -- 戦闘機--
    auto fighter = make_unique<GameObject>(L"自機",
        make_unique<GltfModel>(),
        make_unique<Rigidbody>(),
        make_unique<SphereCollider>()
    );
    auto modelFighter = fighter->GetComponent<GltfModel>(true);
    modelFighter->load<VertexPNT>(
        L"Resource/space_frigate_0.glb",
        L"Resource/AlbedoShade.hlsl",
        L"Resource/space_frigate.png");

    fighter->transform->localPosition = Vector3(-2, 2, 0);
    fighter->GetComponent<Rigidbody>(true)->gravityScale = 0;   // 飛ぶために重力0

    // キューブレンダラを作ってサイズを調整
    auto rb = make_unique<Rigidbody>();
    rb->gravityScale = 0;
    rb->mass = numeric_limits<float>::infinity();
    auto floor = make_unique<GameObject>(L"床",
        CubeRenderer::create<VertexPNT>(L"Resource/AlbedoShade.hlsl", L"Resource/brick-1.png"),
        move(rb),
        make_unique<AABBCollider>());
    floor->transform->localPosition = Vector3(0.0f, -0.5f, 0.0f);
    floor->transform->localScale = Vector3(5, 1, 5);

    // -- カメラ --
    auto cameraBehaviour = make_unique<CameraBehaviour>();
    cameraBehaviour->player = player->GetComponent<Player>(true);

    // -- ライト --
    auto light = make_unique<GameObject>(L"ライト",
        make_unique<Light>());
    light->transform->localRotation = Quaternion::CreateFromYawPitchRoll(0.2f, 0.9f, 0);

    // シーンを作って戻す
    return make_unique<Scene>(

        make_unique<GameObject>(L"オブジェクトルート",
            move(player),
            move(fighter),
            move(floor)
        ),

        move(light),

        make_unique<GameObject>(L"カメラルート",
            make_unique<Camera>(),
            move(cameraBehaviour)
        )
    );
}
