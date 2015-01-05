varying vec4 vertexPosition;
uniform float time;

// makes a pseudorandom number between 0 and 1
float hash(float n) {
  return fract(sin(n)*93942.234);
}

// smoothsteps a grid of random numbers at the integers
float noise(float x, float y) {
  vec2 pos = vec2(x, y);
  vec2 w = floor(pos);
  vec2 k = fract(pos);
  k = k*k*(3.-2.*k); // smooth it
  
  float n = w.x + w.y*57.;
  
  float a = hash(n);
  float b = hash(n+1.);
  float c = hash(n+57.);
  float d = hash(n+58.);
  
  return mix(
    mix(a, b, k.x),
    mix(c, d, k.x),
    k.y);
}

void main(){
    float val = noise(vertexPosition.x * time, vertexPosition.y * time);
    vec3 color = vec3(0.5, 0.5, val);

    gl_FragColor = vec4(color.rgb, 1.0);
}
