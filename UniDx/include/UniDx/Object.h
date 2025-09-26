#pragma once
#include <string>

#include "UniDxDefine.h"
#include "Property.h"

namespace UniDx {

// --------------------
// Object基底クラス
// --------------------
class Object
{
public:
    virtual ~Object() {}

    ReadOnlyProperty<wstring> name;

    Object(ReadOnlyProperty<wstring>::Getter nameGet) : name(nameGet) {}
};

} // namespace UniDx
