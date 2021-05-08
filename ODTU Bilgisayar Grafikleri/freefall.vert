//free fall under gravity
uniform vec3 vel;
uniform float g, t;
void main()
{
	vec3 object_pos;
	object_pos.x = gl_Vertex.x + vel.x*t;
	object_pos.y = gl_Vertex.y + vel.y*t + g/(2.0)*t*t;
	object_pos.z = gl_Vertex.z + vel.z*t;
	gl_Position = 
	    gl_ModelViewProjectionMatrix*vec4(object_pos,1);
}

