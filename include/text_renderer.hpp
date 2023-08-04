#ifndef TEXT_RENDERER_HPP
#define TEXT_RENDERER_HPP

#include <shader_program.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <memory>

class TextRenderer
{
public:
  TextRenderer(const char *font, unsigned height);
  ~TextRenderer();

public:
  void render(glm::vec2& cursor, const char *str);

private:
  ShaderProgram m_shader_program;

  struct Glyph
  {
    glm::ivec3 dimenson;
    glm::vec2  bearing;
    glm::vec2  advance;

    GLuint texture;
  };
  Glyph m_glyphs[128];

  GLuint m_vao;
  GLuint m_veo;
  GLuint m_vbo;
};

#endif // TEXT_RENDERER_HPP
