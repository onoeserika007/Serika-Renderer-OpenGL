#include "FScene.h"

#include "FCamera.h"
#include "ULight.h"
#include "Base/ResourceLoader.h"
#include "Geometry/BufferAttribute.h"
#include "Geometry/Primitives.h"

std::shared_ptr<FScene> FScene::generateDeaultScene(const std::shared_ptr<FCamera> &camera) {
	auto scene = std::make_shared<FScene>();
    auto&& loader = ResourceLoader::getInstance();
    auto&& config = Config::getInstance();
    glm::vec3 focus {};

    // nanosuit
    // TEST_TIME_COST(auto nanosuitMesh = loader.loadMesh("./assets/models/nanosuit/nanosuit.obj", true), NANO_SUIT);
    // nanosuitMesh->setShadingMode(EShadingModel::Shading_BlinnPhong);
    // nanosuitMesh->enableCastShadow(true);
    // auto nanosuit = UObject::makeObject();
    // nanosuit->setMesh(nanosuitMesh);
    // nanosuit->setScale(0.5, 0.5, 0.5);
    // nanosuit->setPosition(glm::vec3(0.f, -0.5f, 0.f));
    // focus = nanosuit->getWorldPosition();
    // scene->addObject(nanosuit);;

    // floor
    auto floorMesh = loader.loadMesh("./assets/models/floor/floor.obj", false);
    floorMesh->setShadingMode(EShadingModel::Shading_BlinnPhong);
    floorMesh->enableCastShadow(true);
    floorMesh->enableFaceCull(false);
    auto floor = UObject::makeObject();
    floor->setMesh(floorMesh);
    floor->setPosition({0.f, 0.0f, 0.f});
    // floor->setScale({0.1f, 0.1f, 0.1f});
    scene->addObject(floor);

    // marry 202 chyan
    if (auto maryMesh = loader.loadMesh("./assets/models/mary/mary.obj", false)) {
        // mary->setScale(0.1, 0.1, 0.1);
        maryMesh->setShadingMode(EShadingModel::Shading_BaseColor);
        maryMesh->enableCastShadow(true);

        auto mary = UObject::makeObject();
        mary->setMesh(maryMesh);
        mary->setPosition(glm::vec3(-2.f, 0.0f, -2.f));
        mary->rotate(270.f, {0.f, 1.f, 0.f});
        mary->lookAt({0.f, 0.f, 0.f});
        focus = mary->getWorldPosition();
        scene->addObject(mary);;
    }

    // skybox
    if (config.bSkybox) {
        if (auto skyboxMesh = loader.loadSkyBox("assets/texture/skybox_default")) {
            skyboxMesh->setShadingMode(Shading_Skybox);
            auto skyBox = UObject::makeObject();
            skyBox->setMesh(skyboxMesh);
            skyBox->setPosition({0.f, 0.f, 0.f});
            skyBox->setScale({100.f, 100.f, 100.f});
            scene->skybox_ = skyBox;
        };;
    }

    // point light
    // shaders will be loaded by ShaderMode now, no longer needed to load manually.
    // here cube point light just use BaseColor.
    {
     //    auto dirLight = ULight::makeLight();
	    // dirLight->setMesh(MeshMakers::loadCubeMesh());
     //    glm::vec3 dirLightPos {6, 10, -6};
     //    dirLight->setAsDirectionalLight(focus - dirLightPos, glm::vec3(0.1f), glm::vec3(0.5f), glm::vec3(0.5f));
     //    dirLight->setScale({ 0.2, 0.2, 0.2 });
     //    dirLight->setPosition(dirLightPos);
     //    scene->addLight(dirLight);;
    }

    {
        auto pointLight = ULight::makeLight();
	    pointLight->setMesh(MeshMakers::loadCubeMesh());
        pointLight->setAsPointLight(glm::vec3(0, 0, 0), glm::vec3(0.f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f), 1.0, 0.045, 0.0075);
        pointLight->setScale({ 0.2, 0.2, 0.2 });
        pointLight->setPosition({-3, 5, -3});
        scene->addLight(pointLight);
    }

    scene->setFocus(focus);
    return scene;;
}

