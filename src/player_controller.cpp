#include <player_controller.hpp>

#include <directions.hpp>
#include <ray_cast.hpp>

PlayerController::PlayerController()
{
  m_cooldown = 0.0f;
}

static bool aabb_collide(glm::vec3 position1, glm::vec3 dimension1, glm::vec3 position2, glm::vec3 dimension2)
{
  glm::vec3 min = glm::max(position1,            position2);
  glm::vec3 max = glm::min(position1+dimension1, position2+dimension2);
  for(int i=0; i<3; ++i)
    if(min[i] >= max[i])
      return false;
  return true;
}

void PlayerController::update(World& world, LightManager& light_manager, float dt)
{
  Player& player        = world.players.front();
  Entity& player_entity = world.entities.at(player.entity_id);

  // 1: Jump
  if(player.key_space)
    if(player_entity.grounded)
    {
      player_entity.grounded = false;
      entity_apply_impulse(player_entity, JUMP_STRENGTH * glm::vec3(0.0f, 0.0f, 1.0f));
    }

  // 2: Movement
  glm::vec3 translation = glm::vec3(0.0f);
  if(player.key_d) translation += player_entity.transform.local_right();
  if(player.key_a) translation -= player_entity.transform.local_right();
  if(player.key_w) translation += player_entity.transform.local_forward();
  if(player.key_s) translation -= player_entity.transform.local_forward();

  if(glm::vec3 direction = translation; direction.z = 0.0f, glm::length(direction) != 0.0f)
    entity_apply_force(player_entity, MOVEMENT_SPEED * glm::normalize(direction), dt);
  else if(glm::vec3 direction = -player_entity.velocity; direction.z = 0.0f, glm::length(direction) != 0.0f)
    entity_apply_force(player_entity, MOVEMENT_SPEED * glm::normalize(direction), dt, glm::length(direction));

  // 3: Rotation
  player_entity.transform = player_entity.transform.rotate(glm::vec3(0.0f,
    -player.cursor_motion_y * ROTATION_SPEED,
    -player.cursor_motion_x * ROTATION_SPEED
  ));

  // 4: Block placement/destruction
  m_cooldown = std::max(m_cooldown - dt, 0.0f);

  player.selection.reset();
  player.placement.reset();
  ray_cast(player_entity.transform.position + glm::vec3(0.0f, 0.0f, player_entity.eye), player_entity.transform.local_forward(), RAY_CAST_LENGTH, [&](glm::ivec3 block_position) -> bool {
      const Block *block = get_block(world, block_position);
      if(block && block->id != BLOCK_ID_NONE)
        player.selection = block_position;
      else
        player.placement = block_position;
      return block && block->id != BLOCK_ID_NONE;
  });

  // Can only place against a selected block
  if(!player.selection)
    player.placement.reset();

  if(m_cooldown == 0.0f)
    if(player.mouse_button_left)
      if(player.selection)
        if(Block *block = get_block(world, *player.selection))
          if(block->id != BLOCK_ID_NONE)
          {
            if(block->destroy_level != 15)
              ++block->destroy_level;
            else
              block->id = BLOCK_ID_NONE;

            invalidate_mesh(world, *player.selection);
            light_manager.invalidate(*player.selection);
            for(glm::ivec3 direction : DIRECTIONS)
            {
              glm::ivec3 neighbour_position = *player.selection + direction;
              invalidate_mesh(world, neighbour_position);
            }
            m_cooldown = ACTION_COOLDOWN;
          }

  if(m_cooldown == 0.0f)
    if(player.mouse_button_right)
      if(player.placement)
        if(Block *block = get_block(world, *player.placement))
          if(block->id == BLOCK_ID_NONE)
            if(!aabb_collide(player_entity.transform.position, player_entity.dimension, *player.placement, glm::vec3(1.0f, 1.0f, 1.0f))) // Cannot place a block that collide with the player
            {
              block->id = BLOCK_ID_STONE;
              invalidate_mesh(world, *player.placement);
              light_manager.invalidate(*player.placement);
              for(glm::ivec3 direction : DIRECTIONS)
              {
                glm::ivec3 neighbour_position = *player.placement + direction;
                invalidate_mesh(world, neighbour_position);
              }
              m_cooldown = ACTION_COOLDOWN;
            }
}

void PlayerController::render(const graphics::Camera& camera, const World& world, graphics::WireframeRenderer& wireframe_renderer)
{
  const Player& player = world.players.front();
  if(player.selection) wireframe_renderer.render_cube(camera, *player.selection, glm::vec3(1.0f), glm::vec3(0.6f, 0.6f, 0.6f), UI_SELECTION_THICKNESS);
  if(player.placement) wireframe_renderer.render_cube(camera, *player.placement, glm::vec3(1.0f), glm::vec3(0.6f, 0.6f, 0.6f), UI_SELECTION_THICKNESS);
}

