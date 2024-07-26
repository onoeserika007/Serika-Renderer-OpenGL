#pragma once

#include <string>
#include <memory>

#include "UMesh.h"
#include "Object.h"

class FMaterial;
class UObject;
class FCamera;
class Texture;
class ULight;
class Shader;
class Renderer;
class UMesh;

class UModel: public UMesh{
public:
	template <typename... Args>
	static std::shared_ptr<UModel> makeModel(Args&&... args);

	virtual std::unique_ptr<UObject> Clone() const override { return std::unique_ptr<UModel>(new UModel(*this)); }

	UModel& operator=(const UModel& other) = delete;
private:
	UModel();

	explicit UModel(const UMesh& other) {}
};

template<typename ... Args>
std::shared_ptr<UModel> UModel::makeModel(Args &&...args) {
	return std::shared_ptr<UModel>(new UModel(std::forward<Args>(args)...));
}
