varying vec4 position;
varying vec3 normal;
varying vec3 light_direction;

void main()
{
    vec4 ambient = gl_LightModel.ambient * gl_FrontMaterial.ambient;
    vec4 diffuse;
    vec4 specular;
    vec3 reflect;
    
    for (int i = 0; i < gl_MaxLights; i++){
        diffuse += gl_LightSource[i].diffuse * gl_FrontMaterial.diffuse * max(dot(normalize(light_direction),normal),0);
        reflect = -light_direction + 2*(dot(light_direction,normal)) * normal;
        specular += gl_LightSource[i].specular * gl_FrontMaterial.specular * pow(max(dot(normalize(reflect),position), 0), gl_FrontMaterial.shininess);
    }
    
    gl_FragColor = ambient + diffuse + specular;
}