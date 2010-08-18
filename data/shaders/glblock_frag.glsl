void main() {
  vec4 color = gl_Color * gl_TexCoord[0].t;
  color.r = pow(color.r, 0.5);
  gl_FragColor = color;
}
