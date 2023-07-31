#include <world.hpp>

#include <chunk_coords.hpp>

#include <glm/gtx/norm.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

/*************
 * Constants *
 *************/
static constexpr float FRICTION = 0.5f;
static constexpr float GRAVITY  = 5.0f;
static constexpr int MAX_COLLISION_ITERATION = 5;

static constexpr float ROTATION_SPEED = 0.1f;

static constexpr glm::vec2   DEBUG_MARGIN       = glm::vec2(3.0f, 3.0f);
static constexpr const char *DEBUG_FONT        = "assets/arial.ttf";
static constexpr float       DEBUG_FONT_HEIGHT = 20.0f;

/**********
 * Entity *
 **********/
static void entity_apply_force(Entity& entity, glm::vec3 force, float dt)
{
  entity.velocity += dt * force;
}

static void entity_update_physics(Entity& entity, float dt)
{
  entity_apply_force(entity, -FRICTION * entity.velocity,             dt);
  entity_apply_force(entity, -GRAVITY  * glm::vec3(0.0f, 0.0f, 1.0f), dt);
  entity.transform.position += dt * entity.velocity;
}

static glm::vec3 aabb_collide(glm::vec3 position1, glm::vec3 dimension1, glm::vec3 position2, glm::vec3 dimension2)
{
  glm::vec3 point1 = position2 - (position1 + dimension1);
  glm::vec3 point2 = (position2 + dimension2) - position1;
  float x = std::abs(point1.x) < std::abs(point2.x) ? point1.x : point2.x;
  float y = std::abs(point1.y) < std::abs(point2.y) ? point1.y : point2.y;
  float z = std::abs(point1.z) < std::abs(point2.z) ? point1.z : point2.z;
  return glm::vec3(x, y, z);
}

static std::vector<glm::vec3> entity_collide(const Entity& entity, const Dimension& dimension)
{
  std::vector<glm::vec3> collisions;

  glm::ivec3 corner1 = glm::floor(entity.transform.position);
  glm::ivec3 corner2 = -glm::floor(-(entity.transform.position + entity.bounding_box))-1.0f;
  for(int z = corner1.z; z<=corner2.z; ++z)
    for(int y = corner1.y; y<=corner2.y; ++y)
      for(int x = corner1.x; x<=corner2.x; ++x)
      {
        glm::ivec3 position = glm::ivec3(x, y, z);
        Block block = dimension.get_block(position).value_or(Block{.presence = false});
        if(block.presence)
        {
          glm::vec3 collision = aabb_collide(entity.transform.position, entity.bounding_box, position, glm::vec3(1.0f, 1.0f, 1.0f));
          spdlog::info("Entity collision {}, {}, {} with block {}, {}, {}",
            collision.x, collision.y, collision.z,
            position.x, position.y, position.z
          );
          collisions.push_back(collision);
        }
      }

  spdlog::info("Entity colliding = {}", !collisions.empty());
  return collisions;
}

void entity_resolve_collisions(Entity& entity, const Dimension& dimension)
{
  glm::vec3 original_position = entity.transform.position;
  glm::vec3 original_velocity = entity.velocity;

  for(int i=0; i<MAX_COLLISION_ITERATION; ++i)
  {
    std::vector<glm::vec3> collisions = entity_collide(entity, dimension);
    if(collisions.empty())
      return;

    float                    min = std::numeric_limits<float>::infinity();
    std::optional<glm::vec3> resolution;

    for(glm::vec3 collision : collisions)
      for(glm::vec3 direction : DIRECTIONS)
        if(float length = glm::dot(collision, direction); 0.0f < length && length < min)
        {
          min        = length;
          resolution = length * direction;
        }

    if(resolution)
    {
      spdlog::info("Resolving collision by {}, {}, {}", resolution->x, resolution->y, resolution->z);
      entity.transform.position += *resolution;
      if(resolution->x != 0.0f) entity.velocity.x = 0.0f;
      if(resolution->y != 0.0f) entity.velocity.y = 0.0f;
      if(resolution->z != 0.0f) entity.velocity.z = 0.0f;
    }

  }
  spdlog::warn("Failed to resolve collision");
  entity.transform.position = original_position;
  entity.velocity = original_velocity;
}

