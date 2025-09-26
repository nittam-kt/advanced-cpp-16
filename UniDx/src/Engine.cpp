#include "pch.h"
#include <UniDx/Engine.h>

#include <string>
#include <chrono>

#include <Keyboard.h>          // DirectXTK
#include <SimpleMath.h>        // DirectXTK 便利数学ユーティリティ
using namespace DirectX;
using namespace DirectX::SimpleMath;

// DirectXテクスチャライブラリを使用できるようにする
#include <DirectXTex.h>

// DirextXフォントライブラリを使用できるようにする
#include <SpriteFont.h>

#include <UniDx/D3DManager.h>
#include <UniDx/UniDxTime.h>
#include <UniDx/SceneManager.h>
#include <UniDx/Scene.h>
#include <UniDx/Behaviour.h>
#include <UniDx/Camera.h>
#include <UniDx/Renderer.h>
#include <UniDx/Physics.h>
#include <UniDx/LightManager.h>
#include <UniDx/Input.h>

// フォント描画用
std::unique_ptr<SpriteBatch> g_spriteBatch;
std::unique_ptr<SpriteFont>  g_spriteFont;
std::wstring text[4];

using namespace std;
using namespace UniDx;

namespace UniDx
{

// -----------------------------------------------------------------------------
//   Initialize(HWND hWnd)
// -----------------------------------------------------------------------------
void Engine::Initialize(HWND hWnd)
{
    // Direct3Dインスタンス作成
    D3DManager::create();

    // Direct3D初期化
    D3DManager::getInstance()->Initialize(hWnd, 1280, 720);

    // シーンマネージャのインスタンス作成
    SceneManager::create();

    // 入力の初期化
    Input::initialize();

    // 物理エンジンのインスタンス作成
    Physics::create();

    // ライトマネージャのインスタンス作成
    LightManager::create();

    // フォント初期化
    g_spriteBatch = std::make_unique<SpriteBatch>(D3DManager::getInstance()->GetContext().Get());
    g_spriteFont = std::make_unique<SpriteFont>(D3DManager::getInstance()->GetDevice().Get(), L"Resource/M PLUS 1.spritefont");
}


// -----------------------------------------------------------------------------
// 初期シーン作成
// -----------------------------------------------------------------------------
void Engine::createScene()
{
    SceneManager::getInstance()->createScene();

    // Awake
    for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
    {
        awake(&*it);
    }
}


// -----------------------------------------------------------------------------
// ゲーム全体のプレイヤーループ
// -----------------------------------------------------------------------------
int Engine::PlayerLoop()
{
    MSG msg;

    Time::Start();
    double restFixedUpdateTime = 0.0f;

    // デフォルトのシーン作成
    createScene();

    // メイン メッセージ ループ:
    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            //============================================
            // ウィンドウメッセージ処理
            //============================================
            // 終了メッセージがきた
            if (msg.message == WM_QUIT) {
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        using clock = std::chrono::steady_clock;          // モノトニックなので経過時間計測向き
        auto start = clock::now();

        //============================================
        // ゲームの処理を書く
        //============================================
        // 画面を塗りつぶす
        D3DManager::getInstance()->Clear(0.3f, 0.5f, 0.9f, 1.0f);

        Time::SetDeltaTimeFixed();

        while (restFixedUpdateTime > Time::fixedDeltaTime)
        {
            // 固定時間更新更新
            fixedUpdate();

            // 物理計算
            physics();

            restFixedUpdateTime -= Time::fixedDeltaTime;
        }

        Time::SetDeltaTimeFrame();

        // 入力更新
        input();

        // 更新処理
        update();

        // 後更新処理
        lateUpdate();

        // 描画処理
        render();

        // バックバッファの内容を画面に表示
        D3DManager::getInstance()->Present();

        // 時間計算
        double deltaTime = std::chrono::duration<double>(clock::now() - start).count();
        restFixedUpdateTime += deltaTime;

        Time::UpdateFrame(deltaTime);
    }

    // 終了処理
    finalize();

    return (int)msg.wParam;
}


// 固定時間更新更新
void Engine::fixedUpdate()
{
    for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
    {
        fixedUpdate(&*it);
    }
}


// 物理計算
void Engine::physics()
{
    Physics::getInstance()->simulatePositionCorrection(Time::fixedDeltaTime);
}


// 入力更新
void Engine::input()
{
    Input::update();
}


//
//  関数: Update()
//
//  目的: ゲームの更新処理を行います。
//
void Engine::update()
{
    // 各コンポーネントの Start()
    for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
    {
        checkStart(&*it);
    }

    // 各コンポーネントの Update()
    for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
    {
        update(&*it);
    }

    text[0] = std::to_wstring(0);
    text[1] = std::to_wstring(0);
    text[2] = std::to_wstring(0);
    text[3] = std::to_wstring(0);
}


// 後更新処理
void Engine::lateUpdate()
{
    // 各コンポーネントの LateUpdate()
    for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
    {
        lateUpdate(&*it);
    }
}


//
//  関数: Render()
//
//  目的: 画面の描画処理を行います。
//
void Engine::render()
{
    g_spriteBatch->Begin();

    Vector2 drawPos(100, 100);
    for (auto& str : text)
    {
        g_spriteFont->DrawString(g_spriteBatch.get(),
            str.c_str(),
            drawPos);
        drawPos.y += 50;
    }

    g_spriteBatch->End();

    // ライトバッファの更新と転送
    LightManager::getInstance()->updateLightCBuffer();

    // 各コンポーネントの Render()
    Camera* camera = Camera::main;
    if (camera != nullptr)
    {
        for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
        {
            render(&*it, *camera);
        }
    }
}


// 終了処理
void Engine::finalize()
{

}


void Engine::awake(GameObject* object)
{
    // 自身のコンポーネントの中でAwakeを呼び出していないものを呼ぶ
    for (auto& it : object->GetComponents())
    {
        it->checkAwake();
    }

    // 子供のオブジェクトについて再帰
    for (auto& it : object->transform->getChildGameObjects())
    {
        awake(&*it);
    }
}


void Engine::fixedUpdate(GameObject* object)
{
    // FixedUpdateを呼ぶ
    for (auto& it : object->GetComponents())
    {
        auto behaviour = dynamic_cast<Behaviour*>(it.get());
        if (behaviour != nullptr && behaviour->enabled)
        {
            behaviour->FixedUpdate();
        }
    }

    // 子供のオブジェクトについて再帰
    for (auto& it : object->transform->getChildGameObjects())
    {
        fixedUpdate(&*it);
    }
}


void Engine::checkStart(GameObject* object)
{
    // 自身のコンポーネントの中でStartを呼び出していないものを呼ぶ
    for (auto& it : object->GetComponents())
    {
        auto behaviour = dynamic_cast<Behaviour*>(it.get());
        if (behaviour != nullptr)
        {
            behaviour->checkStart();
        }
    }

    // 子供のオブジェクトについて再帰
    for (auto& it : object->transform->getChildGameObjects())
    {
        checkStart(&*it);
    }
}


void Engine::update(GameObject* object)
{
    // Updateを呼ぶ
    for (auto& it : object->GetComponents())
    {
        auto behaviour = dynamic_cast<Behaviour*>(it.get());
        if (behaviour != nullptr && behaviour->enabled)
        {
            behaviour->Update();
        }
    }

    // 子供のオブジェクトについて再帰
    for (auto& it : object->transform->getChildGameObjects())
    {
        update(&*it);
    }
}


void Engine::lateUpdate(GameObject* object)
{
    // 自身のコンポーネントの中でFixedUpdateを呼び出していないものを呼ぶ
    for (auto& it : object->GetComponents())
    {
        auto behaviour = dynamic_cast<Behaviour*>(it.get());
        if (behaviour != nullptr && behaviour->enabled)
        {
            behaviour->LateUpdate();
        }
    }

    // 子供のオブジェクトについて再帰
    for (auto& it : object->transform->getChildGameObjects())
    {
        lateUpdate(&*it);
    }
}


void Engine::render(GameObject* object, const Camera& camera)
{
    // 自身のコンポーネントの中でFixedUpdateを呼び出していないものを呼ぶ
    for (auto& it : object->GetComponents())
    {
        auto renderer = dynamic_cast<Renderer*>(it.get());
        if (renderer != nullptr && renderer->enabled)
        {
            renderer->Render(camera);
        }
    }

    // 子供のオブジェクトについて再帰
    for (auto& it : object->transform->getChildGameObjects())
    {
        render(&*it, camera);
    }
}

}