#include "SceneManager.h"

#include <memory>

#include "Material.h"
#include "GameObject.h"
#include "Transform.h"


namespace UniDx{

using namespace std;


// シーン作成
void SceneManager::createScene()
{
	activeScene = std::move(CreateDefaultScene());
//	defaultMaterial = make_unique<Material>();
//	defaultMaterial->shader.compile<VertexPN>(L"Resource/DefaultShade.hlsl");
}

}
