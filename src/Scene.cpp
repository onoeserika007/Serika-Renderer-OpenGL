#include "Scene.h"

void Scene::addModel(std::shared_ptr<Model> model)
{
	models_.emplace_back(model);
}

std::vector<std::shared_ptr<Model>>& Scene::getModels()
{
	// TODO: 在此处插入 return 语句
	return models_;
}
