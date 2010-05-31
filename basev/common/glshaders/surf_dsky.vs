#version 110

attribute vec2		TexCoord;
attribute vec2		TexCoord2;

varying vec2		TextureCoordinate;
varying vec2		Texture2Coordinate;

void main()
{
	//	Transforming The Vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	//	Pass texture coordinates.
	TextureCoordinate = TexCoord;
	Texture2Coordinate = TexCoord2;
}
