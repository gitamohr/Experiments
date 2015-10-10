#include <stdio.h>
#include <iostream>
#include <memory>
#include <bitset>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <cassert>
#include <vector>
#include <type_traits>
#include <random>
#include <vrm/sdl.hpp>

const char* vShaderStr = R"(
    attribute vec4 position; 
    attribute vec4 color;

    varying vec4 vertex_color;

    void main()
    {
        vertex_color = color;
        gl_Position = vec4(position.xyz, 1.0); 
    })";

const char* fShaderStr = R"(
    precision mediump float;

    varying vec4 vertex_color;

    void main()
    {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        // gl_FragColor = vertex_color;
    })";

namespace sdl = vrm::sdl;

/*
foreach(entity/object drawable)
{
    // Bind VAO & Shader
    // glDrawArrays / glDrawElements
    // Unbind VAO & Shader
}
*/

/*
foreach(vertexarray draw call)
{
    apply transform;
    apply view;
    apply blend mode;
    apply texture;

    apply shader;
        use program; (stores a ptr to it)
        bind textures;

        un-use program;

    glVertexPointer;
        glCheck(glVertexPointer(2, GL_FLOAT, sizeof(Vertex), data + 0));

    glColorPointer;
        glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), data + 8));

    glTexCoordPointer;
        glCheck(glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), data + 12));

    glDrawArrays;

    unbind shader;
}
*/

namespace vrm
{
    namespace sdl
    {
        enum class primitive
        {
            lines,
            triangles,
            // quads, // use index buffer (element buffer) + triangles
            triangle_strip,
            triangle_fan
        };

        namespace impl
        {
            template <primitive TP>
            struct primitive_traits;

            template <>
            struct primitive_traits<primitive::lines>
            {
                static constexpr GLenum gl_value{GL_LINES};
            };

            template <>
            struct primitive_traits<primitive::triangles>
            {
                static constexpr GLenum gl_value{GL_TRIANGLES};
            };

            template <>
            struct primitive_traits<primitive::triangle_strip>
            {
                static constexpr GLenum gl_value{GL_TRIANGLE_STRIP};
            };

            template <>
            struct primitive_traits<primitive::triangle_fan>
            {
                static constexpr GLenum gl_value{GL_TRIANGLE_FAN};
            };
        }

        struct color
        {
            float r, g, b, a;
        };

        struct vertex2
        {
            static constexpr auto position_offset = 0;
            static constexpr auto color_offset = sizeof(vec2f);

            vec2f _position;
            color _color;
            // vec2f tex_coords;

            vertex2(const vec2f& position) noexcept : _position{position} {}

            
            vertex2(const vec2f& position, const color& color) noexcept
                : _position{position},
                  _color{color}
            {
            }
            
        };

        template <primitive TP>
        class primitive_vector
        {
        private:
            std::vector<vertex2> _vertices;
            sdl::impl::unique_vao _vao;
            sdl::impl::unique_vbo _vbo;

            using my_primitive_traits = impl::primitive_traits<TP>;
            static constexpr auto primitive_gl_value =
                my_primitive_traits::gl_value;
            static constexpr auto vertex_dimensions = 2;


        public:
            primitive_vector() noexcept
            {
                _vao = sdl::make_vao(1);
                _vbo = sdl::make_vbo(1);
            }

            template <typename... Ts>
            void add(Ts&&... xs)
            {
                _vertices.emplace_back(FWD(xs)...);
            }

            template <typename... Ts>
            void add_more(Ts&&... xs)
            {
                for_args(
                    [this](auto&& x)
                    {
                        add(FWD(x));
                    },
                    FWD(xs)...);
            }

