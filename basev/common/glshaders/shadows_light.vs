#version 110

uniform vec3 ViewOrigin;
uniform vec3 LightPos;
uniform vec3 SAxis;
uniform vec3 TAxis;
uniform float SOffs;
uniform float TOffs;
uniform float TexIW;
uniform float TexIH;

attribute vec3 SurfNormal;
attribute float SurfDist;

varying vec3 Normal;
varying vec3 VertToLight;
varying vec3 VertToView;
varying float Dist;
varying float VDist;
varying vec2 TextureCoordinate;

void main ()
{
	//	Transforming The Vertex
	gl_Position = (gl_ModelViewProjectionMatrix * gl_Vertex);

	//	Calculate texture coordinates.
	vec2 ts;

	ts.x = ((dot (gl_Vertex.xyz, SAxis) + SOffs) * TexIW);
	ts.y = ((dot (gl_Vertex.xyz, TAxis) + TOffs) * TexIH);
	
	TextureCoordinate = ts;

	Normal = SurfNormal;
	float LightDist;
	float ViewDist;

	LightDist = dot (LightPos, SurfNormal);
	ViewDist = dot(ViewOrigin, SurfNormal);
	Dist = (LightDist - SurfDist);
	VDist = (ViewDist - SurfDist);

	VertToLight = (LightPos - gl_Vertex.xyz);
	VertToView = (ViewOrigin - gl_Vertex.xyz);
}
