#pragma once
#include <memory>
#include <tuple>
#include <utility>
#include <queue>
#include <utility>
#include <unordered_map>
#include "frame/System.h"
#include "frame_gl/systems/Render.h"
#include "frame_gl/components/Camera.h"
#include "frame_gl/data/Mesh.h"
#include "frame_gl/data/Shader.h"
#include "frame_gl/math.h"
#include "glm/gtc/matrix_transform.hpp"
using namespace frame;

namespace frame_gl
{
    FRAME_SYSTEM(DebugDraw) {
    private:
        struct Range { float min, max; };

        struct Line {
            Line(const glm::vec3& a, const glm::vec3& b, const glm::vec4& color, float thickness = 1.0f)
                : a(a), b(b), color(color), thickness(thickness) {}
            glm::vec3 a, b;
            glm::vec4 color;
            float thickness;
        };

        struct Arrow {
            Arrow(const glm::vec3& base, const glm::vec3& tip, float size, const glm::vec4& color, float thickness = 1.0f)
                : base(base), tip(tip), size(size), color(color), thickness(thickness) {}
            glm::vec3 base, tip;
            glm::vec4 color;
            float size;
            float thickness;
        };

        struct String {
            String(const glm::vec3& position, const std::string& text, const glm::vec4& color, float size, float thickness)
                : position(position), text(text), color(color), size(size), thickness(thickness) {}
            glm::vec3 position;
            std::string text;
            glm::vec4 color;
            float size;
            float thickness;
        };

        struct Circle {
            Circle(const glm::vec3& position, float inner_radius, float outer_radius, const glm::vec4& color)
                : position(position), radii({ inner_radius, outer_radius }), color(color) {}
            glm::vec3 position;
            Range radii;
            glm::vec4 color;
        };

        struct Arc : Circle {
            Arc(const glm::vec3& position, float inner_radius, float outer_radius, float start_angle, float end_angle, const glm::vec4& color)
                : Circle(position, inner_radius, outer_radius, color), angles({ start_angle, end_angle }) {}
            Range angles;
        };

        struct Shape {
            Shape(const std::vector<glm::vec3>& vertices, const glm::vec4& line_color, const glm::vec4& fill_color)
                : vertices(vertices), line_color(line_color), fill_color(fill_color) {}
            std::vector<glm::vec3> vertices;
            glm::vec4 line_color;
            glm::vec4 fill_color;
        };

        struct DebugMesh {
            DebugMesh(Resource<Mesh> mesh, const glm::mat4& transform, const glm::vec4& line_color, const glm::vec4& fill_color, float line_thickness)
                : mesh(mesh), transform(transform), line_color(line_color), fill_color(fill_color), line_thickness(line_thickness) {}
            Resource<Mesh> mesh;
            glm::mat4 transform;
            glm::vec4 line_color;
            glm::vec4 fill_color;
            float line_thickness;
        };

    public:
        enum Alignment { TopLeft, TopRight, BottomLeft, BottomRight };

    public:
        DebugDraw(int main_layer=0, int gui_layer=1)
        : main_layer(main_layer), gui_layer(gui_layer), render(nullptr), main_camera(nullptr), gui_camera(nullptr) {}
        ~DebugDraw() {}

    public:

        void line(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& color=glm::vec3(1.0f), float thickness=1.0f) {
            line(p0, p1, vec4(color, 1.0f), thickness);
        }

        void line(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color = glm::vec4(1.0f), float thickness = 1.0f) {
            lines.push(Line(p0, p1, color, thickness));
        }

        void arrow(const glm::vec3& base, const glm::vec3& tip, float size=0.1f, const glm::vec4& color=vec4(1.0f), float thickness=1.0f) {
            arrows.push(Arrow(base, tip, size, color, thickness));
            line(base, tip, color, thickness);
        }

        void shape(const std::vector<glm::vec3>& vertices, const glm::vec4& line_color=vec4(1.0f), const glm::vec4& fill_color=vec4(vec3(0.5f), 1.0f)) {
            shapes.push(Shape(vertices, line_color, fill_color));
        }

        void screen_shape(const std::vector<glm::vec3>& vertices, const glm::vec4& line_color=vec4(1.0f), const glm::vec4& fill_color=vec4(vec3(0.5f), 1.0f)) {
            screen_shapes.push(Shape(vertices, line_color, fill_color));
        }

        void mesh(Resource<Mesh> mesh, const glm::mat4& transform, const glm::vec4& line_color=vec4(1.0f), const glm::vec4& fill_color=vec4(vec3(0.5f), 1.0f), float line_thickness=1.0f) {
            meshes.push(DebugMesh(mesh, transform, line_color, fill_color, line_thickness));
        }

        void circle(const glm::vec3& position, float inner_radius=5.0f, float outer_radius=10.0f, const glm::vec4& color=glm::vec4(1.0f)) {
            if (outer_radius < inner_radius)
                outer_radius = inner_radius;
            circles.push(Circle(position, inner_radius, outer_radius, color));
        }

        /*
        void arc(const glm::vec3& position, float inner_radius=5.0f, float outer_radius=10.0f, float start_angle=0.0f, float end_angle=pi, const glm::vec4& color=glm::vec4(1.0f)) {
            if (outer_radius < inner_radius)
                outer_radius = inner_radius;
            arcs.push(Arc(position, inner_radius, outer_radius, start_angle, end_angle, color));
        }
        */

        void cube(const glm::vec3& point, float scale=1.0f) {
            cube(glm::scale(glm::translate(glm::mat4(1.0f), point), glm::vec3(scale)));
        }

        void cube(const glm::mat4& transform) {
            cubes.push(transform);
        }

        void world_text(const glm::vec3& position, const std::string& text, const glm::vec3& color, float size = 12.0f, float thickness=1.0f) {
            world_text(position, text, vec4(color, 1.0f), size, thickness);
        }

        void world_text(const glm::vec3& position, const std::string& text, const glm::vec4& color=glm::vec4(1.0f), float size=12.0f, float thickness=1.0f) {
            world_strings.push(String(position, text, color, size, thickness));
        }

        void screen_text(const glm::vec2& position, const std::string& text, const glm::vec4& color=glm::vec4(1.0f), float size=12.0f, float thickness=1.0f) {
            screen_text(TopLeft, position, text, color, size, thickness);
        }

        void screen_text(Alignment alignment, glm::vec2 position, const std::string& text, const glm::vec3& color, float size=12.0f, float thickness=1.0f) {
            screen_text(alignment, position, text, color, size, thickness);
        }

        void screen_text(Alignment alignment, glm::vec2 position, const std::string& text, const glm::vec4& color=glm::vec4(1.0f), float size=12.0f, float thickness=1.0f) {

            // If we haven't got a gui camera, then this won't get rendered anyway
            if (gui_camera == nullptr) return;

            // Get the camera's target's resolution
            ivec2 screen_size = gui_camera->target()->size();

            // Get the actual size of a character in pixels (they are square)
            //float pixel_size = screen_size

            // Adjust the position for alignment
            if (alignment == TopRight || alignment == BottomRight)
                position.x += screen_size.x - float(text.size() * size);
            if (alignment == BottomLeft || alignment == BottomRight)
                position.y += screen_size.y;
            else
                position.y += size;

            screen_strings.push(String(glm::vec3(position.x, position.y, 0.0f), text, color, size, thickness));
        }

    protected:
        void setup() {

            render = system<Render>();

            line_mesh = new Mesh(POSITION_VEC3, 2, 1, false);
            {
                vec4 color(1.0f);
                line_mesh->set_vertices({ vec3(0.0f), vec3(1.0f, 0.0f, 0.0f) });
                line_mesh->set_triangles({ ivec3(0, 0, 1) });
            }

            cube_mesh = new Mesh(DEFAULT_VERTEX_ATTRIBUTES_SIMPLE, 24, 12, false);
            {
                float half(0.5f);
                vec4 color(1.0f);
                cube_mesh->set_vertices({
                    vec3(-half, -half, -half), vec3(-half, -half, -half), vec3(-half, -half, -half),
                    vec3(half, -half, -half), vec3(half, -half, -half), vec3(half, -half, -half),
                    vec3(-half, half, -half), vec3(-half, half, -half), vec3(-half, half, -half),
                    vec3(half, half, -half), vec3(half, half, -half), vec3(half, half, -half),
                    vec3(-half, -half, half), vec3(-half, -half, half), vec3(-half, -half, half),
                    vec3(half, -half, half), vec3(half, -half, half), vec3(half, -half, half),
                    vec3(-half, half, half), vec3(-half, half, half), vec3(-half, half, half),
                    vec3(half, half, half), vec3(half, half, half), vec3(half, half, half)
                }, {
                    vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
                    vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
                    vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
                    vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
                    vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f),
                    vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f),
                    vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f),
                    vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)
                }, {
                    vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f),
                    vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f),
                    vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f),
                    vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f),
                }, {
                    color, color, color, color, color, color,
                    color, color, color, color, color, color,
                    color, color, color, color, color, color,
                    color, color, color, color, color, color
                });

                cube_mesh->set_triangles({
                    ivec3((0*3)+2, (2*3)+2, (1*3)+2),
                    ivec3((1*3)+2, (2*3)+2, (3*3)+2),
                    ivec3((0*3)+1, (1*3)+1, (4*3)+1),
                    ivec3((1*3)+1, (5*3)+1, (4*3)+1),
                    ivec3((4*3)+2, (5*3)+2, (6*3)+2),
                    ivec3((5*3)+2, (7*3)+2, (6*3)+2),
                    ivec3((7*3)+1, (3*3)+1, (6*3)+1),
                    ivec3((3*3)+1, (2*3)+1, (6*3)+1),
                    ivec3(0*3, 4*3, 2*3),
                    ivec3(2*3, 4*3, 6*3),
                    ivec3(1*3, 3*3, 5*3),
                    ivec3(3*3, 7*3, 5*3)
                });
            }

            circle_mesh = new Mesh(DEFAULT_VERTEX_ATTRIBUTES_SIMPLE_DYNAMIC, 0, 0, true);

            arrowhead_mesh = new Mesh(DEFAULT_VERTEX_ATTRIBUTES_SIMPLE, 5, 2, false);
            {
                float size = 1.0f;
                vec3 norm = vec3(1.0f, 0.0f, 0.0f);
                vec3 perp1 = orthogonal(norm);
                vec3 perp2 = cross(norm, perp1);
                vec3 tip = -norm * dot(norm, size * (perp1 - norm));
                vec3 head1 = tip + size * (perp1 - norm);
                vec3 head2 = tip + size * (-perp1 - norm);
                vec3 head3 = tip + size * (perp2 - norm);
                vec3 head4 = tip + size * (-perp2 - norm);
                arrowhead_mesh->set_vertices(
                    { tip, head1, head2, head3, head4 },
                    { vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f) },
                    { vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f) },
                    { vec4(1.0f), vec4(1.0f), vec4(1.0f), vec4(1.0f), vec4(1.0f) });
                arrowhead_mesh->set_triangles({ ivec3(0, 1, 2), ivec3(0, 3, 4) });
            }

            cube_shader = Resource<Shader>(
                "Debug Cube Shader",
                Shader::Preset::vert_standard(),
                Shader::Preset::frag_normals());

            shape_shader = Resource<Shader>(
                "Debug Shape Shader",
                Shader::Preset::vert_standard(),
                Shader::Preset::frag_solid());

            auto arc_shader_vert = Resource<ShaderPart>(ShaderPart::Type::Vertex,
                "#version 330\n                                                 "
                "layout(location = 0)in vec3 vert_position;                     "
                "layout(location = 1)in vec3 vert_normal;                       "
                "layout(location = 2)in vec2 vert_uv;                           "
                "layout(location = 3)in vec4 vert_color;                        "
                "uniform mat4 model;                                            "
                "uniform mat4 view;                                             "
                "uniform mat4 projection;                                       "
                "out vec4 geom_position;                                        "
                "out vec2 geom_angles;                                          "
                "out vec2 geom_radii;                                           "
                "out vec4 geom_color;                                           "
                "void main() {                                                  "
                "   mat4 transform  = projection * view * model;                "
                "   geom_position   = transform * vec4(vert_position, 1.0);     "
                "   geom_angles     = vert_normal.xy;                           "
                "   geom_radii      = vert_uv;                                  "
                "   geom_color      = vert_color;                               "
                "   gl_Position     = geom_position;                            "
                "}                                                              ");

            circle_shader = Resource<Shader>(
                "Debug Circle Shader",
                arc_shader_vert,
                Resource<ShaderPart>(ShaderPart::Type::Geometry,
                    "#version 330\n"
                    "#define pi 3.1415926535897932384626433832795\n"
                    "layout(triangles) in;"
                    "layout(triangle_strip, max_vertices = 128) out;"
                    "uniform vec2 screen_size;"
                    "in vec4 geom_position[3];"
                    "in vec2 geom_radii[3];"
                    "in vec4 geom_color[3];"
                    "out vec4 frag_color;"
                    "void main() {"
                    "   vec4 scale = vec4(1.0f / screen_size.x, 1.0f / screen_size.y, 0, 0) * (geom_position[0].w);"
                    "   vec4 center = geom_position[0];                             "
                    "   float inner_radius = geom_radii[0].x;                       "
                    "   float outer_radius = geom_radii[0].y;                       "
                    "   for (int i = 0; i < 64; ++i) {                              "
                    "       float t = float(i) / 63;                                "
                    "       float c = cos(2 * pi * t);                              "
                    "       float s = sin(2 * pi * t);                              "
                    "       gl_Position = center + (scale * inner_radius) * vec4(c, s, 0, 0); "
                    "       frag_color = geom_color[0];                             "
                    "       EmitVertex();                                           "
                    "       gl_Position = center + (scale * outer_radius) * vec4(c, s, 0, 0); "
                    "       frag_color = geom_color[0];                             "
                    "       EmitVertex();                                           "
                    "   }                                                           "
                    "}"
                    ),

                Resource<ShaderPart>(ShaderPart::Type::Fragment,
                    "#version 330\n"
                    "in vec4 frag_color;"
                    "out vec4 pixel_color;"
                    "void main() {"
                    "   pixel_color = frag_color;"
                    "}"
                ));

            arc_shader = Resource<Shader>(
                "Debug Arc Shader",
                arc_shader_vert,
                Resource<ShaderPart>(ShaderPart::Type::Geometry,
                    "#version 330\n                                                                 "
                    "#define pi 3.1415926535897932384626433832795\n                                 "
                    "layout(lines) in;                                                              "
                    "layout(triangle_strip, max_vertices = 128) out;                                "
                    "uniform vec2 screen_size;                                                      "
                    "in vec4 geom_position[2];                                                      "
                    "in vec2 geom_angles[2];                                                        "
                    "in vec2 geom_radii[2];                                                         "
                    "in vec4 geom_color[2];                                                         "
                    "out vec4 frag_color;                                                           "
                    "void main() {                                                                  "
                    "   vec4 scale = vec4(1.0f / screen_size.x, 1.0f / screen_size.y, 0, 0) * (geom_position[0].w);"
                    "   vec4 center = geom_position[0];                                             "
                    "   float inner_radius = geom_radii[0].x;                                       "
                    "   float outer_radius = geom_radii[0].y;                                       "
                    "   float start_angle = geom_angles[0].x;                                       "
                    "   float end_angle = geom_angles[0].y;                                         "
                    "   for (int i = 0; i < 64; ++i) {                                              "
                    "       float t = float(i) / 63;                                                "
                    "       float angle = start_angle + (end_angle - start_angle) * t;              "
                    "       float c = cos(angle);                                                   "
                    "       float s = sin(angle);                                                   "
                    "       gl_Position = center + (scale * inner_radius) * vec4(c, s, 0, 0);       "
                    "       frag_color = geom_color[0];                                             "
                    "       EmitVertex();                                                           "
                    "       gl_Position = center + (scale * outer_radius) * vec4(c, s, 0, 0);       "
                    "       frag_color = geom_color[0];                                             "
                    "       EmitVertex();                                                           "
                    "   }                                                                           "
                    "}"
                    ),

                Resource<ShaderPart>(ShaderPart::Type::Fragment,
                    "#version 330\n"
                    "in vec4 frag_color;"
                    "out vec4 pixel_color;"
                    "void main() {"
                    "   pixel_color = frag_color;"
                    "}"
                    ));

            text_shader = Resource<Shader>(
                "Debug Text Shader",

                Resource<ShaderPart>(ShaderPart::Type::Vertex,
                    "#version 330\n                                                 "
                    "layout(location = 0)in vec3 vert_position;                     "
                    "uniform mat4 model;                                            "
                    "uniform mat4 view;                                             "
                    "uniform mat4 projection;                                       "
                    "out vec4 geom_position;                                        "
                    "void main() {                                                  "
                    "    mat4 transform = projection * view * model;                "
                    "    geom_position  = transform * vec4(vert_position, 1.0);     "
                    "    gl_Position    = geom_position;                            "
                    "}                                                              "
                ),

                Resource<ShaderPart>(ShaderPart::Type::Geometry,
                    "#version 330\n"
                    "layout(triangles) in;"
                    "layout(line_strip, max_vertices = 16) out;"
                    "uniform float character_size;"
                    "uniform int character_number;"
                    "uniform int character_code;"
                    "uniform vec2 screen_size;"
                    "in vec4 geom_position[3];"
                    "void main() {"
                    "   vec4 scale = vec4(1.0f / screen_size.x, 1.0f / screen_size.y, 0, 0) * (geom_position[0].w * character_size);"
                    "   vec4 pos = vec4((character_number) * scale.x, 0, 0, 0) + geom_position[0];"

                    // SPACE
                    "   if (character_code == 32) {" 
                    "   }"

                    // -
                    "   if (character_code == 45) {"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale; EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .5, 0, 0) * scale; EmitVertex();"
                    "   }"

                    // .
                    "   if (character_code == 46) {"
                    "       gl_Position = pos + vec4(.45, .1, 0, 0) * scale; EmitVertex();"
                    "       gl_Position = pos + vec4(.55, .1, 0, 0) * scale; EmitVertex();"
                    "       gl_Position = pos + vec4(.55, 0, 0, 0) * scale; EmitVertex();"
                    "       gl_Position = pos + vec4(.45, 0, 0, 0) * scale; EmitVertex();"
                    "   }"

                    // 0
                    "   else if (character_code == 48) {"
                    "       gl_Position = pos + vec4(.25, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.3, .05, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.7, .95, 0, 0) * scale;    EmitVertex();"
                    "   }"

                    // 1
                    "   else if (character_code == 49) {"
                    "       gl_Position = pos + vec4(.35, 0, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 0, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.5, 0, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.5, 1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.4, .9, 0, 0) * scale;    EmitVertex();"
                    "   }"

                    // 2
                    "   else if (character_code == 50) {"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, .35, 0, 0) * scale;   EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .45, 0, 0) * scale;   EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .9, 0, 0) * scale;    EmitVertex();"
                    "   }"

                    // 3
                    "   else if (character_code == 51) {"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .6, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.5, .5, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .4, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // 4
                    "   else if (character_code == 52) {"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // 5
                    "   else if (character_code == 53) {"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .75, 0, 0) * scale;   EmitVertex();"
                    "       gl_Position = pos + vec4(.65, .65, 0, 0) * scale;   EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .55, 0, 0) * scale;   EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .1, 0, 0) * scale;    EmitVertex();"
                    "   }"

                    // 6
                    "   else if (character_code == 54) {"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .4, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .4, 0, 0) * scale;    EmitVertex();"
                    "   }"

                    // 7
                    "   else if (character_code == 55) {"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // 8
                    "   else if (character_code == 56) {"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // 9
                    "   else if (character_code == 57) {"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .6, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .6, 0, 0) * scale;    EmitVertex();"
                    "   }"

                    // A
                    "   else if (character_code == 65 || character_code == 65+32) {"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "   }"

                    // B
                    "   else if (character_code == 66 || character_code == 66+32) {"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, .0, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .0, 0, 0) * scale;    EmitVertex();"
                    "   }"

                    // C
                    "   else if (character_code == 67 || character_code == 67+32) {"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // D
                    "   else if (character_code == 68 || character_code == 68+32) {"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // E
                    "   else if (character_code == 69 || character_code == 69+32) {"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // F
                    "   else if (character_code == 70 || character_code == 70+32) {"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // G
                    "   else if (character_code == 71 || character_code == 71+32) {"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.5, .5, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // H
                    "   else if (character_code == 72 || character_code == 72+32) {"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // I
                    "   else if (character_code == 73 || character_code == 73+32) {"
                    "       gl_Position = pos + vec4(.35, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.5, 0, 0, 0) * scale;      EmitVertex();"
                    "       gl_Position = pos + vec4(.5, 1, 0, 0) * scale;      EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // J
                    "   else if (character_code == 74 || character_code == 74+32) {"
                    "       gl_Position = pos + vec4(.35, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.5, 1, 0, 0) * scale;      EmitVertex();"
                    "       gl_Position = pos + vec4(.5, .1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.4, 0, 0, 0) * scale;      EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // K
                    "   else if (character_code == 75 || character_code == 75+32) {"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.55, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, .6, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .7, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .7, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, .6, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.55, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, .4, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .3, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // L
                    "   else if (character_code == 76 || character_code == 76+32) {"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // M
                    "   else if (character_code == 77 || character_code == 77+32) {"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.4, 1, 0, 0) * scale;      EmitVertex();"
                    "       gl_Position = pos + vec4(.5, .9, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.5, .35, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.5, .9, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.6, 1, 0, 0) * scale;      EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // N
                    "   else if (character_code == 78 || character_code == 78+32) {"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // O
                    "   else if (character_code == 79 || character_code == 79+32) {"
                    "       gl_Position = pos + vec4(.25, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .1, 0, 0) * scale;    EmitVertex();"
                    "   }"

                    // P
                    "   else if (character_code == 80 || character_code == 80+32) {"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .6, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "   }"

                    // Q
                    "   else if (character_code == 81 || character_code == 81+32) {"
                    "       gl_Position = pos + vec4(.65, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.7, .05, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.85, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.95, .1, 0, 0) * scale;    EmitVertex();"
                    "   }"

                    // R
                    "   else if (character_code == 82 || character_code == 82+32) {"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .6, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .4, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // S
                    "   else if (character_code == 83 || character_code == 83+32) {"
                    "       gl_Position = pos + vec4(.75, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .9, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .6, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.35, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, .5, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .4, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .1, 0, 0) * scale;    EmitVertex();"
                    "   }"

                    // T
                    "   else if (character_code == 84 || character_code == 84+32) {"
                    "       gl_Position = pos + vec4(.5, 0, 0, 0) * scale;      EmitVertex();"
                    "       gl_Position = pos + vec4(.5, 1, 0, 0) * scale;      EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // U
                    "   else if (character_code == 85 || character_code == 85+32) {"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // V
                    "   else if (character_code == 86 || character_code == 86+32) {"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.5, 0, 0, 0) * scale;      EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // W
                    "   else if (character_code == 87 || character_code == 87+32) {"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.35, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.4, 0, 0, 0) * scale;      EmitVertex();"
                    "       gl_Position = pos + vec4(.5, .1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.5, .5, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.5, .1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.6, 0, 0, 0) * scale;      EmitVertex();"
                    "       gl_Position = pos + vec4(.65, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .1, 0, 0) * scale;    EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // X
                    "   else if (character_code == 88 || character_code == 88+32) {"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.5, .5, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // Y
                    "   else if (character_code == 89 || character_code == 89+32) {"
                    "       gl_Position = pos + vec4(.5, 0, 0, 0) * scale;      EmitVertex();"
                    "       gl_Position = pos + vec4(.5, .5, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.5, .5, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // Z
                    "   else if (character_code == 90 || character_code == 90+32) {"
                    "       gl_Position = pos + vec4(.25, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 1, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.25, 0, 0, 0) * scale;     EmitVertex();"
                    "       gl_Position = pos + vec4(.75, 0, 0, 0) * scale;     EmitVertex();"
                    "   }"

                    // [
                    "   if (character_code == 91) {"
                    "       gl_Position = pos + vec4(.45, .1, 0, 0) * scale; EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .1, 0, 0) * scale; EmitVertex();"
                    "       gl_Position = pos + vec4(.25, .9, 0, 0) * scale; EmitVertex();"
                    "       gl_Position = pos + vec4(.45, .9, 0, 0) * scale; EmitVertex();"
                    "   }"

                    // ]
                    "   if (character_code == 93) {"
                    "       gl_Position = pos + vec4(.55, .1, 0, 0) * scale; EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .1, 0, 0) * scale; EmitVertex();"
                    "       gl_Position = pos + vec4(.75, .9, 0, 0) * scale; EmitVertex();"
                    "       gl_Position = pos + vec4(.55, .9, 0, 0) * scale; EmitVertex();"
                    "   }"

                    "   EndPrimitive();"
                    "}"
                ),

                Resource<ShaderPart>(ShaderPart::Type::Fragment,
                    "#version 330\n"
                    "uniform vec4 character_color;"
                    "out vec4 pixel_color;"
                    "void main() {"
                    "    pixel_color = character_color;"
                    "}"
                )
            );
        }

        void teardown() {
            delete line_mesh;
            delete cube_mesh;
            delete circle_mesh;
            delete arrowhead_mesh;
        }

        void step() {

            // Get cameras
            main_camera = render->display_camera(main_layer);
            gui_camera = render->display_camera(gui_layer);

            // Get the main display camera
            if (main_camera) {

                // Bind the render target
                main_camera->bind_target();

                // Draw worldspace stuff
                render_lines(main_camera);
                render_arrows(main_camera);
                render_shapes(main_camera, shapes);
                render_meshes(main_camera);
                render_cubes(main_camera);
                render_circles(main_camera);
                render_text(main_camera, world_strings);

                // Tear down
                main_camera->unbind_target();
            }

            // Get the GUI display camera
            if (gui_camera) {

                // Bind the render target
                gui_camera->bind_target();

                // Draw gui (screen) space stuff
                render_text(gui_camera, screen_strings);

                render_shapes(gui_camera, screen_shapes);

                // Tear down
                gui_camera->unbind_target();
            }
        }

    private:

        void render_lines(Camera* camera) {

            if (lines.empty())
                return;

            // Bind the shape shader
            shape_shader->bind();
            shape_shader->uniform(ShaderUniform::View, camera->view_matrix());
            shape_shader->uniform(ShaderUniform::Projection, camera->projection_matrix());
            int color = shape_shader->locate("color");

            // Just need one model matrix
            //line_shader->uniform(ShaderUniform::Model, glm::mat4(1.0f));

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            line_mesh->bind();

            while (!lines.empty()) {
                auto& line = lines.front();

                glLineWidth(line.thickness);

                vec3 ab = line.b - line.a;
                mat4 transform = glm::translate(mat4(1.0f), line.a) * glm::scale(quat(vec3(1.0f, 0.0f, 0.0f), ab).matrix(), vec3(length(ab)));
                shape_shader->uniform(ShaderUniform::Model, transform);
                shape_shader->uniform(color, line.color);

                line_mesh->render();
                lines.pop();
            };

            line_mesh->unbind();
            shape_shader->unbind();
        }

        void render_arrows(Camera* camera) {

            if (arrows.empty())
                return;

            // Bind the shape shader
            shape_shader->bind();
            shape_shader->uniform(ShaderUniform::View, camera->view_matrix());
            shape_shader->uniform(ShaderUniform::Projection, camera->projection_matrix());

            // Find the color uniform location
            int color = shape_shader->locate("color");

            // Set up gl state
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Draw all the arrowheads
            arrowhead_mesh->bind();
            while (!arrows.empty()) {
                auto& arrow = arrows.front();
                glLineWidth(float(arrow.thickness));
                mat4 rotate = quat(vec3(1.0f, 0.0f, 0.0f), arrow.tip - arrow.base).matrix();
                mat4 translate = glm::translate(mat4(1.0f), arrow.tip);
                mat4 scale = glm::scale(mat4(1.0f), vec3(arrow.size));
                shape_shader->uniform(ShaderUniform::Model, translate * rotate * scale);
                shape_shader->uniform(color, arrow.color);
                arrowhead_mesh->draw();
                arrows.pop();
            }
            arrowhead_mesh->unbind();

            shape_shader->unbind();
        }

        void render_shapes(Camera* camera, std::queue< Shape >& shapes_queue) {

            if (shapes_queue.empty())
                return;

            // Bind the shape shader
            shape_shader->bind();
            shape_shader->uniform(ShaderUniform::View, camera->view_matrix());
            shape_shader->uniform(ShaderUniform::Projection, camera->projection_matrix());

            // Just need one model matrix
            shape_shader->uniform(ShaderUniform::Model, glm::mat4(1.0f));

            // Build a mesh with a bunch of lines
            std::vector< std::tuple< vec4, vec4, Resource< Mesh > > > meshes;
            while (!shapes_queue.empty()) {
                Shape& shape = shapes_queue.front();
                Resource< Mesh > mesh(DEFAULT_VERTEX_ATTRIBUTES_SIMPLE, shape.vertices.size(), shape.vertices.size() - 2);
                for (size_t i = 0; i < shape.vertices.size(); ++i) {
                    mesh->set_vertex(i, shape.vertices[i], vec3(0.0f), vec2(0.0f), vec4(1.0f));
                    if (i > 1)
                        mesh->set_triangle(i-2, ivec3(0, i-1, i));
                }
                meshes.push_back(std::make_tuple(shape.fill_color, shape.line_color, mesh));
                shapes_queue.pop();
            }

            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            int color = shape_shader->locate("color");

            // Draw shape fills
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            for (auto& mesh : meshes) {
                if (std::get<0>(mesh).a == 0.0f) continue;
                shape_shader->uniform(color, std::get<0>(mesh));
                std::get<2>(mesh)->render();
            }

            // Draw shape lines
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(2.0f);
            for (auto& mesh : meshes) {
                if (std::get<1>(mesh).a == 0.0f) continue;
                shape_shader->uniform(color, std::get<1>(mesh));
                std::get<2>(mesh)->render();
            }

            shape_shader->unbind();
        }

        void render_meshes(Camera* camera) {

            if (meshes.empty())
                return;

            // Bind the shape shader
            shape_shader->bind();
            shape_shader->uniform(ShaderUniform::View, camera->view_matrix());
            shape_shader->uniform(ShaderUniform::Projection, camera->projection_matrix());

            int color = shape_shader->locate("color");

            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Build a mesh with a bunch of lines
            while (!meshes.empty()) {
                DebugMesh& mesh = meshes.front();

                shape_shader->uniform(ShaderUniform::Model, mesh.transform);

                // Draw shape fills
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                if (mesh.fill_color.a > std::numeric_limits<float>::epsilon()) {
                    shape_shader->uniform(color, mesh.fill_color);
                    mesh.mesh->render();
                }

                // Draw shape lines
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(mesh.line_thickness);
                if (mesh.line_color.a > std::numeric_limits<float>::epsilon()) {
                    shape_shader->uniform(color, mesh.line_color);
                    mesh.mesh->render();
                }

                meshes.pop();
            }

            shape_shader->unbind();
        }

        void render_cubes(Camera* camera) {

            if (cubes.empty())
                return;

            // Bind the shape shader
            cube_shader->bind();
            cube_shader->uniform(ShaderUniform::View, camera->view_matrix());
            cube_shader->uniform(ShaderUniform::Projection, camera->projection_matrix());

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_CULL_FACE);

            while (!cubes.empty()) {
                cube_shader->uniform(ShaderUniform::Model, cubes.front());
                cube_mesh->render();
                cubes.pop();
            }

            cube_shader->unbind();
        }

        void render_circles(Camera* camera) {

            if (circles.empty())
                return;

            circle_shader->bind();
            circle_shader->uniform(ShaderUniform::View, camera->view_matrix());
            circle_shader->uniform(ShaderUniform::Projection, camera->projection_matrix());
            circle_shader->uniform(ShaderUniform::Model, glm::mat4(1.0f));
            circle_shader->uniform("screen_size", camera->target()->size());

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);

            circle_mesh->resize(circles.size(), circles.size());
            int index = 0;
            while (!circles.empty()) {
                auto& circle = circles.front();
                circle_mesh->set_vertex(index, circle.position, vec3(1.0f, 0.0f, 0.0f), vec2(circle.radii.min, circle.radii.max), circle.color);
                circle_mesh->set_triangle(index, ivec3(index));
                ++index;
                circles.pop();
            }

            circle_mesh->render();
            circle_shader->unbind();
        }

        void render_text(Camera* camera, std::queue< String >& strings) {

            if (strings.empty() && strings.empty())
                return;

            // Find the uniforms we'll be tweaking
            int character_color = text_shader->locate("character_color");
            int character_number = text_shader->locate("character_number");
            int character_code = text_shader->locate("character_code");
            int character_size = text_shader->locate("character_size");

            text_shader->uniform("screen_size", camera->target()->size());

            // Bind the text shader
            text_shader->bind();

            Mesh mesh(1, 1);
            mesh.set_vertices({ vec3(0.0f) });
            mesh.set_triangles({ ivec3(0, 0, 0) });

            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);

            // Draw each string
            text_shader->uniform(ShaderUniform::View, camera->view_matrix());
            text_shader->uniform(ShaderUniform::Projection, camera->projection_matrix());

            while (!strings.empty()) {
                const String& line = strings.front();
                text_shader->uniform(ShaderUniform::Model, glm::translate(glm::mat4(1.0f), line.position));
                text_shader->uniform(character_size, line.size);
                text_shader->uniform(character_color, line.color);

                glLineWidth(line.thickness);
                int i = 0;
                for (char c : line.text) {
                    text_shader->uniform(character_number, i++);
                    text_shader->uniform(character_code, (int)c);
                    mesh.render();
                }
                strings.pop();
            }

            text_shader->unbind();
        }

    private:
        Render* render;
        Camera* main_camera;
        Camera* gui_camera;

        Resource<Shader> shape_shader;
        Resource<Shader> cube_shader;
        Resource<Shader> circle_shader;
        Resource<Shader> arc_shader;
        Resource<Shader> text_shader;

        Mesh* line_mesh;
        Mesh* cube_mesh;
        Mesh* circle_mesh;
        Mesh* arrowhead_mesh;

        int text_shader_characters;
        std::queue< Line > lines;
        std::queue< Arrow > arrows;
        std::queue< Shape > shapes;
        std::queue< Shape > screen_shapes;
        std::queue< DebugMesh > meshes;
        std::queue< glm::mat4 > cubes;
        std::queue< String > world_strings;
        std::queue< String > screen_strings;
        std::queue< Circle > circles;
        //std::queue< Arc > arcs;

        std::atomic<bool> loaded;

        int main_layer;
        int gui_layer;
    };
}