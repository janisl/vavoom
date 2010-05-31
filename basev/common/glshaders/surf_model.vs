#version 110

uniform float		Inter;

attribute vec4		Vert2;
attribute vec4		LightVal;
attribute vec2		TexCoord;

varying vec4		Light;
varying vec2		TextureCoordinate;

void main()
{
	//	Transforming The Vertex
	gl_Position = gl_ModelViewProjectionMatrix * mix(gl_Vertex, Vert2, Inter);

	//	Pass light
	Light = LightVal;

	//	Pass texture coordinates.
	TextureCoordinate = TexCoord;
}
