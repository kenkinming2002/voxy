#include <physics.hpp>

#include <optional>

static constexpr float FRICTION_AIR      = 0.03f;
static constexpr float FRICTION_GROUNDED = 0.05f;
static constexpr float GRAVITY           = 9.8f;

struct AABB
{
  glm::vec3 position;
  glm::vec3 dimension;
};

struct SweptAABBResult
{
  float t_in;  glm::vec3 normal_in;
  float t_out; glm::vec3 normal_out;
};

static std::optional<SweptAABBResult> swept_aabb(AABB box1, AABB box2, glm::vec3 direction)
{
  if(direction == glm::vec3(0.0f))
    return std::nullopt;

  glm::vec3 ds_in  = box2.position - (box1.position + box1.dimension); // Distance to enter
  glm::vec3 ds_out = (box2.position + box2.dimension) - box1.position; // Distance to leave

  SweptAABBResult result = {
    .t_in  = -std::numeric_limits<float>::infinity(),
    .t_out = +std::numeric_limits<float>::infinity(),
  };
  for(int i=0; i<3; ++i)
  {
    float d_in  = ds_in[i];
    float d_out = ds_out[i];
    if(direction[i] != 0.0f)
    {
      float t1 = d_in  / direction[i];
      float t2 = d_out / direction[i];

      float t_min = std::min(t1, t2);
      float t_max = std::max(t1, t2);

      if(result.t_in < t_min)
      {
        result.t_in         = t_min;
        result.normal_in    = {};
        result.normal_in[i] = direction[i] > 0.0f ? -1.0f : 1.0f;
      }

      if(result.t_out > t_max)
      {
        result.t_out         = t_max;
        result.normal_out    = {};
        result.normal_out[i] = direction[i] > 0.0f ? 1.0f : -1.0f;
      }
    }
    else if(d_in * d_out >= 0.0f)
      return std::nullopt;
  }
  return result;
}

static void entity_update_physics(const Dimension& dimension_data, Entity& entity_data, float dt)
{
  float friction = entity_data.grounded ? FRICTION_GROUNDED : FRICTION_AIR;
  entity_apply_force(entity_data, -friction * entity_data.velocity,        dt);
  entity_apply_force(entity_data, -GRAVITY  * glm::vec3(0.0f, 0.0f, 1.0f), dt);

  glm::vec3 direction = dt * entity_data.velocity;

  glm::vec3 corner1 = entity_data.transform.position                                  ;
  glm::vec3 corner2 = entity_data.transform.position                       + direction;
  glm::vec3 corner3 = entity_data.transform.position + entity_data.bounding_box            ;
  glm::vec3 corner4 = entity_data.transform.position + entity_data.bounding_box + direction;

  glm::ivec3 corner_min = glm::floor(glm::min(glm::min(glm::min(corner1, corner2), corner3), corner4));
  glm::ivec3 corner_max = glm::ceil (glm::max(glm::max(glm::max(corner1, corner2), corner3), corner4));

  struct Item
  {
    glm::ivec3 position;
    float      distance;
  };
  std::vector<Item> items;

  // We store all voxel that may be in our path and sort them by distance.
  // The idea comes from the video - Arbitrary Rectangle Collision Detection &
  // Resolution - Complete! by javidx9 at
  // https://www.youtube.com/watch?v=8JJ-4JgR7Dg.
  for(int z = corner_min.z; z<=corner_max.z; ++z)
    for(int y = corner_min.y; y<=corner_max.y; ++y)
      for(int x = corner_min.x; x<=corner_max.x; ++x)
        items.push_back(Item{
          .position = glm::ivec3(x, y, z),
          .distance = glm::length(glm::vec3(x, y, z) - entity_data.transform.position),
        });

  std::sort(items.begin(), items.end(), [](const Item& lhs, const Item& rhs) { return lhs.distance < rhs.distance; });

  for(const Item& item : items)
  {
    if(const Block* block = get_block(dimension_data, item.position); block && block->id != BLOCK_ID_NONE)
    {
      AABB entity_aabb = { .position = entity_data.transform.position, .dimension = entity_data.bounding_box, };
      AABB block_aabb  = { .position = item.position,             .dimension = glm::vec3(1.0f),     };
      if(std::optional<SweptAABBResult> result = swept_aabb(entity_aabb, block_aabb, direction))
        if(0.0f <= result->t_in && result->t_in <= 1.0f)
        {
          direction       -= glm::dot(direction,       result->normal_in) * result->normal_in * (1.0f - result->t_in);
          entity_data.velocity -= glm::dot(entity_data.velocity, result->normal_in) * result->normal_in;

          entity_data.collided = true;
          if(result->normal_in.z > 0.0f)
            entity_data.grounded = true;
        }
    }
  }

  entity_data.transform.position += direction;
}

void update_physics(World& world, float dt)
{
  entity_update_physics(world.dimension, world.player.entity, dt);
}