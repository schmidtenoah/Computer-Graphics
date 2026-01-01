 /**
  * Functions used by more than one shader.
  */

 void transform(inout vec3 pos, in vec3 forward, inout vec3 up, in vec3 localScale, in vec3 instanceOffset) {
    vec3 f = normalize(forward);
    vec3 u = normalize(up);
    vec3 r = normalize(cross(f, u));
    u = cross(r, f);
    mat3 rot = mat3(r, u, f);

    pos = rot * (pos * localScale) + instanceOffset;
    up = u;
}
