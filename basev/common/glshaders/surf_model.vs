#version 110

uniform float		Inter;

attribute vec4		Vert2;
attribute vec4		LightVal;
attribute vec2		TexCoord;
attribute vec3		ViewOrigin;

varying vec4		Light;
varying vec2		TextureCoordinate;
varying vec3		VertToView;

void main()
{
	//	Transforming The Vertex
	vec4 Vert = mix(gl_Vertex, Vert2, Inter);
	gl_Position = gl_ModelViewProjectionMatrix * Vert;

	VertToView.xyz = ViewOrigin.xyz - Vert.xyz;

	//	Pass light
	Light = LightVal;

	//	Pass texture coordinates.
	TextureCoordinate = TexCoord;
}
