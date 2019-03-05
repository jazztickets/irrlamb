varying vec3 normal;
varying vec3 vertex;
uniform sampler2D texture;

void main(void) {

	float distance = length(gl_LightSource[0].position.xyz - vertex);
	float attenuation = 1.0 / (gl_LightSource[0].constantAttenuation + distance * gl_LightSource[0].linearAttenuation + distance * distance * gl_LightSource[0].quadraticAttenuation);
	attenuation = clamp(attenuation, 0.0, 1.0);
	vec3 light_vector = normalize(gl_LightSource[0].position.xyz - vertex);
	vec4 diffuse = gl_LightSource[0].diffuse * max(dot(normal, light_vector), 0.0);

	vec4 light_color = attenuation * diffuse;
	light_color.w = 0.0;

	vec4 ambient = vec4(gl_LightModel.ambient.x, gl_LightModel.ambient.y, gl_LightModel.ambient.z, 1);
	vec4 texture_color = texture2D(texture, vec2(gl_TexCoord[0]));
	vec4 frag_color = texture_color * (ambient + light_color);

	if(gl_Fog.density > 0.0) {
		const float LOG2 = 1.442695;
		float frag_z = gl_FragCoord.z / gl_FragCoord.w;
		float fog_factor = clamp(exp2(-gl_Fog.density * gl_Fog.density * frag_z * frag_z * LOG2), 0.0, 1.0);

		gl_FragColor = mix(gl_Fog.color, frag_color, fog_factor);
	}
	else {
		gl_FragColor = frag_color;
	}
}