            void draw(program& p)
            {
                auto data(_vertices.data());

                // Bind vao
                _vao->bind();

                // Bind vbo and buffer vertex data
                _vbo->bind(GL_ARRAY_BUFFER);
                _vbo->buffer_data(
                    GL_ARRAY_BUFFER, GL_STATIC_DRAW, data, _vertices.size());

                // Enable pos attribute
                auto pos(p.get_attribute("position"));
                pos.enable();
                pos.vertex_attrib_pointer(vertex_dimensions, GL_FLOAT, true, sizeof(vertex2), (void*)vertex2::position_offset);
                // pos.vertex_attrib_pointer(vertex_dimensions, GL_FLOAT, true, sizeof(vertex2), data + vertex2::position_offset);

                // Enable color attribute
                auto col(p.get_attribute("color"));
                col.enable();
                col.vertex_attrib_pointer(vertex_dimensions, GL_FLOAT, true, sizeof(vertex2), (void*)vertex2::color_offset);

                // glDrawArrays(primitive_gl_value, 0, _vertices.size());
                glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
            }
        };
    }
}
/*
struct test_vtx2D_group
{
    std::vector<float> vertices;
    sdl::impl::unique_vao _vao;
    sdl::impl::unique_vbo _vbo;

    test_vtx2D_group()
    {
        _vao = sdl::make_vao(1);
        _vbo = sdl::make_vbo(1);
    }

    void add(const sdl::vec2f v)
    {
        vertices.emplace_back(v.x());
        vertices.emplace_back(v.y());
    }

    // TODO: specialize buffer_data for std::vector
    template <typename TF>
    void draw(TF&& f)
    {
        _vao->bind();
        _vbo->bind(GL_ARRAY_BUFFER);
        _vbo->buffer_data(
            GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertices.data(), vertices.size());

        f();

        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);
    }
};*/

int main(int argc, char** argv)
{
    std::random_device rnd_device;
    std::default_random_engine rnd_gen{rnd_device()};

    auto rndf = [&](auto min, auto max)
    {
        using common_min_max_t =
            std::common_type_t<decltype(min), decltype(max)>;

        using dist_t = std::uniform_real_distribution<common_min_max_t>;

        return dist_t{min, max}(rnd_gen);
    };

    auto add_rnd_triangle = [&](auto& v)
    {
        auto c_x = rndf(-1.f, 1.f);
        auto c_y = rndf(-1.f, 1.f);
        auto sz = rndf(0.01f, 0.05f);

        auto p0 = sdl::make_vec2(c_x - sz, c_y - sz);
        auto p1 = sdl::make_vec2(c_x + sz, c_y - sz);
        auto p2 = sdl::make_vec2(c_x + sz, c_y);

        v.add_more(p0, p1, p2);
    };

    auto c_handle(sdl::make_global_context("test game", 1000, 600));
    auto& c(*c_handle);

    sdl::primitive_vector<sdl::primitive::triangles> triangles;
    add_rnd_triangle(triangles);
    add_rnd_triangle(triangles);
    add_rnd_triangle(triangles);

    // GLfloat vertices[] = {0.0f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f};

    // Create Vertex Array Object
    /*
    auto vao(sdl::make_vao(1));
    vao->bind();

    // Create a Vertex Buffer Object and copy the vertex data to it
    auto vbo(sdl::make_vbo(1));
    vbo->bind(GL_ARRAY_BUFFER);
    vbo->buffer_data(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertices);
    */

    auto v_shader = sdl::make_shader(GL_VERTEX_SHADER, &vShaderStr);
    auto f_shader = sdl::make_shader(GL_FRAGMENT_SHADER, &fShaderStr);
    auto program = sdl::make_program(*v_shader, *f_shader);
    program.use();


    // Specify the layout of the vertex data
    /*
    auto posAttrib(program.get_attribute("position"));
    posAttrib.enable();
    posAttrib.vertex_attrib_pointer(2, GL_FLOAT);
    */

    auto enablePos = [&]
    {
        auto posAttrib(program.get_attribute("position"));
        posAttrib.enable();
        posAttrib.vertex_attrib_pointer(2, GL_FLOAT);
    };

    // enablePos();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // glEnableVertexAttribArray(posAttrib);
    // glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    c.update_fn() = [&](auto)
    {
        if(c.key(sdl::kkey::space)) add_rnd_triangle(triangles);
    };

    c.draw_fn() = [&]
    {
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        triangles.draw(program);
        // triangle1.draw(enablePos);
    };

    sdl::run_global_context();
    return 0;
}


