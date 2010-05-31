#version 110

uniform vec3		SAxis;
uniform vec3		TAxis;
uniform float		SOffs;
uniform float		TOffs;
uniform float		TexIW;
uniform float		TexIH;
uniform float		TexMinS;
uniform float		TexMinT;
uniform float		CacheS;
uniform float		CacheT;

varying vec2		TextureCoordinate;
varying vec2		LightmapCoordinate;

void main()
{
	const float BLOCK_WIDTH = 128.0;
	const float BLOCK_HEIGHT = 128.0;

	//	Transforming The Vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	//	Calculate texture coordinates.
	float s = (dot(gl_Vertex.xyz, SAxis) + SOffs);
	float t = (dot(gl_Vertex.xyz, TAxis) + TOffs);
	float lights = (s - TexMinS + CacheS * 16.0 + 8.0) / (BLOCK_WIDTH * 16.0);
	float lightt = (t - TexMinT + CacheT * 16.0 + 8.0) / (BLOCK_HEIGHT * 16.0);
	TextureCoordinate = vec2(s * TexIW, t * TexIH);
	LightmapCoordinate = vec2(lights, lightt);
}