std::shared_ptr<FScene> FScene::generateRaytracingStanfordBunnyScene(const std::shared_ptr<FCamera> &camera) {
    auto scene = std::make_shared<FScene>();
    auto&& loader = ResourceLoader::getInstance();
    auto&& config = Config::getInstance();
    glm::vec3 focus {};

    glm::vec3 lightColor = 15.6f * glm::vec3(0.740f+0.287f,0.740f+0.160f,0.740f);
    // point light
    {
        auto pointLightMesh = MeshMakers::loadCubeMesh();
        pointLightMesh->setEmission(GREEN_COLOR);
        auto pointLight = ULight::makeLight();
        pointLight->setMesh(pointLightMesh);
        pointLight->setAsPointLight(glm::vec3(0, 0, 0), glm::vec3(0.f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f), 1.0, 0.045, 0.0075);
        pointLight->setScale({ 0.2, 0.2, 0.2 });
        pointLight->setLightDiffuse(lightColor);
        pointLight->setPosition({-2, 2, -5});
        scene->addLight(pointLight);;
    }

    // {
    // auto dirLight = ULight::makeLight();
    // dirLight->setMesh(MeshMakers::loadCubeMesh());
    // glm::vec3 dirLightPos {6, 10, -6};
    // dirLight->setAsDirectionalLight(focus - dirLightPos, glm::vec3(0.1f), glm::vec3(0.5f), glm::vec3(0.5f));
    // dirLight->setScale({ 0.2, 0.2, 0.2 });
    // dirLight->setPosition(dirLightPos);
    // scene->addLight(dirLight);
    // }

    // bunny
    if (auto bunnyMesh = loader.loadMesh("assets/models/bunny/bunny.obj", false)) {
        bunnyMesh->setShadingMode(EShadingModel::Shading_BlinnPhong);
        bunnyMesh->enableCastShadow(true);
        auto bunny = UObject::makeObject();
        bunny->setMesh(bunnyMesh);
        bunny->setScale(10.f, 10.f, 10.f);
        // bunny->setPosition(glm::vec3(2.f, 5.f, 8.f));;
        bunny->setPosition(glm::vec3(-1.f, 0.0f, -4.f));;
        // mary->rotate(180.f, {0.f, 1.f, 0.f});
        // bunny->lookAt({0.f, 0.f, 0.f});
        bunny->bShowDebugBBox_ = true;
        focus = bunny->getWorldPosition();
        scene->addObject(bunny);
    }
    // floor
    {
        auto floorMesh = loader.loadMesh("./assets/models/floor/floor.obj", false);
        floorMesh->setShadingMode(EShadingModel::Shading_BlinnPhong);
        floorMesh->enableCastShadow(true);
        floorMesh->enableFaceCull(false);

        auto floor = UObject::makeObject();
        floor->setMesh(floorMesh);
        floor->setPosition({0.f, 0.0f, 0.f});
        // floor->setScale({0.1f, 0.1f, 0.1f});
        scene->addObject(floor);
    }

    scene->setFocus(focus);
    return scene;
}

std::shared_ptr<FScene> FScene::generatePBRScene(const std::shared_ptr<FCamera> &camera) {
    camera->setPosition({-2, 2, 1});
    auto scene = std::make_shared<FScene>();
    auto&& loader = ResourceLoader::getInstance();
    auto&& config = Config::getInstance();
    glm::vec3 focus {};

    auto material = std::make_shared<FMaterial>();
    material->setTextureData(TEXTURE_TYPE_DIFFUSE, "assets/texture/pbr/rusted_iron/albedo.png");
    material->setTextureData(TEXTURE_TYPE_AMBIENT_OCCLUSION, "assets/texture/pbr/rusted_iron/ao.png");
    material->setTextureData(TEXTURE_TYPE_METALNESS, "assets/texture/pbr/rusted_iron/metallic.png");
    material->setTextureData(TEXTURE_TYPE_NORMALS, "assets/texture/pbr/rusted_iron/normal.png");
    material->setTextureData(TEXTURE_TYPE_DIFFUSE_ROUGHNESS, "assets/texture/pbr/rusted_iron/roughness.png");
    auto sphereMesh = MeshMakers::loadSphereMesh();;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            auto&& mesh = sphereMesh->Clone();
            auto&& mat = material->Clone();
            mat->material_info_.uMetallic = 0.2f * i;
            mat->material_info_.uRoughness = 0.2f * j;
            mesh->setMaterial(mat);
            mesh->setDiffuse(RED_COLOR);
            auto&& object = UObject::makeObject();
            object->setMesh(mesh);
            object->setPosition(glm::vec3(i, j, 0) * 0.5f);
            object->setScale({0.2f, 0.2f, 0.2f});
            scene->addObject(object);
        }
    }
    // auto&& object = UObject::makeObject();
    // object->setMesh(sphereMesh);
    // scene->addObject(object);

    glm::vec3 lightColor = 15.6f * glm::vec3(0.740f+0.287f,0.740f+0.160f,0.740f);
    // point light
    {
        auto pointLightMesh = MeshMakers::loadCubeMesh();
        pointLightMesh->setEmission(GREEN_COLOR);
        auto pointLight = ULight::makeLight();
        pointLight->setMesh(pointLightMesh);
        pointLight->setAsPointLight(glm::vec3(0, 0, 0), glm::vec3(0.f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f), 1.0, 0.045, 0.0075);
        pointLight->setScale({ 0.2, 0.2, 0.2 });
        pointLight->setLightDiffuse({1.f, 1.f, 1.f});
        pointLight->setPosition({-2, 2, -5});
        scene->addLight(pointLight);;
    }

    scene->setFocus(focus);
    return scene;
}