/*********
 * World *
 *********/
World::World(std::size_t seed) :
  m_camera{
    .aspect = 1024.0f / 720.0f,
    .fovy   = 45.0f,
  },
  m_player{
    .transform = {
      .position = glm::vec3(0.0f, 0.0f, 50.0f),
      .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
    },
    .velocity     = glm::vec3(0.0f, 0.0f, 0.0f),
    .bounding_box = glm::vec3(0.9f, 0.9f, 1.9f),
  },
  m_dimension(seed),
  m_text_renderer(DEBUG_FONT, DEBUG_FONT_HEIGHT),
  m_chunk_generator_system(ChunkGeneratorSystem::create(seed)),
  m_chunk_mesher_system(ChunkMesherSystem::create()),
  m_light_system(LightSystem::create())
{}

void World::handle_event(SDL_Event event)
{
  switch(event.type)
  {
    case SDL_MOUSEMOTION:
      m_player.transform = m_player.transform.rotate(glm::vec3(0.0f,
        -event.motion.yrel * ROTATION_SPEED,
        -event.motion.xrel * ROTATION_SPEED
      ));
      break;
    case SDL_MOUSEWHEEL:
      m_camera.zoom(-event.wheel.y);
      break;
  }
}

void World::update(float dt)
{
  // 1: Camera Movement
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
    translation = m_player.transform.gocal_to_global(translation);
    translation = glm::normalize(translation);
    translation *= dt;
    m_player.velocity += translation * 10.0f;
  }

  // 2: Lazy chunk loading
  glm::ivec2 center = {
    std::floor(m_player.transform.position.x / CHUNK_WIDTH),
    std::floor(m_player.transform.position.y / CHUNK_WIDTH),
  };
  load(center, CHUNK_LOAD_RADIUS);

  // 3: Update
  m_light_system->update(*this);
  for(auto& [chunk_index, chunk] : m_dimension.chunks())
    if(chunk.data)
      m_chunk_mesher_system->update_chunk(*this, chunk_index);

  // 4: Entity Update
  entity_update_physics(m_player, dt);
  entity_resolve_collisions(m_player, m_dimension);

  // 5: Camera update
  m_camera.transform           = m_player.transform;
  m_camera.transform.position += glm::vec3(0.5f, 0.5f, 1.5f);
}

void World::render()
{
  m_dimension.render(m_camera);

  std::string line;
  glm::vec2   cursor = DEBUG_MARGIN;

  line = fmt::format("position: x = {}, y = {}, z = {}", m_player.transform.position.x, m_player.transform.position.y, m_player.transform.position.z);
  m_text_renderer.render(cursor, line.c_str());
  cursor.x = DEBUG_MARGIN.x;
  cursor.y += DEBUG_FONT_HEIGHT;

  line = fmt::format("velocity: x = {}, y = {}, z = {}", m_player.velocity.x, m_player.velocity.y, m_player.velocity.z);
  m_text_renderer.render(cursor, line.c_str());
  cursor.x = DEBUG_MARGIN.x;
  cursor.y += DEBUG_FONT_HEIGHT;
}

void World::load(glm::ivec2 chunk_index)
{
  if(!m_dimension.chunks()[chunk_index].data)
  {
    if(!m_chunk_generator_system->try_generate_chunk(*this, chunk_index))
      return;

    for(int lz=0; lz<CHUNK_HEIGHT; ++lz)
      for(int ly=0; ly<CHUNK_WIDTH; ++ly)
        for(int lx=0; lx<CHUNK_WIDTH; ++lx)
        {
          glm::ivec3 position = { lx, ly, lz };
          m_dimension.lighting_invalidate(local_to_global(position, chunk_index));
        }
  }

  if(!m_dimension.chunks()[chunk_index].mesh)
    m_chunk_mesher_system->remesh_chunk(*this, chunk_index);
}

void World::load(glm::ivec2 center, int radius)
{
  for(int cy = center.y - radius; cy <= center.y + radius; ++cy)
    for(int cx = center.x - radius; cx <= center.x + radius; ++cx)
    {
      glm::ivec2 chunk_index(cx, cy);
      load(chunk_index);
    }
}


