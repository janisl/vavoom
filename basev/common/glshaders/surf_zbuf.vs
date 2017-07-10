#version 110

void main ()
{
	//	Transforming The Vertex
	gl_Position = (gl_ModelViewProjectionMatrix * gl_Vertex);
}
