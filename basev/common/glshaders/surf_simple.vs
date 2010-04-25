uniform vec3		SAxis;
uniform vec3		TAxis;
uniform float		SOffs;
uniform float		TOffs;
uniform float		TexIW;
uniform float		TexIH;

varying vec2		TextureCoordinate;

void main()
{
	//	Transforming The Vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	//	Calculate texture coordinates.
	float s = (dot(gl_Vertex, SAxis) + SOffs) * TexIW;
	float t = (dot(gl_Vertex, TAxis) + TOffs) * TexIH;
	TextureCoordinate = vec2(s, t);
}
