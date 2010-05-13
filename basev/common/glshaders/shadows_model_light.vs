uniform float		Inter;
uniform vec3		LightPos;
uniform float		LightRadius;
uniform mat4		ModelToWorldMat;
uniform mat3		NormalToWorldMat;

attribute vec4		Vert2;
attribute vec3		VertNormal;
attribute vec3		Vert2Normal;
attribute vec2		TexCoord;

varying vec3		Normal;
//varying float		Dist;
varying vec3		VertToLight;
varying vec2		TextureCoordinate;

void main()
{
	vec4 Vert = mix(gl_Vertex, Vert2, Inter);
	//Vert = ModelToWorldMat * Vert;
	Vert = Vert * ModelToWorldMat;
	gl_Position = gl_ModelViewProjectionMatrix * Vert;

	Normal = NormalToWorldMat * mix(VertNormal, Vert2Normal, Inter); 
	VertToLight.xyz = LightPos.xyz - Vert.xyz;
	TextureCoordinate = TexCoord;
}
