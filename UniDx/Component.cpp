#include "Component.h"
#include "GameObject.h"
#include "Transform.h"

namespace UniDx{

// コンストラクタ
Component::Component() :
    Object([this]() {return gameObject->name;}),
    enabled(
        // get
        [this]() { return _enabled && isCalledAwake; },

        // set
        [this](bool value) {
            if (!_enabled && value) {
                if (!isCalledAwake) { Awake(); isCalledAwake = true; }
                OnEnable();
            }
            else if (_enabled && !value) {
                if (isCalledAwake) { OnDisable(); }
            }
            _enabled = value;
        }
    ),
    transform(
        [this]() { return gameObject->transform; }
    ),
    _enabled(true),
    isCalledAwake(false),
    isCalledStart(false)
{

}

// デストラクタ
Component::~Component()
{
    if (_enabled)
    {
        enabled = false;
    }
    if (isCalledAwake)
    {
        OnDestroy();
    }
}

}