int main_old(int argc, char** argv)
{
    // SDL_Init(SDL_INIT_VIDEO);

    auto c_handle(sdl::make_global_context("test game", 1000, 600));
    auto& c(*c_handle);

    auto v_shader = sdl::make_shader(GL_VERTEX_SHADER, &vShaderStr);
    auto f_shader = sdl::make_shader(GL_FRAGMENT_SHADER, &fShaderStr);
    auto program = sdl::make_program(*v_shader, *f_shader);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    auto toriel_image(c.make_image("files/toriel.png"));
    auto soul_image(c.make_image("files/soul.png"));
    auto fireball_image(c.make_image("files/fireball.png"));

    auto toriel_texture(c.make_texture(*toriel_image));
    auto soul_texture(c.make_texture(*soul_image));
    auto fireball_texture(c.make_texture(*fireball_image));

    struct entity
    {
        sdl::vec2f _pos;
        float _hitbox_radius;
        sdl::sprite _sprite;
        bool alive{true};
        std::function<void(entity&, sdl::ft)> _update_fn;
        std::function<void(entity&)> _draw_fn;
    };

    constexpr sdl::sz_t max_entities{10000};
    std::vector<entity> entities;
    entities.reserve(max_entities);

    auto make_soul = [&](auto pos)
    {
        entity e;
        e._pos = pos;
        e._hitbox_radius = 3.f;
        e._sprite = c.make_sprite(*soul_texture);
        e._sprite.set_origin_to_center();

        e._update_fn = [&](auto& x, auto)
        {
            constexpr float speed{5.f};
            sdl::vec2i input;

            if(c.key(sdl::kkey::left))
                input.x() = -1;
            else if(c.key(sdl::kkey::right))
                input.x() = 1;

            if(c.key(sdl::kkey::up))
                input.y() = -1;
            else if(c.key(sdl::kkey::down))
                input.y() = 1;

            x._pos += input * speed;
        };

        e._draw_fn = [&](auto& x)
        {
            x._sprite.pos() = x._pos;
            c.draw(x._sprite);
        };

        return e;
    };

    auto make_fireball = [&](auto pos, auto vel, auto speed)
    {
        entity e;
        e._pos = pos;
        e._hitbox_radius = 3.f;
        e._sprite = c.make_sprite(*fireball_texture);
        e._sprite.set_origin_to_center();

        e._update_fn = [&, vel, speed, life = 100.f ](auto& x, auto) mutable
        {
            x._pos += vel * speed;

            if(life-- <= 0.f) x.alive = false;
        };

        e._draw_fn = [&](auto& x)
        {
            x._sprite.pos() = x._pos;
            c.draw(x._sprite);
        };

        return e;
    };

    auto make_toriel = [&](auto pos)
    {
        entity e;
        e._pos = pos;
        e._hitbox_radius = 30.f;
        e._sprite = c.make_sprite(*toriel_texture);
        e._sprite.set_origin_to_center();

        e._update_fn = [&](auto& x, auto)
        {
            if((rand() % 100) > 30)
            {
                for(int i = 0; i < 30; ++i)
                    if(entities.size() < max_entities)
                        entities.emplace_back(make_fireball(x._pos,
                            sdl::make_vec2(-2.f + (rand() % 500) / 100.f, 2.f),
                            1.f + (rand() % 100) / 80.f));
            }
        };

        e._draw_fn = [&](auto& x)
        {
            x._sprite.pos() = x._pos;
            c.draw(x._sprite);
        };

        return e;
    };

    entities.emplace_back(make_toriel(sdl::make_vec2(500.f, 100.f)));
    entities.emplace_back(make_soul(sdl::make_vec2(500.f, 500.f)));

    c.update_fn() = [&](auto ft)
    {

        for(auto& e : entities) e._update_fn(e, ft);
        entities.erase(std::remove_if(std::begin(entities), std::end(entities),
                           [](auto& e)
                           {
                               return !e.alive;
                           }),
            std::end(entities));


        if(c.key(sdl::kkey::escape)) sdl::stop_global_context();

        // if(rand() % 100 < 20)
        //    std::cout << "(" << c.fps() << ") " << entities.size() << "\n";
    };

    c.draw_fn() = [&]
    {
        program.use();
        GLfloat vVertices[] = {
            0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f};

        // Set the viewport
        glViewport(0, 0, 1000, 600);

        // Clear the color buffer
        glClear(
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        // Use the program object

        // Load the vertex data
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        // for(auto& e : entities) e._draw_fn(e);

    };

    sdl::run_global_context();

    return 0;
}