varying vec2 vUv;
varying float noise;
uniform sampler2D wave;

float random(vec3 scale, float seed){
    return fract(sin(dot(gl_FragCoord.xyz + seed, scale)) + seed);
}

void main(){
    float r = 0.0001 * random(vec3(0.0001), 0.0);
    vec2 tPos = vec2(0, noise + r);
    vec4 color = texture2D(wave, tPos);

    gl_FragColor = vec4(color.rgb, 1.0);
}
