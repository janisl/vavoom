#version 110

uniform vec3 ViewOrigin;
uniform float Inter;

attribute vec4 Vert2;
attribute vec4 LightVal;
attribute vec2 TexCoord;

varying vec4 Light;
varying vec3 VertToView;
varying vec3 VPos;
varying vec2 TextureCoordinate;

void main ()
{
	vec4 Vert;

	Vert = mix (gl_Vertex, Vert2, Inter);
	gl_Position = (gl_ModelViewProjectionMatrix * Vert);
	VertToView = (ViewOrigin - Vert.xyz);
	VPos = (ViewOrigin - gl_Position.xyz);
	Light = LightVal;
	TextureCoordinate = TexCoord;
}