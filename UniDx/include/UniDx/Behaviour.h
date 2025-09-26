#pragma once
#include <SimpleMath.h>

#include "Component.h"
#include "Collision.h"
#include "Transform.h"

namespace UniDx {

class Collider;

// --------------------
// Behaviour基底クラス
// --------------------
class Behaviour : public Component
{
public:
    virtual void FixedUpdate() {}
    virtual void Update() {}
    virtual void LateUpdate() {}
    virtual void OnTriggerEnter(Collider* other) {}
    virtual void OnTriggerStay(Collider* other) {}
    virtual void OnTriggerExit(Collider* other) {}
    virtual void OnCollisionEnter(const Collision& collision) {}
    virtual void OnCollisionStay(const Collision& collision) {}
    virtual void OnCollisionExit(const Collision& collision) {}

    virtual ~Behaviour() = default;

    template<typename T>
    T* GetComponent(bool includeInactive = false) const { return gameObject->GetComponent<T>(includeInactive); }
};


} // namespace UniDx
