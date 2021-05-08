varying vec3 normal; 
void main() 
{ 
   normal = gl_Normal; //color = gl_Color; gets the color (irrelevant here)
   gl_Position = ftransform(gl_Vertex); 
}
