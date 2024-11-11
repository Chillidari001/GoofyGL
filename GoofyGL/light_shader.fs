#version 330 core
struct Material {
    //material vectors
    //vec3 ambient; //defines what color the surface reflects under ambient lighting, usually same as surface color
    sampler2D diffuse; //defines the color of the surface under diffuse lighting. diffuse color is set to the desired surface color (like ambient lighting)
    vec3 specular; //specular sets the color of the specular highlight on the surface (or maybe even reflect a surface-specific color)
    float shininess; //shininess impacts scattering/radius of the specular highlight
};

//a light source has a different intensity for its components. ambient light is usually set to a low intensity so the ambient color isnt too dominant.
//the diffuse color of a light source is set to the color we want to have, usually a bright white color. specular is usually kept at vec3(1.0) shining a tfull intensity.
struct Light {
    vec3 position; 
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 normal;  
in vec3 frag_pos;
in vec2 tex_coords;
  
uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;
uniform Material material;
uniform Light light;

out vec4 frag_color;

void main()
{
    //ambient
    vec3 ambient = light.diffuse * vec3(texture(material.diffuse, tex_coords));
    
    //diffuse
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, tex_coords));

    //specular
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);
    
    vec3 result = ambient + diffuse + specular;
    frag_color = vec4(result, 1.0);
}