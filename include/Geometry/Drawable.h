#pragma once
#include "Object.h"

class UDrawableObject: public UObject {
    UDrawableObject();
    virtual std::vector<std::shared_ptr<UDrawableObject>> getDrawableChildren() const = 0;
};
