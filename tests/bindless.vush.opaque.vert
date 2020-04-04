// file generated by vush compiler, from ../../tests/bindless.vush
#version 460
#pragma stage(vertex)
#extension GL_GOOGLE_cpp_style_line_directive : require
#extension GL_GOOGLE_include_directive : require

struct VS_IN {
	vec3 position;
	vec4 color;
	vec2 texcoord;
};

struct VS_OUT {
	vec4 position;
	vec4 color;
	vec2 texcoord;
};

#include "opaque.vuas"

/*
scopes:
	(None) - from previous stage
	Attribute - as an attribute (vattr or PVP)
	User - bound by hand
	Aspect - bound by the pass implementing this aspect
	BaseInstance - coming from a bindless storage indexed by BaseInstanceIndex
	Instance - coming from a bindless storage indexed by InstanceIndex
	Draw - coming from a bindless storage indexed by DrawID
	Push - coming from a bindless storage indexed by a PushConstant
*/

layout(location = 0) out VS_OUT _out;

layout(location = 0+0) in vec3 _VS_IN_position;
layout(location = 0+1) in vec4 _VS_IN_color;
layout(location = 0+2) in vec2 _VS_IN_texcoord;
layout(std140, binding = 0) uniform _aspect_ {
	VP vp;
} _aspect;
layout(std140, binding = 1) uniform _user_ {
	vec4 tint;
} _user;
#define _DRAW_SIZE 32
struct _draw_t {
	vec4 tint2;
};
layout(std140, binding = 2) uniform _draw_ {
	_draw_t _[_DRAW_SIZE];
} _draw;
#define _BASE_INSTANCE_SIZE 32
struct _baseinstance_t {
	mat4 model_matrix;
};
layout(std140, binding = 3) uniform _base_instance_ {
	_baseinstance_t _[_BASE_INSTANCE_SIZE];
} _base_instance;

#line 27 "../../tests/bindless.vush"
VS_OUT opaque_vertex(VS_IN vin, VP vp, vec4 tint, mat4 model_matrix, vec4 tint2) {
	VS_OUT vout;
	vout.position = vp.projection * vp.view * model_matrix * vec4(vin.position, 1.0);
	vout.texcoord = vin.texcoord;
	vout.color = vin.color * tint * tint2;
	return vout;
}

void main() {
	VS_IN vin;
	vin.position = _VS_IN_position;
	vin.color = _VS_IN_color;
	vin.texcoord = _VS_IN_texcoord;
	VP vp = _aspect.vp;
	vec4 tint = _user.tint;
	vec4 tint2 = _draw._[gl_DrawID].tint2;
	mat4 model_matrix = _base_instance._[gl_BaseInstance].model_matrix;
	_out = opaque_vertex(vin, vp, tint, model_matrix, tint2);
}