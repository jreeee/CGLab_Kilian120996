#version 150

const float DIST = 1.0f/500.0f;

in vec2 TexCoord;

uniform int Sfx;

uniform sampler2D texFramebuffer;

out vec4 outColor;

vec2 TxC = TexCoord;

void main()
{
    if (Sfx % 3 == 0) {
        TxC.y = 1.0f - TxC.y;
    }
    if (Sfx % 5 == 0) {
        TxC.x = 1.0f - TxC.x;
    }
    vec3 res = vec3(texture(texFramebuffer, TxC)); 
    
    if (Sfx % 7 == 0) {
        float kernel[9] = float[](  1.0f/16.0f, 1.0f/8.0f, 1.0f/16.0f,
                                    1.0f/8.0f,  1.0f/4.0f, 1.0f/8.0f,
                                    1.0f/16.0f, 1.0f/8.0f, 1.0f/16.0f);

        vec2 pixelgrid[9] = vec2[]( vec2(-DIST, DIST),  vec2(0.0f, DIST),    vec2(DIST, DIST),
                                    vec2(-DIST, 0.0f),   vec2(0.0f, 0.0f),     vec2(DIST, 0.0f),
                                    vec2(-DIST, -DIST), vec2(0.0f, -DIST),   vec2(DIST, -DIST));

        res = vec3(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < kernel.length(); ++i) {
            res += kernel[i] * vec3(texture(texFramebuffer, TxC.st + pixelgrid[i]));
        }
    }

    if (Sfx % 2 == 0) {
        float avg = 0.2126f * res.r + 0.7152f * res.g + 0.0722f * res.b;
        res = vec3(avg, avg, avg);
    }

    outColor = vec4(res, 1.0f);
}