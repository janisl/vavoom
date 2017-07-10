#version 110

uniform vec3 SAxis;
uniform vec3 TAxis;
uniform float SOffs;
uniform float TOffs;
uniform float TexIW;
uniform float TexIH;
uniform float TexMinS;
uniform float TexMinT;
uniform float CacheS;
uniform float CacheT;

varying vec2 TextureCoordinate;
varying vec2 LightmapCoordinate;

void main ()
{
	gl_Position = (gl_ModelViewProjectionMatrix * gl_Vertex);
	float s;

	s = (dot (gl_Vertex.xyz, SAxis) + SOffs);
	float t;

	t = (dot (gl_Vertex.xyz, TAxis) + TOffs);
	vec2 st;

	st.x = (s * TexIW);
	st.y = (t * TexIH);

	TextureCoordinate = st;
	vec2 lightst;

	lightst.x = (((
		(s - TexMinS)
		+ 
		(CacheS * 16.0)
		) + 8.0) / 2048.0);
	lightst.y = (((
		(t - TexMinT)
		+ 
		(CacheT * 16.0)
		) + 8.0) / 2048.0);

	LightmapCoordinate = lightst;
}
