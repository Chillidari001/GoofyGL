#version 330 core
struct Material 
{
    //material vectors
    //vec3 ambient; //defines what color the surface reflects under ambient lighting, usually same as surface color
    sampler2D diffuse; //defines the color of the surface under diffuse lighting. diffuse color is set to the desired surface color (like ambient lighting). texture based
    sampler2D specular; //specular sets the color of the specular highlight on the surface (or maybe even reflect a surface-specific color). texture based

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

out vec4 frag_color;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_dir);
vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir);
vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 frag_pos, vec3 view_dir);

void main()
{   
    vec3 norm     = normalize(normal);
    vec3 view_dir = normalize(view_pos - frag_pos);

    // 1) directional lighting
    vec3 result = CalculateDirectionalLight(directional_light, norm, view_dir);
    
    // 2) point lighting
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalculatePointLight(point_lights[i], norm, frag_pos, view_dir);
    
    // 3) spotlight
    result += CalculateSpotLight(spot_light, norm, frag_pos, view_dir);

    // If wireframe is on, we force white lines
    if(wireframe)
        frag_color = vec4(1.0);
    else
        frag_color = vec4(result, 1.0);
}

// ------------------------------------------------------------------------
// DIRECTIONAL LIGHT
// ------------------------------------------------------------------------
vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(-light.direction);

    // 1) DIFFUSE shading
    float diff = max(dot(normal, light_dir), 0.0);

    // 2) SPECULAR shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    // 3) Sample textures *and* multiply by color
    vec3 base_diffuse  = texture(material.diffuse, tex_coords).rgb * material.diffuse_color;
    vec3 base_specular = texture(material.specular, tex_coords).rgb * material.specular_color;
    
    // For ambient, many engines use the “diffuse texture” as well, 
    // but you could also do:  light.ambient * material.ambient_color
    // We'll stay consistent with your approach (using baseDiffuse).
    vec3 ambient  = light.ambient  * base_diffuse;
    vec3 diffuse  = light.diffuse  * diff * base_diffuse;
    vec3 specular = light.specular * spec * base_specular;

    return (ambient + diffuse + specular);
}

// ------------------------------------------------------------------------
// POINT LIGHT
// ------------------------------------------------------------------------
vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    // 1) Light direction & basic diffuse/spec
    vec3 light_dir   = normalize(light.position - frag_pos);
    float diff       = max(dot(normal, light_dir), 0.0);

    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec       = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    // 2) Combine texture + color
    vec3 base_diffuse  = texture(material.diffuse, tex_coords).rgb * material.diffuse_color;
    vec3 base_specular = texture(material.specular, tex_coords).rgb * material.specular_color;

    // 3) Basic ambient/diffuse/spec
    vec3 ambient  = light.ambient  * base_diffuse;
    vec3 diffuse  = light.diffuse  * diff * base_diffuse;
    vec3 specular = light.specular * spec * base_specular;

    // 4) Attenuation
    float distance    = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant 
                               + light.linear * distance 
                               + light.quadratic * (distance * distance));

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

// ------------------------------------------------------------------------
// SPOT LIGHT
// ------------------------------------------------------------------------
vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);

    // 1) Basic diffuse/spec
    float diff       = max(dot(normal, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec       = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    // 2) Combine texture + color
    vec3 base_diffuse  = texture(material.diffuse, tex_coords).rgb * material.diffuse_color;
    vec3 base_specular = texture(material.specular, tex_coords).rgb * material.specular_color;

    // 3) Basic ambient/diffuse/spec
    vec3 ambient  = light.ambient  * base_diffuse;
    vec3 diffuse  = light.diffuse  * diff * base_diffuse;
    vec3 specular = light.specular * spec * base_specular;

    // 4) Spotlight angle (soft edges)
    float theta   = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

    // 5) Attenuation
    float distance    = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant 
                               + light.linear * distance 
                               + light.quadratic * (distance * distance));

    // 6) Combine spotlight intensity + attenuation
    ambient  *= (attenuation * intensity);
    diffuse  *= (attenuation * intensity);
    specular *= (attenuation * intensity);

    return (ambient + diffuse + specular);
}