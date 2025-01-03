#version 430 core
struct Material 
{
    //material vectors
    //vec3 ambient; //defines what color the surface reflects under ambient lighting, usually same as surface color

    vec3 diffuse_color;
    vec3 ambient_color;
    vec3 specular_color; 

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
uniform bool wireframe;
//samplers outside of material struct now because thats not allowed in later versions?
uniform sampler2D material_diffuse; //defines the color of the surface under diffuse lighting. diffuse color is set to the desired surface color (like ambient lighting). texture based
uniform sampler2D material_specular; //specular sets the color of the specular highlight on the surface (or maybe even reflect a surface-specific color). texture based

out vec4 frag_color;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_dir);
vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir);
vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 frag_pos, vec3 view_dir);

void main()
{   
    vec3 norm     = normalize(normal);
    vec3 view_dir = normalize(view_pos - frag_pos);

    // 1) Directional lighting
    vec3 result = CalculateDirectionalLight(directional_light, norm, view_dir);
    
    // 2) Point lighting
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalculatePointLight(point_lights[i], norm, frag_pos, view_dir);
    
    // 3) Spotlight
    result += CalculateSpotLight(spot_light, norm, frag_pos, view_dir);

    // If wireframe is on, force white color
    if(wireframe)
        frag_color = vec4(1.0);
    else
        frag_color = vec4(result, 1.0);
}

// Calculate directional light
vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(-light.direction);

    // Diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);

    // Specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    // Sample textures and multiply by color
    vec3 base_diffuse  = texture(material_diffuse, tex_coords).rgb * material.diffuse_color;
    vec3 base_specular = texture(material_specular, tex_coords).rgb * material.specular_color;

    // Calculate ambient, diffuse, and specular components
    vec3 ambient  = light.ambient  * base_diffuse;
    vec3 diffuse  = light.diffuse  * diff * base_diffuse;
    vec3 specular = light.specular * spec * base_specular;

    return (ambient + diffuse + specular);
}

// Calculate point light
vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);
    float diff = max(dot(normal, light_dir), 0.0);

    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    vec3 base_diffuse  = texture(material_diffuse, tex_coords).rgb * material.diffuse_color;
    vec3 base_specular = texture(material_specular, tex_coords).rgb * material.specular_color;

    vec3 ambient  = light.ambient  * base_diffuse;
    vec3 diffuse  = light.diffuse  * diff * base_diffuse;
    vec3 specular = light.specular * spec * base_specular;

    // Attenuation
    float distance    = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

// Calculate spotlight
vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);

    float diff = max(dot(normal, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    vec3 base_diffuse  = texture(material_diffuse, tex_coords).rgb * material.diffuse_color;
    vec3 base_specular = texture(material_specular, tex_coords).rgb * material.specular_color;

    vec3 ambient  = light.ambient  * base_diffuse;
    vec3 diffuse  = light.diffuse  * diff * base_diffuse;
    vec3 specular = light.specular * spec * base_specular;

    // Spotlight angle
    float theta   = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

    // Attenuation
    float distance    = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Combine results
    ambient  *= attenuation * intensity;
    diffuse  *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}