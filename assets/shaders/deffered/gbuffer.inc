#ifndef GBUFFER_H_
#define GBUFFER_H_

layout(set=GBUFFER_SET,binding=0) uniform sampler2D gbufferBaseColorSampler;
layout(set=GBUFFER_SET,binding=1) uniform sampler2D gbufferDepthSampler;
layout(set=GBUFFER_SET,binding=2) uniform sampler2D gbufferNormalSampler;
layout(set=GBUFFER_SET,binding=3) uniform sampler2D gbufferShadingSampler;



#endif
