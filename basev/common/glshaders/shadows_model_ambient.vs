uniform float		Inter;

attribute vec4		Vert2;
attribute vec2		TexCoord;

varying vec2		TextureCoordinate;

void main()
{
	//	Transforming The Vertex
	gl_Position = gl_ModelViewProjectionMatrix * mix(gl_Vertex, Vert2, Inter);

	//	Pass texture coordinates.
	TextureCoordinate = TexCoord;
}
