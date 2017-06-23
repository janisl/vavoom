#version 110

uniform float		Inter;
uniform mat4		ModelToWorldMat;

attribute vec4		Vert2;
attribute vec2		TexCoord;
attribute vec3		ViewOrigin;

varying vec2		TextureCoordinate;
varying vec3		VertToView;

void main()
{
	//	Transforming The Vertex
	vec4 Vert = mix(gl_Vertex, Vert2, Inter);
	//Vert = ModelToWorldMat * Vert;
	Vert = Vert * ModelToWorldMat;
	gl_Position = gl_ModelViewProjectionMatrix * Vert;

	VertToView.xyz = ViewOrigin.xyz - Vert.xyz;

	//	Pass texture coordinates.
	TextureCoordinate = TexCoord;
}
