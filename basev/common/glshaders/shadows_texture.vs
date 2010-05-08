attribute vec2		TexCoord;

varying vec2		TextureCoordinate;

void main()
{
	//	Transforming The Vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	//	Pass texture coordinates.
	TextureCoordinate = TexCoord;
}
