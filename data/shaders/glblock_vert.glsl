void main() {
  gl_FrontColor = gl_Color;
  gl_Position = ftransform();

  gl_TexCoord[0].st = gl_Vertex.xy;
  gl_TexCoord[0].pq = gl_MultiTexCoord0.xy;
}
