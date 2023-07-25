#include <blocks.hpp>

#include <perlin.hpp>

static constexpr size_t STONE_SEED = 0b1101101011011010101011100000011101001010101010000010111101000110;
static constexpr size_t GRASS_SEED = 0b0101110111011101010110111101101010101010101010001010111100010100;
static constexpr size_t CAVE_SEED  = 0b1101111110111100001110100000110110101010111010000101010101010011;

Blocks::Blocks(glm::ivec2 cpos)
{
  int stone_heights[Blocks::WIDTH][Blocks::WIDTH];
  for(int cy=0; cy<Blocks::WIDTH; ++cy)
    for(int cx=0; cx<Blocks::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Blocks::WIDTH * cpos) + glm::vec2(cx, cy);
      stone_heights[cy][cx] = 0.0f;
      stone_heights[cy][cx] += perlin(STONE_SEED, pos, 1.0f   / Blocks::WIDTH, 5.0f);
      stone_heights[cy][cx] += perlin(STONE_SEED, pos, 0.5f   / Blocks::WIDTH, 10.0f);
      stone_heights[cy][cx] += perlin(STONE_SEED, pos, 0.25f  / Blocks::WIDTH, 20.0f);
      stone_heights[cy][cx] += perlin(STONE_SEED, pos, 0.125f / Blocks::WIDTH, 40.0f);
    }

  int grass_heights[Blocks::WIDTH][Blocks::WIDTH];
  for(int cy=0; cy<Blocks::WIDTH; ++cy)
    for(int cx=0; cx<Blocks::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Blocks::WIDTH * cpos) + glm::vec2(cx, cy);
      grass_heights[cy][cx] = 0.0f;
      grass_heights[cy][cx] += perlin(GRASS_SEED, pos, 0.25f / Blocks::WIDTH, 1.25f);
      grass_heights[cy][cx] += perlin(GRASS_SEED, pos, 0.5f  / Blocks::WIDTH, 2.5f);
      grass_heights[cy][cx] += perlin(GRASS_SEED, pos, 1.0f  / Blocks::WIDTH, 5.0f);
    }

  int max_height = 0;
  for(int cy=0; cy<Blocks::WIDTH; ++cy)
    for(int cx=0; cx<Blocks::WIDTH; ++cx)
      if(max_height < stone_heights[cy][cx]+grass_heights[cy][cx])
        max_height = stone_heights[cy][cx]+grass_heights[cy][cx];

  for(int cz=0; cz<max_height; ++cz)
  {
    Blocks::Layer layer;
    for(int cy=0; cy<Blocks::WIDTH; ++cy)
      for(int cx=0; cx<Blocks::WIDTH; ++cx)
        if(cz <= stone_heights[cy][cx])
          layer.blocks[cy][cx] = Block {
            .presence = true,
            .color    = glm::vec3(0.7, 0.7, 0.7),
          };
        else if(cz <= stone_heights[cy][cx] + grass_heights[cy][cx])
          layer.blocks[cy][cx] = Block {
            .presence = true,
            .color    = glm::vec3(0.2, 1.0, 0.2),
          };
        else
          layer.blocks[cy][cx] = Block {
            .presence = false,
          };

    m_layers.push_back(layer);
  }

  // Hopefully carve some caves
  for(int lz=0; lz<height(); ++lz)
    for(int ly=0; ly<width(); ++ly)
      for(int lx=0; lx<width(); ++lx)
      {
        glm::ivec3 lpos  = { lx, ly, lz };
        glm::ivec3 gpos  = glm::ivec3(cpos.x * Blocks::WIDTH, cpos.y * Blocks::WIDTH, 0.0f) + lpos;

        float noise = 0.0f;
        noise += perlin(CAVE_SEED, glm::vec3(gpos), 16.0 / Blocks::WIDTH, 0.03125f);
        noise += perlin(CAVE_SEED, glm::vec3(gpos), 8.0  / Blocks::WIDTH, 0.0625f);
        noise += perlin(CAVE_SEED, glm::vec3(gpos), 4.0  / Blocks::WIDTH, 0.125f);
        noise += perlin(CAVE_SEED, glm::vec3(gpos), 2.0  / Blocks::WIDTH, 0.25f);
        noise += perlin(CAVE_SEED, glm::vec3(gpos), 1.0  / Blocks::WIDTH, 0.5f);
        if(noise<=0.45)
          set(lpos, Block{ .presence = false });
      }
}

