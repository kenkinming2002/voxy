#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <chunk_defs.hpp>
#include <chunk_generator.hpp>
#include <block.hpp>

#include <mesh.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <optional>

struct ChunkData
{
  struct Layer { Block blocks[CHUNK_WIDTH][CHUNK_WIDTH]; };
  std::vector<Layer> layers;
};

struct Chunk
{
public:
  int width()  { assert(data); return CHUNK_WIDTH; }
  int height() { assert(data); return data->layers.size(); }

public:
  std::optional<Block> get_block(glm::ivec3 position) const;
  bool set_block(glm::ivec3 position, Block block);
  void explode(glm::vec3 center, float radius);

public:
  void generate(glm::ivec2 chunk_position, const ChunkGenerator& chunk_generator);
  void remash(const std::vector<BlockData>& block_datas);

public:
  std::unique_ptr<ChunkData> data;
  std::unique_ptr<Mesh>      mesh;
};

#endif // CHUNK_HPP
