#version 110

uniform mat4 ModelToWorldMat;
uniform mat3 NormalToWorldMat;
uniform vec3 LightPos;
uniform vec3 ViewOrigin;
uniform float Inter;

attribute vec4 Vert2;
attribute vec3 VertNormal;
attribute vec3 Vert2Normal;
attribute vec2 TexCoord;

varying vec3 Normal;
varying vec3 VertToLight;
varying vec3 VertToView;
varying vec2 TextureCoordinate;
varying float PlaneDist;
varying float Dist;
varying float VDist;

void main ()
{
	//	Transforming The Vertex
	vec4 Vert_1;

	Vert_1 = (mix (gl_Vertex, Vert2, Inter) * ModelToWorldMat);
	gl_Position = (gl_ModelViewProjectionMatrix * Vert_1);

	Normal = (NormalToWorldMat * mix (VertNormal, Vert2Normal, Inter));
	float SurfDist;

	SurfDist = dot (Normal, Vert_1.xyz);
	PlaneDist = SurfDist;
	Dist = (dot (LightPos, Normal) - SurfDist);
	VDist = (dot (ViewOrigin, Normal) - SurfDist);

	VertToLight = (LightPos - Vert_1.xyz);
	VertToView = (ViewOrigin - Vert_1.xyz);

	TextureCoordinate = TexCoord;
}
