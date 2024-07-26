#include "FScene.h"
#include "Light.h"
#include "Base/ResourceLoader.h"
#include "Geometry/BufferAttribute.h"
#include "Geometry/Model.h"
#include "Geometry/Primitives.h"

std::shared_ptr<FScene> FScene::generateDeaultScene() {
	auto scene = std::make_shared<FScene>();
    auto&& loader = ResourceLoader::getInstance();
    auto&& config = Config::getInstance();
    glm::vec3 focus {};

    // nanosuit
    // TEST_TIME_COST(auto nanosuit = loader.loadModel("./assets/models/nanosuit/nanosuit.obj", true), NANO_SUIT);
    // nanosuit->setScale(0.5, 0.5, 0.5);
    // nanosuit->setPosition(glm::vec3(0.f, -0.5f, 0.f));
    // nanosuit->setShadingMode(EShadingModel::Shading_BlinnPhong);
    // nanosuit->enableCastShadow(true);
    // focus = nanosuit->getPosition();
    // scene->addModel(nanosuit);;

    // floor
    auto floor = loader.loadModel("./assets/models/floor/floor.obj", false);
    floor->setPosition({0.f, 0.0f, 0.f});
    // floor->setScale({0.1f, 0.1f, 0.1f});
    floor->setShadingMode(EShadingModel::Shading_BlinnPhong);
    floor->enableCastShadow(true);
    floor->enableFaceCull(false);
    scene->addModel(floor);

    // marry 202 chyan
    if (auto mary = loader.loadModel("./assets/models/mary/mary.obj", false)) {
        // mary->setScale(0.1, 0.1, 0.1);
        mary->setPosition(glm::vec3(-2.f, 0.0f, -2.f));
        mary->setShadingMode(EShadingModel::Shading_BlinnPhong);
        mary->enableCastShadow(true);
        // mary->rotate(180.f, {0.f, 1.f, 0.f});
        mary->lookAt({0.f, 0.f, 0.f});
        focus = mary->getPosition();
        scene->addModel(mary);;

        // auto mary2 = UModel::makeModel(*mary);
        // mary2->setPosition({2.f, 0.f, 2.f});
        // mary2->setShadingMode(EShadingModel::Shading_BlinnPhong);
        // mary2->enableCastShadow(true);
        // mary2->lookAt({0.f, 0.f, 0.f});
        // scene->addModel(mary2);;
    }

    // skybox
    if (config.bSkybox) {
        if (auto skybox = loader.loadSkyBox("assets/texture/skybox_default")) {
            skybox->setPosition({0.f, 0.f, 0.f});
            skybox->setScale({100.f, 100.f, 100.f});
            skybox->setShadingMode(Shading_Skybox);
            scene->skybox_ = skybox;
        };;
    }

    // point light
    // shaders will be loaded by ShaderMode now, no longer needed to load manually.
    // here cube point light just use BaseColor.
    {
        // auto dirLight = ULight::makeLight(Cube::loadCubeMesh());
        // glm::vec3 dirLightPos {6, 10, -6};
        // dirLight->setAsDirectionalLight(focus - dirLightPos, glm::vec3(0.1f), glm::vec3(0.5f), glm::vec3(0.5f));
        // dirLight->setScale({ 0.2, 0.2, 0.2 });
        // dirLight->setPosition(dirLightPos);
        // scene->addLight(dirLight);;
    }

    {
        auto pointLight = ULight::makeLight(Cube::loadCubeMesh());
        pointLight->setAsPointLight(glm::vec3(0, 0, 0), glm::vec3(0.f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f), 1.0, 0.045, 0.0075);
        pointLight->setScale({ 0.2, 0.2, 0.2 });
        pointLight->setPosition({-3, 5, -3});
        scene->addLight(pointLight);
    }

    scene->setFocus(focus);
    return scene;
}

void FScene::addModel(std::shared_ptr<UModel> model)
{
	models_.emplace_back(std::move(model));
}

std::vector<std::shared_ptr<UModel>>& FScene::getModels()
{
	// TODO: 在此处插入 return 语句
	return models_;
}

void FScene::addLight(std::shared_ptr<ULight> light) {
	lights_.emplace_back(std::move(light));;
}

std::vector<std::shared_ptr<ULight>>& FScene::getLights() {
	return lights_;
}
