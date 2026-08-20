// Array and textureless 4D simplex noise. Based on the implementation by
// Ian McEwan, Ashima Arts, maintained by Stefan Gustavson.
//
// Copyright (C) 2011 by Ashima Arts (Simplex noise)
// Copyright (C) 2011-2016 by Stefan Gustavson (Classic noise and others)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

// Use unique function names for each type. Do not use overloaded functions, as not all GLSL implementations support them.
highp vec4 simplex_4d_mod289_vec4(highp vec4 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

highp float simplex_4d_mod289_float(highp float x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

highp vec4 simplex_4d_permute_vec4(highp vec4 x) {
	return simplex_4d_mod289_vec4(((x * 34.0) + 10.0) * x);
}

highp float simplex_4d_permute_float(highp float x) {
	return simplex_4d_mod289_float(((x * 34.0) + 10.0) * x);
}

highp vec4 simplex_4d_taylor_inverse_sqrt_vec4(highp vec4 r) {
	return 1.79284291400159 - 0.85373472095314 * r;
}

highp float simplex_4d_taylor_inverse_sqrt_float(highp float r) {
	return 1.79284291400159 - 0.85373472095314 * r;
}

highp vec4 simplex_4d_gradient(highp float j, highp vec4 ip) {
	const highp vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
	highp vec4 p;
	highp vec4 s;

	p.xyz = floor(fract(vec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
	p.w = 1.5 - dot(abs(p.xyz), ones.xyz);
	s = vec4(lessThan(p, vec4(0.0)));
	p.xyz = p.xyz + (s.xyz * 2.0 - 1.0) * s.www;
	return p;
}

highp float simplex_4d_noise(highp vec4 v) {
	const highp vec2 c = vec2(0.138196601125011, 0.309016994374947);

	highp vec4 i = floor(v + dot(v, vec4(c.y)));
	highp vec4 x0 = v - i + dot(i, vec4(c.x));

	highp vec4 i0;
	highp vec3 is_x = step(x0.yzw, x0.xxx);
	highp vec3 is_yz = step(x0.zww, x0.yyz);
	i0.x = is_x.x + is_x.y + is_x.z;
	i0.yzw = 1.0 - is_x;
	i0.y += is_yz.x + is_yz.y;
	i0.zw += 1.0 - is_yz.xy;
	i0.z += is_yz.z;
	i0.w += 1.0 - is_yz.z;

	highp vec4 i3 = clamp(i0, 0.0, 1.0);
	highp vec4 i2 = clamp(i0 - 1.0, 0.0, 1.0);
	highp vec4 i1 = clamp(i0 - 2.0, 0.0, 1.0);

	highp vec4 x1 = x0 - i1 + c.x;
	highp vec4 x2 = x0 - i2 + c.x * 2.0;
	highp vec4 x3 = x0 - i3 + c.x * 3.0;
	highp vec4 x4 = x0 - 1.0 + c.x * 4.0;

	i = simplex_4d_mod289_vec4(i);
	highp float j0 = simplex_4d_permute_float(simplex_4d_permute_float(simplex_4d_permute_float(simplex_4d_permute_float(i.w) + i.z) + i.y) + i.x);
	highp vec4 j1 = simplex_4d_permute_vec4(simplex_4d_permute_vec4(simplex_4d_permute_vec4(simplex_4d_permute_vec4(i.w + vec4(i1.w, i2.w, i3.w, 1.0)) + i.z + vec4(i1.z, i2.z, i3.z, 1.0)) + i.y + vec4(i1.y, i2.y, i3.y, 1.0)) + i.x + vec4(i1.x, i2.x, i3.x, 1.0));

	const highp vec4 ip = vec4(1.0 / 294.0, 1.0 / 49.0, 1.0 / 7.0, 0.0);
	highp vec4 p0 = simplex_4d_gradient(j0, ip);
	highp vec4 p1 = simplex_4d_gradient(j1.x, ip);
	highp vec4 p2 = simplex_4d_gradient(j1.y, ip);
	highp vec4 p3 = simplex_4d_gradient(j1.z, ip);
	highp vec4 p4 = simplex_4d_gradient(j1.w, ip);

	highp vec4 norm = simplex_4d_taylor_inverse_sqrt_vec4(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;
	p4 *= simplex_4d_taylor_inverse_sqrt_float(dot(p4, p4));

	highp vec3 m0 = max(0.6 - vec3(dot(x0, x0), dot(x1, x1), dot(x2, x2)), 0.0);
	highp vec2 m1 = max(0.6 - vec2(dot(x3, x3), dot(x4, x4)), 0.0);
	m0 = m0 * m0;
	m1 = m1 * m1;
	return 49.0 * (dot(m0 * m0, vec3(dot(p0, x0), dot(p1, x1), dot(p2, x2))) + dot(m1 * m1, vec2(dot(p3, x3), dot(p4, x4))));
}
