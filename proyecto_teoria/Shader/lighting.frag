#version 330 core

#define NUMBER_OF_POINT_LIGHTS 13   // 0 destello del railgun, 1 casco de CUAU, 2..12 veladoras

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight
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

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform int transparency;
uniform float emissive;   // 0..1: suma el color propio de la textura (partes que "se encienden": visor, luces del casco)
uniform float brillo;     // multiplica la iluminacion (1 = normal; <1 = pieza apagada). main lo pone en 1 cada cuadro

uniform samplerCube envMap;   // cubemap del skybox (unidad 10): environment mapping
uniform int   envKind;        // 0 = solo Phong | 1 = metal (reflejo + Fresnel) | 2 = cristal (reflexion + refraccion + Fresnel)
uniform float envOn;          // 1 = efecto activo, 0 = solo Phong (comparar antes/despues)

// Fresnel de Schlick: F = F0 + (1 - F0) * (1 - cos(theta))^5
float Fresnel(float cosT, float F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosT, 0.0, 1.0), 5.0);
}

// Function prototypes
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir );
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir );
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir );

void main( )
{
    // Properties
    vec3 norm = normalize( Normal );
    vec3 viewDir = normalize( viewPos - FragPos );
    
    // Directional lighting
    vec3 result = CalcDirLight( dirLight, norm, viewDir );
    
    // Point lights
    for ( int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++ )
    {
        result += CalcPointLight( pointLights[i], norm, FragPos, viewDir );
    }
    
    // Spot light
    result += CalcSpotLight( spotLight, norm, FragPos, viewDir );
 	
    vec4 albedo = texture(material.diffuse, TexCoords);
    vec3 phong = result * brillo;
    float alpha = albedo.a;

    if (envOn > 0.5 && envKind != 0)
    {
        vec3 I = -viewDir;                       // del ojo al fragmento
        float cosT = max(dot(norm, viewDir), 0.0);
        vec3 refl = texture(envMap, reflect(I, norm)).rgb;
        if (envKind == 1)
        {
            // Metal: F0 alto y teñido por el color del material
            vec3 F0 = mix(vec3(0.25), albedo.rgb, 0.35);
            vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosT, 5.0);
            phong = mix(phong, refl * mix(vec3(1.0), albedo.rgb * 1.6, 0.5), min(F * 1.2, vec3(0.9)));
        }
        else
        {
            // Cristal: refraccion (con ligera dispersion cromatica) mezclada con reflexion segun Fresnel
            vec3 refr = vec3(texture(envMap, refract(I, norm, 1.0 / 1.50)).r,
                             texture(envMap, refract(I, norm, 1.0 / 1.52)).g,
                             texture(envMap, refract(I, norm, 1.0 / 1.54)).b);
            float F = Fresnel(cosT, 0.04);
            vec3 env = mix(refr * albedo.rgb * 1.5, refl, F);
            phong = mix(phong * 0.5, env, 0.9);
        }
    }
    color = vec4(phong + emissive * albedo.rgb, alpha);
	  if(color.a < 0.1 && transparency==1)
        discard;

}

// Calculates the color when using a directional light.
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir )
{
    vec3 lightDir = normalize( -light.direction );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    // Combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, TexCoords ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, TexCoords ) );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, TexCoords ) );
    
    return ( ambient + diffuse + specular );
}

// Calculates the color when using a point light.
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - fragPos );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    // Attenuation
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + light.linear * distance + light.quadratic * ( distance * distance ) );
    
    // Combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, TexCoords ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, TexCoords ) );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, TexCoords ) );
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ( ambient + diffuse + specular );
}

// Calculates the color when using a spot light.
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - fragPos );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    // Attenuation
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + light.linear * distance + light.quadratic * ( distance * distance ) );
    
    // Spotlight intensity
    float theta = dot( lightDir, normalize( -light.direction ) );
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp( ( theta - light.outerCutOff ) / epsilon, 0.0, 1.0 );
    
    // Combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, TexCoords ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, TexCoords ) );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, TexCoords ) );
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return ( ambient + diffuse + specular );
}