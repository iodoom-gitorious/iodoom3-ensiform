#version 120     

varying vec3          	var_Vertex; 
varying vec2            	var_TexDiffuse; 
varying vec2            	var_TexNormal;
varying vec4            	var_TexLightProj;
varying vec2            	var_TexLightFalloff;
varying mat3     			var_TangentToWorldMatrix; 
varying vec4            	var_Color; 

attribute vec4          	attr_TexCoord;     
attribute vec3          	attr_Tangent;     
attribute vec3          	attr_Binormal;     
attribute vec3          	attr_Normal;  
 
uniform mat4				u_modelMatrix;
uniform vec4 				u_lightProjectionS; 
uniform vec4 				u_lightProjectionT; 
uniform vec4 				u_lightProjectionQ; 
uniform vec4 				u_lightFalloff; 
uniform vec4 				u_colorModulate; 
uniform vec4				u_colorAdd; 
     
void main( void ) {     
    // mvp transform into clip space     
	gl_Position = ftransform( ); 

	// transform position into world space 
	var_Vertex = ( u_modelMatrix * gl_Vertex ).xyz;

	// diffuse map texgen     
	var_TexDiffuse.xy = ( gl_TextureMatrix[0] * attr_TexCoord ).st; 

	// normal map texgen 
	var_TexNormal.xy = ( gl_TextureMatrix[2] * attr_TexCoord ).st; 

	// light projection texgen
	var_TexLightProj.x = dot( u_lightProjectionS, gl_Vertex );
	var_TexLightProj.y = dot( u_lightProjectionT, gl_Vertex );
	var_TexLightProj.z = dot( u_lightFalloff, gl_Vertex );
	var_TexLightProj.w = dot( u_lightProjectionQ, gl_Vertex );

	// light falloff texgen
	var_TexLightFalloff.x = dot( u_lightFalloff, gl_Vertex );
	var_TexLightFalloff.y = 0.5;
	
	// construct tangent-space-to-world-space 3x3 matrix 
	var_TangentToWorldMatrix = mat3(attr_Tangent, attr_Binormal, attr_Normal); 

	// primary color 
	var_Color = gl_Color * u_colorModulate + u_colorAdd; 
}