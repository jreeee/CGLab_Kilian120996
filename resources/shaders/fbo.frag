#version 150

in vec2 TexCoord;

uniform int Sfx;

uniform sampler2D texFramebuffer;

out vec4 outColor;

//has to be done because TexCoord is read only
vec2 TxC = TexCoord;

void main()
{
    //horizontal mirroring
    if (Sfx % 3 == 0) {
        TxC.y = 1.0f - TxC.y;
    }
    //vertical mirroring
    if (Sfx % 5 == 0) {
        TxC.x = 1.0f - TxC.x;
    }
    //introducing res so we can overwrite it later
    vec3 res = vec3(texture(texFramebuffer, TxC)); 
    //blur
    if (Sfx % 7 == 0) {
        //distance for the gaussian blur
        const float DIST = 1.0f/500.0f;
        //3x3 fir kernel
        const float kernel[9] = float[](  
                                    1.0f/16.0f, 1.0f/8.0f, 1.0f/16.0f,
                                    1.0f/8.0f,  1.0f/4.0f, 1.0f/8.0f,
                                    1.0f/16.0f, 1.0f/8.0f, 1.0f/16.0f);
        //3x3 pixelgrid, corresponding to the kernel
        const vec2 pixelgrid[9] = vec2[]( 
                                    vec2(-DIST, DIST),  vec2(0.0f, DIST),    vec2(DIST, DIST),
                                    vec2(-DIST, 0.0f),   vec2(0.0f, 0.0f),     vec2(DIST, 0.0f),
                                    vec2(-DIST, -DIST), vec2(0.0f, -DIST),   vec2(DIST, -DIST));
        //setting the res to black
        res = vec3(0.0f, 0.0f, 0.0f);
        //adding up all pixel color values multiplied by their respective kernel value 
        for (int i = 0; i < kernel.length(); ++i) {
            res += kernel[i] * vec3(texture(texFramebuffer, TxC.st + pixelgrid[i]));
        }
    }
    //greyscale
    if (Sfx % 2 == 0) {
        //multiplying the colors with specific values to preserve the luminance and get an average
        float avg = 0.2126f * res.r + 0.7152f * res.g + 0.0722f * res.b;
        res = vec3(avg, avg, avg);
    }

    outColor = vec4(res, 1.0f);
}