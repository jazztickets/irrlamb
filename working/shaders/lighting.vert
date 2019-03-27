varying vec3 normal;
varying vec3 vertex;

void main(void) {

	// Get normal
	normal = normalize(gl_NormalMatrix * gl_Normal);

	// Get vertex
	vertex = vec3(gl_ModelViewMatrix * gl_Vertex);

	// Set vertex position
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	// Get texture coordinates
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}
