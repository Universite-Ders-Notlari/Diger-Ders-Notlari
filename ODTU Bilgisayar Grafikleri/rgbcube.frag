varying vec3 vc; //value is interpolated by rasterizer 
			    //across each primitive (usually triangle)
void main() {
	gl_FragColor = vec4(vc, 1.0);
} //end of Fragment Shader
