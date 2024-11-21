#version 330 core
struct Material 
{
    //material vectors
    //vec3 ambient; //defines what color the surface reflects under ambient lighting, usually same as surface color
    sampler2D diffuse; //defines the color of the surface under diffuse lighting. diffuse color is set to the desired surface color (like ambient lighting)
    sampler2D specular; //specular sets the color of the specular highlight on the surface (or maybe even reflect a surface-specific color)
    float shininess; //shininess impacts scattering/radius of the specular highlight
};

//a light source has a different intensity for its components. ambient light is usually set to a low intensity so the ambient color isnt too dominant.
//the diffuse color of a light source is set to the color we want to have, usually a bright white color. specular is usually kept at vec3(1.0) shining a tfull intensity.

struct DirectionalLight
{
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;

    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 4  

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutoff;
    float outer_cutoff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;  
};

in vec3 normal;  
in vec3 frag_pos;
in vec2 tex_coords;
  
//uniform vec3 object_color;
//uniform vec3 light_color;
//uniform vec3 light_pos;
uniform vec3 view_pos;
uniform Material material;
uniform DirectionalLight directional_light;
uniform PointLight point_lights[NR_POINT_LIGHTS];
uniform SpotLight spot_light;

out vec4 frag_color;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_dir);
vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir);
vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 frag_pos, vec3 view_dir);

void main()
{   
    vec3 norm = normalize(normal);
    vec3 view_dir = normalize(view_pos - frag_pos);

    //directional lighting
    vec3 result = CalculateDirectionalLight(directional_light, norm, view_dir);
    //point lighting
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalculatePointLight(point_lights[i], norm, frag_pos, view_dir);
    //spot light
    result += CalculateSpotLight(spot_light, norm, frag_pos, view_dir);

    frag_color = vec4(result, 1.0);
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(-light.direction);
    //diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    //specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    //combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, tex_coords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, tex_coords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, tex_coords));
    return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);
    //diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    //specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    //attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    //combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, tex_coords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, tex_coords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, tex_coords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);
    //diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    //specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    
    //soft edges for spotlight
    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

    //attenuation and distance
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    //combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, tex_coords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, tex_coords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, tex_coords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}