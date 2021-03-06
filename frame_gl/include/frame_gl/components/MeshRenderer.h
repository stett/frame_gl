#pragma once
#define GLEW_STATIC
#include <GLFW/glfw3.h>
#include "frame/Component.h"
#include "frame_gl/components/Transform.h"
#include "frame_gl/components/Camera.h"
#include "frame_gl/data/Mesh.h"
#include "frame_gl/data/MeshFactory.h"
#include "frame_gl/data/Texture.h"
#include "frame_gl/data/Shader.h"
#include "frame/Resource.h"

namespace frame
{
    FRAME_COMPONENT(MeshRenderer, Transform) {
    public:
        enum PolyMode {
            Fill = GL_FILL,
            Line = GL_LINE,
            Point = GL_POINT
        };

    public:
        MeshRenderer() : _mesh(MeshFactory::cube()), _texture(Texture::white_pixel()), _shader(Shader::Preset::model_colors()), _poly_mode(Fill), _cull_back(true), _layer(0) {}
        MeshRenderer(Resource<Mesh> mesh, Resource<Texture> texture, Resource<Shader> shader, PolyMode poly_mode=Fill, bool cull_back=true, unsigned int layer=0)
        : _mesh(mesh), _texture(texture), _shader(shader), _poly_mode(poly_mode), _cull_back(cull_back), _layer(layer) {}

    public:

        void draw(Camera* camera) const {
            bind(camera);
            render(camera);
            unbind();
        }

        void bind(Camera* camera) const {

            // Set GL state
            glPolygonMode(GL_FRONT_AND_BACK, _poly_mode);
            if (_cull_back) glEnable(GL_CULL_FACE);
            else glDisable(GL_CULL_FACE);

            // Get transformation matrices
            auto model = get<Transform>()->world_matrix();
            auto view = camera->view_matrix();
            auto projection = camera->projection_matrix();

            // Bind stuff
            _texture->bind(0);
            _shader->bind();
            _shader->uniform(ShaderUniform::Model, model);
            _shader->uniform(ShaderUniform::View, view);
            _shader->uniform(ShaderUniform::Projection, projection);
        }

        void render(Camera* camera) const {

            // Render
            _mesh->render();
        }

        void unbind() const {
            _shader->unbind();
            _texture->unbind();
        }

    public:
        MeshRenderer* set_shader(Resource<Shader> shader) { _shader = shader; return this; }
        MeshRenderer* set_layer(unsigned int layer) { _layer = layer; return this; }

    public:
        Resource<Mesh> mesh() { return _mesh; }
        Resource<Texture> texture() { return _texture; }
        Resource<Shader> shader() { return _shader; }
        unsigned int layer() { return _layer; }

    protected:
        void write(Archive& archive) {
            archive.write<int>(_poly_mode);
            archive.write(_cull_back);
            archive.write(_layer);
            archive.write(_mesh.index());
            archive.write(_texture.index());
            archive.write(_shader.index());
        }

        void read(Archive& archive) {
            int poly_mode_int;
            archive.read<int>(poly_mode_int);
            _poly_mode = (PolyMode)poly_mode_int;

            archive.read(_cull_back);
            archive.read(_layer);

            // Load the body and shape resources.
            size_t mesh_index, texture_index, shader_index;
            archive.read(mesh_index);
            archive.read(texture_index);
            archive.read(shader_index);
            _mesh.lookup(mesh_index);
            _texture.lookup(texture_index);
            _shader.lookup(shader_index);
        }

    private:
        Resource<Mesh> _mesh;
        Resource<Texture> _texture;
        Resource<Shader> _shader;
        PolyMode _poly_mode;
        bool _cull_back;
        unsigned int _layer;
    };
}