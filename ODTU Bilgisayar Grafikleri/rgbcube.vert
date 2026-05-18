varying vec3 vc; //written in VS, read in FS w/ interpolation
void main() {
	gl_Position = ftransform();
	vc = gl_Vertex.xyz; //same as vec3(gl_Vertex)
} //end of Vertex Shader
