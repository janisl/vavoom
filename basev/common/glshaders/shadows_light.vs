#version 110

uniform vec3		LightPos;
uniform float		LightRadius;

uniform vec3		SAxis;
uniform vec3		TAxis;
uniform float		SOffs;
uniform float		TOffs;
uniform float		TexIW;
uniform float		TexIH;

attribute vec3		SurfNormal;
attribute float		SurfDist;
attribute vec3		ViewOrigin;

varying vec3		Normal;
varying float		Dist;
varying vec3		VertToLight;
varying vec3        VertToView;

varying vec2		TextureCoordinate;
//varying vec3        VOrg;

void main()
{
	//	Transforming The Vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	//	Calculate texture coordinates.
	float s = (dot(gl_Vertex.xyz, SAxis) + SOffs) * TexIW;
	float t = (dot(gl_Vertex.xyz, TAxis) + TOffs) * TexIH;
	TextureCoordinate = vec2(s, t);

	Normal = SurfNormal;
	Dist = dot(LightPos, SurfNormal) - SurfDist;
	VertToLight.xyz = LightPos.xyz - gl_Vertex.xyz;
    VertToView.xyz = ViewOrigin.xyz - gl_Vertex.xyz;
//	VOrg = ViewOrigin;
}
