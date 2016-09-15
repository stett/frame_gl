#pragma once
#include "frame/Resource.h"
#include "frame_gl/math.h"

namespace frame
{
    class Texture {
    public:
        Texture(const ivec2& size=ivec2(1), bool multisample=true);
        Texture(const std::string& filename);
        ~Texture();
        Texture(const Texture& other) = delete;
        Texture& operator=(const Texture& other) = delete;

    public:
        const ivec2& size() const { return _size; }
        unsigned int id() const { return _id; }
        bool multisample() const { return _multisample; }
        void bind(unsigned int texture_unit);
        void unbind();

    private:
        ivec2 _size;
        unsigned int _id;
        bool _multisample;
    };
}