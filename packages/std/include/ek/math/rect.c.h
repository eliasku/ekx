rect_t rect(float x, float y, float w, float h) {
    return (rect_t) {{x, y, w, h}};
}

rect_t irect_to_rect(irect_t irc) {
    return (rect_t) {{
                             (float) irc.x,
                             (float) irc.y,
                             (float) irc.w,
                             (float) irc.h
                     }};
}

rect_t rect_size(const vec2_t size) {
    return (rect_t) {{0, 0, size.x, size.y}};
}

rect_t rect_wh(const float w, const float h) {
    return (rect_t) {{0, 0, w, h}};
}

rect_t rect_half_extents(const float w, const float h) {
    return (rect_t) {{-0.5 * w, -0.5 * h, w, h}};
}

rect_t rect_minmax(const vec2_t min, const vec2_t max) {
    return (rect_t) {{min.x, min.y, max.x - min.x, max.y - min.y}};
}

rect_t rect_01(void) {
    return (rect_t) {{0, 0, 1, 1}};
}

vec2_t rect_rb(const rect_t rc) {
    return (vec2_t) {{rc.x + rc.w, rc.y + rc.h}};
}

vec2_t rect_center(rect_t rc) {
    return (vec2_t) {{rc.x + rc.w * 0.5f, rc.y + rc.h * 0.5f}};
}

rect_t rect_clamp_bounds(const rect_t a, const rect_t b) {
    return rect_minmax(max_vec2(a.position, b.position),
                       min_vec2(rect_rb(a), rect_rb(b)));
}

irect_t irect_clamp_bounds(irect_t a, irect_t b) {
    const int x0 = MAX(a.x, b.x);
    const int y0 = MAX(a.y, b.y);
    const int x1 = MIN(RECT_R(a), RECT_R(b));
    const int y1 = MIN(RECT_B(a), RECT_B(b));
    return (irect_t) {{x0, y0, x1 - x0, y1 - y0}};
}

rect_t rect_combine(const rect_t a, const rect_t b) {
    return rect_minmax(min_vec2(a.position, b.position),
                       max_vec2(rect_rb(a), rect_rb(b)));
}

irect_t irect_combine(irect_t a, irect_t b) {
    irect_t result;
    result.x = MIN(a.x, b.x);
    result.y = MIN(a.y, b.y);
    result.w = MAX(RECT_R(a), RECT_R(b)) - result.x;
    result.h = MAX(RECT_B(a), RECT_B(b)) - result.y;
    return result;
}

bool rect_overlaps(const rect_t a, const rect_t b) {
    return a.x <= RECT_R(b) && b.x <= RECT_R(a) && a.y <= RECT_B(b) && b.y <= RECT_B(a);
}

bool irect_overlaps(irect_t a, irect_t b) {
    return a.x <= RECT_R(b) && b.x <= RECT_R(a) && a.y <= RECT_B(b) && b.y <= RECT_B(a);
}

rect_t rect_scale(const rect_t a, const vec2_t s) {
    rect_t r = (rect_t) {{a.x * s.x, a.y * s.y, a.w * s.x, a.h * s.y}};
    if (r.w < 0.0f) {
        r.x += r.w;
        r.w = -r.w;
    }
    if (r.h < 0.0f) {
        r.y += r.h;
        r.h = -r.h;
    }
    return r;
}

rect_t rect_scale_f(const rect_t a, const float s) {
    return rect_scale(a, vec2(s, s));
}

rect_t rect_translate(const rect_t a, const vec2_t t) {
    return (rect_t) {{a.x + t.x, a.y + t.y, a.w, a.h}};
}

rect_t rect_transform(const rect_t rc, const mat3x2_t matrix) {
    return aabb2_get_rect(
            aabb2_add_transformed_rect(aabb2_empty(), rc, matrix)
    );
}

bool rect_contains(rect_t rc, vec2_t point) {
    return rc.x <= point.x &&
           point.x <= (rc.x + rc.w) &&
           rc.y <= point.y &&
           point.y <= (rc.y + rc.h);
}

bool rect_is_empty(rect_t a) {
    return a.w <= 0 || a.h <= 0;
}

rect_t rect_expand(rect_t a, float w) {
    return (rect_t) {{a.x - w, a.y - w, a.w + w + w, a.h + w + w}};
}

aabb2_t aabb2_from_rect(rect_t rc) {
    aabb2_t br;
    br.x0 = rc.x;
    br.y0 = rc.y;
    br.x1 = rc.x + rc.w;
    br.y1 = rc.y + rc.h;
    return br;
}

aabb2_t aabb2_empty(void) {
    aabb2_t br;
    br.x0 = 1.0e24f;
    br.y0 = 1.0e24f;
    br.x1 = -1.0e24f;
    br.y1 = -1.0e24f;
    return br;
}

static aabb2_t brect_push(aabb2_t br, float e0, float e1, float e2, float e3) {
    if (e0 < br.data[0]) br.data[0] = e0;
    if (e1 < br.data[1]) br.data[1] = e1;
    if (e2 > br.data[2]) br.data[2] = e2;
    if (e3 > br.data[3]) br.data[3] = e3;
    return br;
}

aabb2_t aabb2_add_circle(aabb2_t bb, vec3_t circle) {
    return brect_push(bb,
                      circle.x - circle.z,
                      circle.y - circle.z,
                      circle.x + circle.z,
                      circle.y + circle.z);
}

aabb2_t aabb2_add_rect(aabb2_t bb, rect_t rc) {
    return brect_push(bb, rc.x, rc.y, rc.x + rc.w, rc.y + rc.h);
}

aabb2_t aabb2_add_point(aabb2_t bb, vec2_t point) {
    return brect_push(bb, point.x, point.y, point.x, point.y);
}

aabb2_t aabb2_add_transformed_rect(aabb2_t bb, rect_t rc, mat3x2_t matrix) {
    bb = aabb2_add_point(bb, vec2_transform(vec2(rc.x, rc.y), matrix));
    bb = aabb2_add_point(bb, vec2_transform(vec2(rc.x + rc.w, rc.y), matrix));
    bb = aabb2_add_point(bb, vec2_transform(vec2(rc.x, rc.y + rc.h), matrix));
    bb = aabb2_add_point(bb, vec2_transform(vec2(rc.x + rc.w, rc.y + rc.h), matrix));
    return bb;
}

rect_t aabb2_get_rect(aabb2_t bb) {
    rect_t r;
    r.x = bb.x0;
    r.y = bb.y0;
    r.w = bb.x1 > bb.x0 ? (bb.x1 - bb.x0) : 0.0f;
    r.h = bb.y1 > bb.y0 ? (bb.y1 - bb.y0) : 0.0f;
    return r;
}

// TODO: simple test
bool aabb2_is_empty(aabb2_t bb) {
    return bb.x0 >= bb.x1 || bb.y0 >= bb.y1;
}
