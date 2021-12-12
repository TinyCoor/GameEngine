#version 450 core

uniform float fGlobalTime;
uniform vec2 v2Resolution;

uniform sampler1D texFFT;
uniform sampler1D texFFTSmoothed;
uniform sampler1D texFFTIntegrated;
uniform sampler2D texChecker;
uniform sampler2D texNoise;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;

layout(location =0) out vec4 out_color;
#define time fGlobalTime

float sph(vec3 p,float r)
{
    return length(p) -r;
}

float box(vec3 p, vec3 r)
{
    vec3 ap= abs(p) -r;
    return length(max(vec3(0),ap)) + min(0,max(ap.x,max(ap.y,ap.z)));
}

mat2 rot(float a)
{
    float ca = cos(a);
    float sa = sin(a);
    return mat2(ca,sa,-sa,ca);
}


float map(vec3 p)
{
    float t1 = time;
    p.xy *= rot(t1* 0.7);
    p.yz *= rot(t1);

    return box(p,vec3(0.5));
}

vec3 norm(vec3 p)
{
    vec2 off = vec2(0.01,0);
    return normalize(map(p) - vec3(map(p-off.xyy),map(p-off.yxy),map(p-off.yyx)));
}

void main()
{
    vec2 uv =vec2(gl_FragCoord.x/ v2Resolution.x,gl_FragCoord.y/v2Resolution.y);
    uv -= 0.5;
    uv /= vec2(v2Resolution.y/v2Resolution.x,1);

    vec3 s = vec3(0,0,-3);
    vec3 r = normalize(vec3(-uv,1));

    vec3 col = vec3(0);

    vec3 p = s;
    float dd=0.0;
    float side = 1;
    for(int i=0; i < 100; ++i)
    {
        float d =map(p) *side;
        if(d < 0.001) {

            vec3 n = norm(p) * side ;
            vec3 l = normalize(vec3(-1));
            vec3 h = normalize(l-r);

            col += max(0,dot(n,l)) * (0.3 +pow( max(0,dot(n,h)),10));
            side = -side;
            d = 0.01;

           // break;
        }
        if(d > 100){
            dd = 100;
            break;
        }
        p += r * d;
        dd += d;
    }



    out_color = vec4(col,l);


}