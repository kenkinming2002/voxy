#pragma once

#include <camera.hpp>
#include <entity.hpp>
#include <dimension.hpp>

#include <graphics/texture_array.hpp>
#include <graphics/mesh.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <optional>

#include <cstddef>

struct TerrainLayerConfig
{
  uint32_t block_id;

  float    base;
  float    frequency;
  float    amplitude;
  float    lacunarity;
  float    persistence;
  unsigned octaves;
};

struct TerrainConfig
{
  std::vector<TerrainLayerConfig> layers;
};

struct CavesConfig
{
  unsigned max_per_chunk;
  unsigned max_segment;
  float    step;

  float min_height;
  float max_height;

  float dig_frequency;
  float dig_amplitude;
  float dig_lacunarity;
  float dig_persistence;
  float dig_octaves;

  float radius;
  float radius_frequency;
  float radius_amplitude;
  float radius_lacunarity;
  float radius_persistence;
  float radius_octaves;
};

struct WorldConfig
{
  std::size_t   seed;
  TerrainConfig terrain;
  CavesConfig   caves;
};

struct World
{
public:
  WorldConfig config;

  Camera    camera;
  Entity    player;
  Dimension dimension;

  std::optional<glm::ivec3> placement;
  std::optional<glm::ivec3> selection;

public:
  Block* get_block(glm::ivec3 position);
  const Block* get_block(glm::ivec3 position) const;

  void invalidate_mesh_major(glm::ivec3 position);
  void invalidate_mesh_minor(glm::ivec3 position);

  void invalidate_light(glm::ivec3 position);
};

