#version 120         

varying vec3          	var_Vertex;        
varying vec2            	var_TexDiffuse;        
varying vec2            	var_TexNormal;      
varying vec4            	var_TexLightProj;      
varying vec2            	var_TexLightFalloff; 
varying mat3     			var_TangentToWorldMatrix;      
varying vec4            	var_Color;        
     
uniform sampler2D   	u_normalTexture;         
uniform sampler2D   	u_lightFalloffTexture;         
uniform sampler2D   	u_lightProjectionTexture;         
uniform sampler2D   	u_diffuseTexture;   
         
uniform vec4 				u_lightOrigin;   
uniform vec4 				u_diffuseColor;    
      
void main( void ) {         
	// compute light direction
	vec3 L = normalize( u_lightOrigin.xyz - var_Vertex ); 

	// compute normal in tangent space from normalmap        
	vec3 N = normalize( 2.0 * ( texture2D( u_normalTexture, var_TexNormal.st ).xyz - 0.5 ) );      
 
	// transform normal and light direction into world space 
	N = var_TangentToWorldMatrix * N; 
	L = normalize( var_TangentToWorldMatrix * L );
	
	// compute N dot L
	float NdotL = clamp( dot( N, L ), 0.0, 1.0 ); 
 
	// compute the diffuse term     
	vec4 diffuse = texture2D( u_diffuseTexture, var_TexDiffuse );     
	diffuse *= u_diffuseColor; 
	diffuse *= NdotL;
    
	// compute light projection and falloff 
	vec3 lightFalloff = texture2D( u_lightFalloffTexture, var_TexLightFalloff ).rgb;     
	vec3 lightProjection = texture2DProj( u_lightProjectionTexture, var_TexLightProj.xyw ).rgb; 
	vec3 lightAttenuation = lightProjection * lightFalloff; 

	// compute lighting model     
    vec4 color = diffuse;
	color.rgb *= lightAttenuation; 
	color.rgb *= var_Color.rgb;
 
	// output final color     
	gl_FragColor = color;        
}