#version 330

out vec4 vCol;


uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

layout (location = 0) in vec3 pos;
uniform vec3 blockColor;

void main()
{
	gl_Position = projection * view * model * vec4(pos, 1.0);
	if(blockColor.x == 0.0 && blockColor.y == 0.0 && blockColor.z == 0.0) {
		vCol = vec4(clamp(pos, 0.0f, 1.0f), 1.0f);
	} else {
		vCol = vec4(blockColor,1.0f);
	}
	
	


}