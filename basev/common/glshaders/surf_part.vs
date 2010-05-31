#version 110

attribute vec4		LightVal;
attribute vec2		TexCoord;

varying vec4		Light;
varying vec2		TextureCoordinate;

void main()
{
	//	Transforming The Vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	//	Pass light
	Light = LightVal;

	//	Pass texture coordinates.
	TextureCoordinate = TexCoord;
}
