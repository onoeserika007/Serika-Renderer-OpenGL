#version 330 core

in vec2 TexCoord;
in vec3 worldNormal;
in vec3 fragPos;

uniform vec3 viewPos;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emissive;
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

uniform float setValue;

void main()
{
    vec2 emissive_texcoord = TexCoord + vec2(0.0, setValue * 0.5);
    vec3 emissive = vec3(texture(material.emissive, emissive_texcoord));
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoord));

    vec3 normal = normalize(worldNormal);
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, TexCoord));

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal); // reflect函数要求第一个向量是从光源指向片段位置的向量
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // 没有使用半程向量
    vec3 specular = light.specular * (spec * vec3(texture(material.specular, TexCoord)));

    vec3 result = ambient + diffuse + specular + emissive;

    gl_FragColor = vec4(result, 1.0);
}