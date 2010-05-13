uniform float		Inter;
uniform mat4		ModelToWorldMat;

attribute vec4		Vert2;
attribute vec2		TexCoord;

varying vec2		TextureCoordinate;

void main()
{
	//	Transforming The Vertex
	vec4 Vert = mix(gl_Vertex, Vert2, Inter);
	//Vert = ModelToWorldMat * Vert;
	Vert = Vert * ModelToWorldMat;
	gl_Position = gl_ModelViewProjectionMatrix * Vert;

	//	Pass texture coordinates.
	TextureCoordinate = TexCoord;
}
