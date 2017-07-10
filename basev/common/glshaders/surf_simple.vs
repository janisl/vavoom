#version 110

uniform vec3 SAxis;
uniform vec3 TAxis;
uniform float SOffs;
uniform float TOffs;
uniform float TexIW;
uniform float TexIH;

varying vec2 TextureCoordinate;

void main ()
{
	//	Transforming The Vertex
	gl_Position = (gl_ModelViewProjectionMatrix * gl_Vertex);

	//	Calculate texture coordinates.
	vec2 st;

	st.x = ((dot (gl_Vertex.xyz, SAxis) + SOffs) * TexIW);
	st.y = ((dot (gl_Vertex.xyz, TAxis) + TOffs) * TexIH);

	TextureCoordinate = st;
}
