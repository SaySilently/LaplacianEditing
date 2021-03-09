#version 330 core
out vec4 FragColor;

in vec3 vertex_color;
in vec3 Normal;
in vec3 FragPos;
uniform vec3 viewPos;


void main()
{
   
    // ambient
    float ambientStrength = 0.5;
    vec3 lightColor = vec3(0.7, 0.7, 0.7);
    vec3 ambient = ambientStrength * lightColor;
    
    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightPos = vec3(5.0f, 5.0f,  5.0f);
    vec3 lightPos1 = vec3(0.0f, 0.0f, 5.0f);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 lightDir1 = normalize(lightPos1 - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float diff1 = max(dot(norm, lightDir1), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 diffuse1 = diff1 * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + diffuse1) * vertex_color;
    FragColor = vec4(result, 1.0);
    
}
