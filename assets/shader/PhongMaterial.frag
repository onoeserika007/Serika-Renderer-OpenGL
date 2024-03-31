#version 330 core

in vec2 TexCoord;
in vec3 worldNormal;
in vec3 fragPos;

uniform vec3 objectColor;
uniform vec3 lightColor;
// uniform vec3 lightPos;
uniform vec3 viewPos;
// uniform sampler2D texture1;
// uniform sampler2D texture2;
// uniform float mixValue;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

void main()
{
    vec3 ambient = material.ambient * light.ambient;

    vec3 normal = normalize(worldNormal);
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * material.diffuse;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal); // reflect函数要求第一个向量是从光源指向片段位置的向量
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // 没有使用半程向量
    vec3 specular = light.specular * (spec * material.specular); // 定义了RGB的反射系数就相当于定义了材质颜色了

    vec3 result = ambient + diffuse + specular;

    // vec3 fragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixValue).xyz;
    // gl_FragColor = vec4(fragColor * lightColor * (ambient + diff+ spec), 1.0);
    gl_FragColor = vec4(result, 1.0);
}