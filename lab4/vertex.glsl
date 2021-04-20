varying vec4 position;
varying vec3 normal;
varying vec3 light_direction;

void main()
{
    normal = normalize(gl_NormalMatrix * gl_Normal);
    position = gl_ModelViewProjectionMatrix * gl_Vertex;
    light_direction = normalize(gl_LightSource[0].position.xyz - position.xyz);
    gl_Position = position;
}
