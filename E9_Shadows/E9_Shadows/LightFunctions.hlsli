// requires attenuation and light type additions to light.h

// notes and lighting calculations based on and adapted: https://www.3dgep.com/texturing-lighting-directx-11/#Light_Properties
// edit as required for scene
#define MAX_LIGHTS 4
// Light types. (must match enum from app1)
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct LightProps
{
    float4 ambient; //move to material struct
    float4 diffuse; //move to material struct (change to colour)
    //float4 colour;
    float3 lightDirection;
    float cone;
    float3 position;
    float specularPower;
    float4 specularColour;
	
	// attenuation
    float constantFactor;
    float linearFactor;
    float quadraticFactor;
    float attenPadding;
    // light type
    int lightType; //4bytes
    bool enabled; //4bytes
    int typePadding0; //4bytes
    int typePadding1; //4bytes
    
    // total = 112 bytes
};

// used to store light variables that will be added together for final light results
struct LightCalcResult
{
    float4 diffuse;
    float4 specular;
};

// multiply to diffuse factor before ambient and texture values (point and spotLights)
float calculateAttenuation(LightProps light, float3 worldPos)
{
    float lightDist = distance(light.position, worldPos);
    //float attenuation = 1 / (constantFactor + (linearFactor * lightDist) + (quadraticFactor * pow(lightDist, 2)));
    return 1 / (light.constantFactor + (light.linearFactor * lightDist) + (light.quadraticFactor * pow(lightDist, 2)));
}

// calculates diffuse lighting contribution
float4 calculateDiffuse(float4 lColour, float3 lightVec, float3 norm)
{
    float lightIntensity = max(0, dot(norm, lightVec));
    return lColour * lightIntensity;
}

//calculates speculat lighting contribution
float4 calculateSpecular(LightProps light, float3 lightVec, float3 norm, float3 viewVec)
{
    // phong lighting
    float3 reflectionP = normalize(reflect(-lightVec, norm));
    //reflec dot view
    float phong = max(0, dot(reflectionP, viewVec));
    
    //blinn -Phong
    float3 halfAngle = normalize(lightVec + viewVec);
    //norm dot half
    float blinnPhong = max(0, dot(norm, halfAngle));
    
    return light.diffuse * pow(blinnPhong, light.specularPower);

}

// need to clamp returned values for all light calculations

LightCalcResult calculatePointLight(LightProps light, float3 norm, float3 worldPos, float3 viewVec)
{
    LightCalcResult result;
    float3 lightVector = normalize(light.position - worldPos);
    float atten = calculateAttenuation(light, worldPos);
    
    result.diffuse = calculateDiffuse(light.diffuse, lightVector, norm) * atten;
    result.specular = calculateSpecular(light, lightVector, norm, viewVec) * atten;
    return result;
}

LightCalcResult calculateDirectionLight(LightProps light, float3 norm, float3 viewVec)
{
    LightCalcResult result;
    float3 lightVector = -light.lightDirection.xyx;
    result.diffuse = calculateDiffuse(light.diffuse, lightVector, norm);
    result.specular = calculateSpecular(light, lightVector, norm, viewVec);
    return result;
}

float calculateSpotCone(LightProps light, float3 lightVec)
{
    float minCos = cos(radians(light.cone));
    float maxCos = (minCos + 1.0f) / 2.0f;
    float cosAngle = dot(light.lightDirection.xyz, -lightVec);
    return smoothstep(minCos, maxCos, cosAngle);
}

LightCalcResult calcualteSpotLight(LightProps light, float3 norm, float3 worldPos, float3 viewVec)
{
    LightCalcResult result;
    float3 lightVector = normalize(light.position - worldPos);
    
    float atten = calculateAttenuation(light, worldPos);
    float spotIntensity = calculateSpotCone(light, lightVector);
    
    result.diffuse = calculateDiffuse(light.diffuse, lightVector, norm) * atten * spotIntensity;
    result.specular = calculateSpecular(light, lightVector, norm, viewVec) * atten * spotIntensity;
    
    return result;
}