#version 120    
    
varying vec3          	var_Vertex;   
varying vec2            	var_TexDiffuse;   
varying vec2            	var_TexNormal;   
varying vec2            	var_TexSpecular;  
varying vec4            	var_TexLightProj; 
varying vec2            	var_TexLightFalloff; 
varying mat3     			var_TangentToWorldMatrix; 
varying vec4            	var_Color;   

uniform sampler2D   	u_normalTexture;    
uniform sampler2D   	u_lightFalloffTexture;    
uniform sampler2D   	u_lightProjectionTexture;    
uniform sampler2D   	u_diffuseTexture;    
uniform sampler2D   	u_specularTexture;   
    
uniform vec4 				u_lightOrigin;   
uniform vec4 				u_viewOrigin;   
uniform vec4 				u_diffuseColor;   
uniform vec4 				u_specularColor;   
 
// traditional lambertian blinn-phong lighting model 
vec4 lightingModel( vec4 diffuse, vec4 specular, vec3 L, vec3 V, vec3 N, vec3 H ) { 
	float NdotL = clamp( dot( N, L ), 0.0, 1.0 );
	float NdotH = pow( clamp( dot( N, H ), 0.0, 1.0 ), 16 );

	return ( diffuse * NdotL ) + ( specular * NdotH );
} 

void main( void ) {    
	// compute view direction, light direction, and half angle     
	vec3 V = normalize( u_viewOrigin.xyz - var_Vertex ); 
	vec3 L = normalize( u_lightOrigin.xyz - var_Vertex );  
   	vec3 H = normalize( L + V );
	
	// compute normal in tangent space from normalmap
	// NOTE: red is swaped with alpha here...
	vec3 N = normalize( 2.0 * ( texture2D( u_normalTexture, var_TexNormal.st ).wyz - 0.5 ) ); 

	// transform normal, light direction, and half angle into world space 
	N = var_TangentToWorldMatrix * N;
	L = normalize( var_TangentToWorldMatrix * L ); 
	H = normalize( var_TangentToWorldMatrix * H ); 
 
	// compute the diffuse term
	vec4 diffuse = texture2D( u_diffuseTexture, var_TexDiffuse );
	diffuse *= u_diffuseColor;

	// compute the specular term   
	vec4 specular = texture2D( u_specularTexture, var_TexSpecular );
	specular *= u_specularColor;

	// compute light projection and falloff 
	vec4 lightProjection = texture2DProj( u_lightProjectionTexture, var_TexLightProj.xyw );  
	vec4 lightFalloff = texture2D( u_lightFalloffTexture, var_TexLightFalloff );
	
	// compute lighting model
    vec4 color = lightingModel( diffuse, specular, L, V, N, H );
	color *= lightProjection;  
	color *= lightFalloff;  
	color *= var_Color;  
 
	// output final color
	gl_FragColor = color;   
}