std::shared_ptr<FScene> FScene::generateRaytracingCornellBoxScene(const std::shared_ptr<FCamera> &camera) {
    auto scene = std::make_shared<FScene>();
    auto&& loader = ResourceLoader::getInstance();
    auto&& config = Config::getInstance();
    glm::vec3 focus {};

    if (auto&& floor = loader.loadMesh("assets/models/cornellbox/floor.obj")) {
        floor->setDiffuse(WHITE_COLOR);
        auto obj = UObject::makeObject();
        obj->setMesh(floor);
        scene->addObject(obj);
    }

    if (auto&& shortbox = loader.loadMesh("assets/models/cornellbox/shortbox.obj")) {
        shortbox->setDiffuse(WHITE_COLOR);
        // shortbox->setPosition({-50.f, 0.f, -25.f});
        auto obj = UObject::makeObject();
        obj->setMesh(shortbox);
        scene->addObject(obj);
    }

    if (auto&& tallbox = loader.loadMesh("assets/models/cornellbox/tallbox.obj")) {
        tallbox->setDiffuse(WHITE_COLOR);
        auto obj = UObject::makeObject();
        obj->setMesh(tallbox);
        scene->addObject(obj);
    }

    if (auto&& left = loader.loadMesh("assets/models/cornellbox/left.obj")) {
        left->setDiffuse(WHITE_COLOR);
        auto obj = UObject::makeObject();
        obj->setMesh(left);
        scene->addObject(obj);
    }

    if (auto&& right = loader.loadMesh("assets/models/cornellbox/right.obj")) {
        right->setDiffuse(WHITE_COLOR);
        auto obj = UObject::makeObject();
        obj->setMesh(right);
        scene->addObject(obj);
    }

    if (auto&& lightMesh = loader.loadMesh("assets/models/cornellbox/light.obj")) {
        glm::vec3 lightEmit = (8.0f * glm::vec3(0.747f+0.058f, 0.747f+0.258f, 0.747f)
            + 15.6f * glm::vec3(0.740f+0.287f,0.740f+0.160f,0.740f)
            + 18.4f * glm::vec3(0.737f+0.642f,0.737f+0.159f,0.737f));
        glm::vec3 lightColor {0.65f};
        lightMesh->setEmission(lightEmit);
        lightMesh->setDiffuse(lightColor);

        auto light = ULight::makeLight();
        light->setMesh(lightMesh);
        light->setAsDirectionalLight({0, -1, 0}, glm::vec3(0.1f), glm::vec3(0.7f), glm::vec3(1.0f));
        scene->addLight(light);
        // scene->addMesh(light);; // the problem now has solved, this may be caused by some cpu compute error
    }

    // point light
    {
        // auto pointLight = ULight::makeLight();
        // pointLight->setMesh(MeshMakers::loadCubeMesh());
        // pointLight->setAsPointLight(glm::vec3(0, 0, 0), glm::vec3(0.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.5f), 0.05, 0, 0);
        // pointLight->setPosition({278, 278, 0});
        // scene->addLight(pointLight);
    }

    scene->camera_ = camera;
    camera->setPosition(278, 273, -700);
    scene->setFocus({278, 273, 0});

    return scene;
}

void FScene::addObject(std::shared_ptr<UObject> obj) { objects_.emplace_back(std::move(obj)); }

std::vector<std::shared_ptr<UObject>> &FScene::getObjects() { return objects_; }

void FScene::addLight(std::shared_ptr<ULight> light) { lights_.emplace_back(std::move(light)); }

void FScene::addBBox(const BoundingBox &bbox) { bboxes_.emplace_back(bbox); }

void FScene::addIntersectable(const std::shared_ptr<Intersectable> &inters) { intersectables_.emplace_back(inters); }

void FScene::setFocus(const glm::vec3 &focus) { focus_ = focus; }

std::vector<BoundingBox> & FScene::getBBoxes() { return bboxes_; }

const std::vector<std::shared_ptr<UMesh>> & FScene::getPackedMeshes() const { return packed_meshes_cache_; }

const std::vector<std::shared_ptr<UMesh>> & FScene::getPackedLightMeshes() const { return packed_lights_meshes_cache_; }

glm::vec3 FScene::getFocus() const { return focus_; }

std::vector<std::shared_ptr<ULight>>& FScene::getLights() { return lights_; }

