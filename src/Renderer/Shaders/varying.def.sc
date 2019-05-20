vec3 a_position  : POSITION;
vec3 a_normal    : NORMAL;
vec3 a_tangent   : TANGENT;
vec3 a_bitangent : BITANGENT;
vec2 a_texcoord0 : TEXCOORD0;

vec3 v_eyepos    : POSITION1 = vec3(0.0, 0.0, 0.0);
vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 0.0);
vec3 v_tangent   : TANGENT   = vec3(0.0, 0.0, 0.0);
vec3 v_bitangent : BITANGENT = vec3(0.0, 0.0, 0.0);
vec2 v_texcoord0 : TEXCOORD0 = vec2(1.0, 0.0);
