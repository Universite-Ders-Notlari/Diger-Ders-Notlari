uniform vec3 lightDir; //same for each vertex
varying float intensity; //output of VS that will then go into the FS
void main() //all vertices call here
{ 
   vec3 ld; 
   intensity = dot(lightDir,gl_Normal); 
   gl_Position = ftransform(gl_Vertex); //deprecated in new GLSL
   //gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
   //gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex; }
