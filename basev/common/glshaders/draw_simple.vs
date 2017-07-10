#version 110

varying vec2 TextureCoordinate;

void main ()
{
	//	Transforming The Vertex
	gl_Position = (gl_ModelViewProjectionMatrix * gl_Vertex);

	//	Pass texture coordinates.
	TextureCoordinate = gl_MultiTexCoord0.xy;
}
