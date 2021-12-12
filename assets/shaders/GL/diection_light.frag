#version 450


uniform vec3 ambient;
uniform vec3 light_color;
uniform vec3 light_direction;
uniform vec3 half_vector;
uniform float Shininess;
uniform float Strength;


in vec4 Color;
in vec3 Normal;

void main()
{
    //计算方向的余玄
    float diffsue = max(0.0,dot(Normal,light_direction));
    float specular = max(0.0,dot(Normal,HalfVector));

    if(diffsue == 0.f)
        specular =0.f;
    else
        specular = pow(specular,Shininess);

    vec3 scatterredLight = ambient + light_color *diffsue;
    vec3 reflectedLight = LightColor *specular *Strength;

    vec3 rgb = min(Color.rgb* scatterredLight + reflectedLight,vec3(1.0));
    FragColor=vec4(rgb,Color.a);



}

