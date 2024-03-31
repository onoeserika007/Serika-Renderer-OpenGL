// #version 330 core

in vec2 TexCoord;
in vec3 worldNormal;
in vec3 fragPos;
in vec3 viewFragPos;

layout(std140) uniform Model {
    mat4 uModel;
    mat4 uView;
    mat4 uProjection;
    mat4 uNormalToWorld;
    vec3 viewPos;
};

#ifdef Texture_diffuse
    uniform sampler2D uTexture_diffuse;
#endif

#ifdef Texture_specular
    uniform sampler2D uTexture_specular;
#endif

#ifdef Texture_height
    uniform sampler2D uTexture_height;
#endif

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutoff;
    float outerCutoff;
};

#define MAX_LIGHT_NUM 4
#define MAX_TEXTURE_NUM 16
#define EPSILON 1e-3

uniform DirectionalLight dirLightArray[MAX_LIGHT_NUM];
uniform PointLight pointLightArray[MAX_LIGHT_NUM];
uniform SpotLight spotLightArray[1];

mat3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);
mat3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float calcSpotLight(SpotLight light, vec3 fragPos);

float shininess = 32.0f;

void main()
{
    // vec2 spotlightCoord = vec2(normalize(viewFragPos)) / 0.5 + vec2(1.0f);
    // vec3 spotlightDiffuse = vec3(texture(material.emissive, spotlightCoord));

    vec3 norm = normalize(worldNormal);
    vec3 viewDir = normalize(viewPos - fragPos);

    // vec3 emissive = vec3(texture(material.emissive, TexCoord));
    // mat3 result = mat3(0.0f);
    // float spotLightIntensity = 0.0;

    // for(int i=0; i < MAX_LIGHT_NUM; i++) {
    //     result += calcDirLight(dirLightArray[i], norm, viewDir);
    //     result += calcPointLight(pointLightArray[i], norm, fragPos, viewDir);
    // }

    // mat3 color = mat3(0.0f);
    // color += calcPointLight(pointLightArray[0], norm, fragPos, viewDir);
    // for(int i=0; i<100; i++) {
    //     result += color;
    // }

    // spotLightIntensity += calcSpotLight(spotLightArray[0], fragPos);
    // result[1] *= spotLightIntensity;
    // result[2] *= spotLightIntensity;

    // gl_FragColor = vec4(result[0] + result[1] + result[2] + spotlightDiffuse, 1.0);
    // gl_FragColor = vec4(result[0] + result[1] + result[2], 1.0);
    // gl_FragColor = texture(material.texture_diffuse0, TexCoord);
#ifdef Texture_diffuse
    gl_FragColor = texture(uTexture_diffuse, TexCoord);
#else
    gl_FragColor = vec4(1.0f);
#endif
}

// mat3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
//     vec3 lightDir = normalize(-light.direction);
//     // 漫反射着色
//     float diff = max(dot(normal, lightDir), 0.0);
//     // 镜面光着色
//     vec3 reflectDir = reflect(-lightDir, normal);
//     float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
//     // 合并结果
//     vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse0, TexCoord));
//     vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse0, TexCoord));
//     vec3 specular = vec3(0.0f);
//     if(texture_specular_valid[0]) {
//         specular = light.specular * spec * vec3(texture(material.texture_specular0, TexCoord));
//     }
//     // return mat3(ambient, diffuse, vec3(0.0f));
//     return mat3(ambient, diffuse, specular);
// }

// mat3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
//     if(abs(light.constant) < EPSILON) {
//         light.constant = 1.0f;
//     }
//     vec3 lightDir = normalize(light.position - fragPos);
//     // 漫反射着色
//     float diff = max(dot(normal, lightDir), 0.0);
//     // 镜面光着色
//     vec3 reflectDir = reflect(-lightDir, normal);
//     float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//     // 衰减
//     float distance    = length(light.position - fragPos);
//     // division by zero problem here!
//     float attenuation = 1.0 / (light.constant + light.linear * distance + 
//                  light.quadratic * (distance * distance));    
//     // 合并结果
//     vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse0, TexCoord));
//     vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse0, TexCoord));
//     vec3 specular = vec3(0.0f);
//     if(texture_specular_valid[0]) {
//         specular = light.specular * spec * vec3(texture(material.texture_specular0, TexCoord));
//     }
//     ambient  *= attenuation;
//     diffuse  *= attenuation;
//     specular *= attenuation;
//     // return mat3(ambient, diffuse, specular);
//     return mat3(ambient, diffuse, specular);
// }

// float calcSpotLight(SpotLight light, vec3 fragPos) {
//     vec3 lightDir = normalize(light.position - fragPos); // from frag to light
//     float theta = dot(normalize(-lightDir), light.direction);
//     float epsilon = light.cutoff - light.outerCutoff; // inner cos - outer cos
//     // zero division
//     if(abs(epsilon) < EPSILON) {
//         epsilon = 1.0f;
//     }
//     float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0f, 1.8f);
//     return intensity;
// }