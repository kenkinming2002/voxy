#include <world.hpp>

#include <perlin.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include <math.h>

World::World() :
  program(gl::compile_program("assets/shader.vert", "assets/shader.frag")) {}

void World::generate_chunk(glm::ivec2 cpos)
{
  if(chunks.contains(cpos))
    return;

  int heights[Layer::WIDTH][Layer::WIDTH];
  for(int cy=0; cy<Layer::WIDTH; ++cy)
    for(int cx=0; cx<Layer::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Layer::WIDTH * cpos) + glm::vec2(cx, cy);

      heights[cy][cx] = 0.0f;
      heights[cy][cx] += perlin(pos, 0.1f  / Layer::WIDTH, 1.0f);
      heights[cy][cx] += perlin(pos, 0.25f / Layer::WIDTH, 5.0f);
      heights[cy][cx] += perlin(pos, 0.5f  / Layer::WIDTH, 10.0f);
      heights[cy][cx] += perlin(pos, 1.0f  / Layer::WIDTH, 20.0f);
    }

  int max_height = 0;
  for(int cy=0; cy<Layer::WIDTH; ++cy)
    for(int cx=0; cx<Layer::WIDTH; ++cx)
      if(max_height < heights[cy][cx])
        max_height = heights[cy][cx];

  Chunk chunk;
  for(int cz=0; cz<max_height; ++cz)
  {
    Layer layer;
    for(int cy=0; cy<Layer::WIDTH; ++cy)
      for(int cx=0; cx<Layer::WIDTH; ++cx)
        if(cz <= heights[cx][cy])
          layer.blocks[cy][cx] = Block {
            .presence = true,
            .color    = glm::vec3(0.0, 1.0, 0.0),
          };
        else
          layer.blocks[cy][cx] = Block {
            .presence = false,
          };

    chunk.layers.push_back(layer);
  }

  chunks.insert({cpos, std::move(chunk)});
}

void World::generate_chunk_mesh(glm::ivec2 cpos)
{
  if(chunk_meshes.contains(cpos))
    return;

  Chunk& chunk = chunks.at(cpos);

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  for(int z=0; z<chunk.layers.size(); ++z)
    for(int y=0; y<Layer::WIDTH; ++y)
      for(int x=0; x<Layer::WIDTH; ++x)
      {
        const Block& block = chunk.layers[z].blocks[y][x];
        if(block.presence)
        {
          static constexpr uint32_t cube_indices[] = {
            0,  1,  2,  2,  1,  3,
            6,  5,  4,  7,  5,  6,
            10,  9,  8, 11, 9,  10,
            12, 13, 14, 14, 13, 15,
            16, 17, 18, 18, 17, 19,
            22, 21, 20, 23, 21, 22,
          };

          static constexpr std::pair<glm::vec3, glm::vec3> cube_vertices[] = {
            {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
            {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
            {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
            {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},

            {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

            {{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
            {{0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
            {{1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
            {{1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},

            {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
            {{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

            {{0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
            {{0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
            {{0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
            {{0.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},

            {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
            {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
          };

          uint32_t index_base = vertices.size();
          for(uint32_t cube_index : cube_indices)
            indices.push_back(index_base + cube_index);

          glm::vec3 position_base = glm::vec3(x, y, z);
          for(auto [cube_position, cube_normal] : cube_vertices)
            vertices.push_back(Vertex{
                .pos    = position_base + cube_position,
                .normal = cube_normal,
                .color  = block.color,
            });
        }
      }

  Mesh mesh(indices, vertices);
  chunk_meshes.insert({cpos, std::move(mesh)});
}

void World::unload(glm::vec2 center, float radius)
{
  glm::ivec2 ccenter = center / (float)Layer::WIDTH;
  int        cradius = radius / Layer::WIDTH;

  for(auto it = chunk_meshes.begin(); it != chunk_meshes.end();)
  {
    const auto& [cpos, chunk_mesh] = *it;
    glm::ivec2 coff = cpos - ccenter;
    if(coff.x * coff.x + coff.y * coff.y > cradius)
      it = chunk_meshes.erase(it);
    else
      ++it;
  }
}

void World::load(glm::vec2 center, float radius)
{
  glm::ivec2 ccenter = center / (float)Layer::WIDTH;
  int        cradius = radius / Layer::WIDTH;

  for(int yoff = -cradius; yoff <= cradius; ++yoff)
    for(int xoff = -cradius; xoff <= cradius; ++xoff)
    {
      glm::ivec2 coff(xoff, yoff);
      if(xoff * xoff + yoff * yoff < cradius)
      {
        glm::ivec2 cpos = ccenter + coff;
        generate_chunk(cpos);
        generate_chunk_mesh(cpos);
      }
    }
}

void World::handle_event(SDL_Event event)
{
  switch(event.type) {
    case SDL_MOUSEMOTION:
      camera.rotate(-event.motion.xrel, -event.motion.yrel);
      break;
    case SDL_MOUSEWHEEL:
      camera.zoom(-event.wheel.y);
      break;
  }
}

void World::update(float dt)
{
  glm::vec3 translation = glm::vec3(0.0f);

  const Uint8 *keys = SDL_GetKeyboardState(nullptr);
  if(keys[SDL_SCANCODE_SPACE])  translation.z += 1.0f;
  if(keys[SDL_SCANCODE_LSHIFT]) translation.z -= 1.0f;
  if(keys[SDL_SCANCODE_W])      translation.y += 1.0f;
  if(keys[SDL_SCANCODE_S])      translation.y -= 1.0f;
  if(keys[SDL_SCANCODE_D])      translation.x += 1.0f;
  if(keys[SDL_SCANCODE_A])      translation.x -= 1.0f;
  if(glm::length(translation) != 0.0f)
  {
    translation = glm::normalize(translation);
    translation *= dt;
    camera.translate(translation.x, translation.y, translation.z);
  }

  unload(camera.position, 300.0f);
  load  (camera.position, 300.0f);
}

void World::render()
{
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(program);

  glUniform3f(glGetUniformLocation(program, "lightColor"), 1.0f,  1.0f, 1.0f);
  glUniform3f(glGetUniformLocation(program, "lightPos"),   0.0f, 0.0f, 50.0f);

  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();

  for(const auto& [cpos, chunk_mesh] : chunk_meshes)
  {
    glm::mat4 model  = glm::translate(glm::mat4(1.0f), glm::vec3( Layer::WIDTH * cpos.x, Layer::WIDTH * cpos.y, 0.0f));
    glm::mat4 normal = glm::transpose(glm::inverse(model));
    glm::mat4 MVP    = projection * view * model;

    glUniformMatrix4fv(glGetUniformLocation(program, "MVP"),    1, GL_FALSE, glm::value_ptr(MVP));
    glUniformMatrix4fv(glGetUniformLocation(program, "model"),  1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(program, "normal"), 1, GL_FALSE, glm::value_ptr(normal));

    chunk_mesh.draw();
  }
}
