#version 450


uniform vec3 ambient;
uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3  half_vector;
uniform float Shininess;
uniform float Strength;

uniform vec3 EyeDirection;
uniform float ConstantAttenuation; //衰减系数
uniform float LinearAttenuation;
uniform float QuadraticAttenuation;

in vec4 Color;
in vec3 Normal;
in vec4 Position;

out vec4 FragColor;


void main()
{
    //获取光照的方向和距离
    vec3 lightDirection = light_position -vec3(Position);
    float distance = length(lightDirection);

    //规一化
    lightDirection= lightDirection /distance;

    //判断当前片元是不是在光源方位内
    float attenuation = 1.0 / (ConstantAttenuation + LinearAttenuation * distance + QuadraticAttenuation * distance + distance);

    //每个片元的最大高光方向不断变化的
    vec3 half_vector = normalize(lightDirection + EyeDirection);

    float diffsue = max(0.0,dot(Normal,lightDirection));
    float specular = max(0.0,dot(Normal,half_vector));

    if(diffsue == 0.0)
        specular = 0.0;
    else
        specular = pow(specular,Shininess * Strength);

    vec3 scatterredLight = ambient + light_color * diffsue * attenuation;
    vec3 reflectedLight = light_color * specular * attenuation;

    vec3 rgb = min(Color.rgb * scatterredLight + reflectedLight,vec3(1.0));
    FragColor = vec4(rgb,Color.a);

}