void FScene::buildBVH() {
    for(auto&& model: objects_) {
        model->buildBVH();
        intersectables_.emplace_back(model);;
    }

    for(auto&& light: lights_) {
        light->buildBVH();
        intersectables_.emplace_back(light);
    }

    bvh_accel_ = std::make_shared<BVHAccel>(intersectables_);
}

std::shared_ptr<BVHAccel> FScene::getBVH() { return bvh_accel_; }

void FScene::setupScene() {
    for (auto&& light: lights_) {
        light->init();
    }

    for (auto&& mesh: objects_) {
        mesh->init();
    }

    buildBVH();
}

Intersection FScene::intersect(const Ray &ray) const {
    if (bvh_accel_) {
        return bvh_accel_->Intersect(ray);
    }
    LOGE("BVH not built, intersect scene failed!");
    return {};
}

void FScene::sampleLight(Intersection &outIsct, float &pdf) const {
    float emit_area_sum = 0;
    for (auto&& light: lights_) {
        emit_area_sum += light->getArea();
    }

    float split = MathUtils::get_random_float() * emit_area_sum;

    float sampled_area_sum = 0.f;
    for (auto&& light: lights_) {
        sampled_area_sum += light->getArea();
        if (sampled_area_sum >= split) {
            // do sample
            light->Sample(outIsct, pdf);
            break;;
        }
    }
    // if (emit_area_sum > 0.f) pdf = 1.f / emit_area_sum;
}

glm::vec3 FScene::castRay(const Ray &ray, int depth, int SobolIndex, float RussianRoulette) const {
    if (depth > 4) return{};

    // Dont have Intersection In Scene
    Intersection intersToScene = intersect(ray);
    if (!intersToScene.bHit) return {}; // early return

    // hit light
    if (intersToScene.hasEmission()) {
        return intersToScene.getEmission();
    }

    // hit diffuse, compute L_directly & L_indirectly
    glm::vec3 L_dir {};
    glm::vec3 L_indir {};

    // Calculate the Intersection from point to light in order to calculate direct Color
    Intersection LightPos;
    float lightpdf = 0.0f;
    sampleLight(LightPos, lightpdf);
    glm::vec3 LightDir = LightPos.impactPoint - intersToScene.impactPoint;
    glm::vec3 LightDirNormal = glm::normalize(LightDir);
    Ray rayToLight(intersToScene.impactPoint, LightDirNormal);

    // check if block by other object
    Intersection interToLight = intersect(rayToLight);
    auto f_r = intersToScene.evalRadiance(ray.direction, LightDirNormal, intersToScene.normal);
    // 由于精度问题，可能会出现黑色条纹，请务必使用较大的偏移量
    if (interToLight.traceDistance + 0.1f > glm::length(LightDir))
    {
        float dis2 = glm::dot(LightDir, LightDir);
        L_dir = LightPos.getEmission() * f_r * glm::dot(LightDirNormal, intersToScene.normal) * glm::dot(-LightDirNormal, LightPos.normal) / dis2 / lightpdf;
    }

    // RR exit
    if (MathUtils::get_random_float() > RussianRoulette) return L_dir;

    //Calculate the Intersection from point to point in order to calculate indirect Color
    glm::vec3 wo;
    float pdf;
    MathUtils::UniformHemisphereSample(wo, pdf, ray.direction, intersToScene.normal, depth, SobolIndex);
    Ray indirRay {intersToScene.impactPoint, wo};
    Intersection intersToDiffuse = intersect(indirRay);
    if( intersToDiffuse.bHit && !intersToDiffuse.hasEmission()) // 已经对光照采样过了，所以不能重复采样
    {
        L_indir = castRay(indirRay, depth + 1, SobolIndex)
        * intersToScene.evalRadiance(ray.direction, wo, intersToScene.normal)
        * glm::dot(wo,intersToScene.normal) / RussianRoulette / pdf;
    }

    return L_dir + L_indir ;
}

void FScene::packMeshesFromScene() {
    if (!bMeshCacheDirty) return;
    for (auto&& obj: objects_) {
        traverseToPackMesh(obj, packed_meshes_cache_);
    }

    for (auto&& light: lights_) {
        traverseToPackMesh(light, packed_lights_meshes_cache_);
    }

    bMeshCacheDirty = false;
}

void FScene::traverseToPackMesh(const std::shared_ptr<UObject> &obj, std::vector<std::shared_ptr<UMesh>> &cache_to_pack) {
    if (!obj) return;

    obj->updateFrame();
    if (obj->getMesh()) {
        cache_to_pack.emplace_back(obj->getMesh());
    }

    for (auto&& child: obj->getChildren()) {
        traverseToPackMesh(child, cache_to_pack);
    }
}
