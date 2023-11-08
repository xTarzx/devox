#include <climits>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <string>
#include <time.h>
#include <vector>

#define RAYGUI_IMPLEMENTATION

#include "raygui.h"

const int grid_h_count = 10;
const int grid_v_count = 20;

const int grid_size_p = 40;
const int game_width = grid_h_count * grid_size_p;

const int window_height = grid_v_count * grid_size_p;
const int window_width = window_height;

const Color block_color = GetColor(0x888888DD);
const Color ghost_color = GetColor(0x88888822);

struct Block
{
  Vector2 center;

  bool isnull = false;

  Rectangle get_rect()
  {
    Rectangle r;

    r.x = center.x - (float)grid_size_p / 2;
    r.y = center.y - (float)grid_size_p / 2;
    r.width = grid_size_p;
    r.height = grid_size_p;

    return r;
  }
};

struct Tetromino
{
  Vector2 rot_point;

  std::vector<Block> blocks;

  void rotate()
  {
    for (auto &block : blocks)
    {

      // tranlate to origin
      Vector2 pos_trans = Vector2Subtract(block.center, rot_point);

      Vector2 rotated = Vector2Rotate(pos_trans, DEG2RAD * -90);

      block.center = Vector2Add(rotated, rot_point);
    }
  }

  bool can_move_down(std::vector<Block> &other_blocks)
  {

    for (auto &block : blocks)
    {
      if (block.center.y + grid_size_p > window_height)
      {
        return false;
      }

      for (auto &other_block : other_blocks)
      {
        Rectangle next_r = block.get_rect();
        next_r.y += grid_size_p;
        if (CheckCollisionRecs(next_r, other_block.get_rect()))
        {
          return false;
        }
      }
    }

    return true;
  }

  bool can_move_right(std::vector<Block> &other_blocks)
  {

    for (auto &block : blocks)
    {
      for (auto &other_block : other_blocks)
      {
        Rectangle next_r = block.get_rect();
        next_r.x += grid_size_p;
        if (CheckCollisionRecs(next_r, other_block.get_rect()))
        {
          return false;
        }
      }
    }

    return true;
  }

  bool can_move_left(std::vector<Block> &other_blocks)
  {

    for (auto &block : blocks)
    {
      for (auto &other_block : other_blocks)
      {
        Rectangle next_r = block.get_rect();
        next_r.x -= grid_size_p;
        if (CheckCollisionRecs(next_r, other_block.get_rect()))
        {
          return false;
        }
      }
    }

    return true;
  }

  void move_down()
  {
    rot_point.y += grid_size_p;

    for (auto &block : blocks)
    {
      block.center.y += grid_size_p;
    }
  }

  void move_up()
  {
    rot_point.y -= grid_size_p;

    for (auto &block : blocks)
    {
      block.center.y -= grid_size_p;
    }
  }

  void move_left()
  {

    rot_point.x -= grid_size_p;

    for (auto &block : blocks)
    {
      block.center.x -= grid_size_p;
    }
  }
  void move_right()
  {

    rot_point.x += grid_size_p;

    for (auto &block : blocks)
    {
      block.center.x += grid_size_p;
    }
  }

  void add_block_rel(int xc, int yc)
  {
    Block b;
    b.center =
        Vector2{rot_point.x + grid_size_p * xc, rot_point.y + grid_size_p * yc};
    blocks.push_back(b);
  }

  void draw_raylib(Vector2 offset, Color color)
  {
    for (auto &block : blocks)
    {
      Rectangle rect = block.get_rect();
      rect.x += offset.x;
      rect.y += offset.y;

      DrawRectangleRec(rect, color);
    }
  }
};

void game_bound_check(Tetromino *t)
{
  if (t == NULL)
    return;

  int max_right = -1;
  int max_left = INT_MAX;
  int max_down = -1;

  for (auto &block : t->blocks)
  {
    if (block.center.x > max_right)
      max_right = block.center.x;
    if (block.center.x < max_left)
      max_left = block.center.x;
    if (block.center.y > max_down)
      max_down = block.center.y;
  }
  max_right += grid_size_p / 2;
  max_left -= grid_size_p / 2;
  max_down += grid_size_p / 2;

  if (max_right > game_width)
  {
    int diff = max_right - game_width;
    for (int i = 0; i < diff / grid_size_p; ++i)
    {
      t->move_left();
    }
  }

  if (max_left < 0)
  {
    int diff = -1 * max_left;
    for (int i = 0; i < diff / grid_size_p; ++i)
    {
      t->move_right();
    }
  }

  if (max_down > window_height)
  {
    int diff = max_down - window_height;
    for (int i = 0; i < diff / grid_size_p; ++i)
    {
      t->move_up();
    }
  }
}

void draw_grid()
{
  for (int x = 0; x < game_width; x += grid_size_p)
  {
    DrawLine(x, 0, x, window_height, GetColor(0xEEEEEE1F));
  }
  for (int y = 0; y < window_height; y += grid_size_p)
  {
    DrawLine(0, y, game_width, y, GetColor(0xEEEEEE1F));
  }
}

Tetromino gen_I_tetri()
{
  Tetromino t;
  t.rot_point = Vector2{grid_size_p * 4 + ((float)grid_size_p / 2),
                        grid_size_p * -1 + ((float)grid_size_p / 2)};
  t.add_block_rel(0, 0);
  t.add_block_rel(-1, 0);
  t.add_block_rel(1, 0);
  t.add_block_rel(2, 0);

  return t;
}

Tetromino gen_T_tetri()
{
  Tetromino t;
  t.rot_point = Vector2{grid_size_p * 4 + ((float)grid_size_p / 2),
                        grid_size_p * -1 + ((float)grid_size_p / 2)};
  t.add_block_rel(0, 0);
  t.add_block_rel(-1, 0);
  t.add_block_rel(1, 0);
  t.add_block_rel(0, 1);
  return t;
}

Tetromino gen_O_tetri()
{
  Tetromino t;
  t.rot_point = Vector2{grid_size_p * 4 + ((float)grid_size_p / 2),
                        grid_size_p * -1 + ((float)grid_size_p / 2)};
  t.add_block_rel(0, 0);
  t.add_block_rel(1, 0);
  t.add_block_rel(1, 1);
  t.add_block_rel(0, 1);

  t.rot_point.x += (float)grid_size_p / 2;
  t.rot_point.y += (float)grid_size_p / 2;
  return t;
}

Tetromino gen_J_tetri()
{
  Tetromino t;
  t.rot_point = Vector2{grid_size_p * 4 + ((float)grid_size_p / 2),
                        grid_size_p * -1 + ((float)grid_size_p / 2)};

  t.add_block_rel(0, 0);
  t.add_block_rel(0, -1);
  t.add_block_rel(0, 1);
  t.add_block_rel(-1, 1);

  return t;
}

Tetromino gen_L_tetri()
{
  Tetromino t;
  t.rot_point = Vector2{grid_size_p * 4 + ((float)grid_size_p / 2),
                        grid_size_p * -1 + ((float)grid_size_p / 2)};

  t.add_block_rel(0, 0);
  t.add_block_rel(0, -1);
  t.add_block_rel(0, 1);
  t.add_block_rel(1, 1);

  return t;
}

Tetromino gen_S_tetri()
{
  Tetromino t;
  t.rot_point = Vector2{grid_size_p * 4 + ((float)grid_size_p / 2),
                        grid_size_p * -1 + ((float)grid_size_p / 2)};

  t.add_block_rel(0, 0);
  t.add_block_rel(0, -1);
  t.add_block_rel(1, -1);
  t.add_block_rel(-1, 0);
  return t;
}
Tetromino gen_Z_tetri()
{
  Tetromino t;
  t.rot_point = Vector2{grid_size_p * 4 + ((float)grid_size_p / 2),
                        grid_size_p * -1 + ((float)grid_size_p / 2)};

  t.add_block_rel(0, 0);
  t.add_block_rel(0, -1);
  t.add_block_rel(-1, -1);
  t.add_block_rel(1, 0);
  return t;
}

bool colliding(Tetromino &t, std::vector<Block> &other_blocks)
{
  for (auto &block : t.blocks)
  {
    for (auto &other_block : other_blocks)
    {
      if (CheckCollisionRecs(block.get_rect(), other_block.get_rect()))
      {
        return true;
      }
    }
  }

  return false;
}

int check_line_completion(std::vector<Block> &blocks)
{
  Block grid[grid_v_count][grid_h_count];

  // initialize grid with nulled blocks
  for (int row = 0; row < grid_v_count; row++)
  {
    for (int col = 0; col < grid_h_count; col++)
    {
      Block b;
      b.isnull = true;
      grid[row][col] = b;
    }
  }

  // convert block vector to correspondent grid position
  for (auto block : blocks)
  {
    Rectangle r = block.get_rect();
    int x = r.x / grid_size_p;
    int y = r.y / grid_size_p;

    grid[y][x] = block;
  }

  // check if row is filled, true -> move all above rows down

  int completed_count = 0;

  for (int row = 0; row < grid_v_count; ++row)
  {
    bool shoulderase = true;
    for (int col = 0; col < grid_h_count; ++col)
    {
      if (grid[row][col].isnull)
      {
        shoulderase = false;
        break;
      }
    }

    if (shoulderase)
    {
      completed_count += 1;
      for (int j = row; j > 0; --j)
      {
        for (int x = 0; x < grid_h_count; ++x)
        {
          Block b = grid[j - 1][x];
          b.center.y += grid_size_p;
          grid[j][x] = b;
        }
      }

      for (int col = 0; col < grid_h_count; col++)
      {
        Block b;
        b.isnull = true;
        grid[0][col] = b;
      }
    }
  }

  // convert grid to blocks

  blocks.clear();

  for (int row = 0; row < grid_v_count; ++row)
  {
    for (int col = 0; col < grid_h_count; ++col)
    {
      Block b = grid[row][col];

      if (!b.isnull)
      {
        blocks.push_back(b);
      }
    }
  }

  // maybe return how many rows were completed so points can be calculated
  // accordingly

  return completed_count;
}

typedef Tetromino t_func();
typedef t_func *t_func_p;
std::vector<t_func_p> shape_functions{gen_I_tetri, gen_T_tetri, gen_L_tetri,
                                      gen_J_tetri, gen_S_tetri, gen_Z_tetri,
                                      gen_O_tetri};

t_func_p get_random_shape_func()
{
  int i = rand() % shape_functions.size();

  return shape_functions[i];
}

int main(void)
{

  srand(time(0));

  InitWindow(window_width, window_height, "totri");

  auto func = *get_random_shape_func();
  Tetromino t = func();
  func = *get_random_shape_func();
  Tetromino next_t = func();

  int lines_deleted = 0;

  std::vector<Block> other_blocks;

  const float move_down_timer_r = 0.5;
  float move_down_timer = move_down_timer_r;

  bool lost = false;
  bool should_exit = false;

  while (!should_exit && !WindowShouldClose())
  {
    bool fast_fall = false;

    if (!lost)
    {

      float dt = GetFrameTime();

      if (IsKeyPressed(KEY_SPACE))
      {
        t.rotate();
      }

      if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
      {

        if (t.can_move_left(other_blocks))
        {

          t.move_left();
        }
      }

      if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
      {
        if (t.can_move_right(other_blocks))
        {
          t.move_right();
        }
      }

      if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
      {
        fast_fall = true;
      }

      if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
      {
        while (t.can_move_down(other_blocks))
        {
          t.move_down();
        }
        move_down_timer = -1;
      }

      move_down_timer -= dt;

      if (fast_fall)
      {
        move_down_timer -= 3 * dt;
      }

      game_bound_check(&t);

      while (colliding(t, other_blocks))
      {
        t.move_up();
      }

      if (move_down_timer < 0)
      {
        move_down_timer = move_down_timer_r;

        if (t.can_move_down(other_blocks))
        {
          t.move_down();
        }
        else
        {

          for (auto block : t.blocks)
          {
            if (block.center.y < 0)
            {
              lost = true;
            }
            other_blocks.push_back(block);
          }

          if (!lost)
          {

            t = next_t;

            func = *get_random_shape_func();
            next_t = func();

            lines_deleted += check_line_completion(other_blocks);
          }
        }
      }
    }
    else
    {
      // lost
    }
    BeginDrawing();
    ClearBackground(GetColor(0x222222FF));

    DrawRectangle(0, 0, game_width, window_height, GetColor(0x111111FF));

    t.draw_raylib({0, 0}, block_color);

    Tetromino ghost = t;

    while (ghost.can_move_down(other_blocks))
    {
      ghost.move_down();
    }

    ghost.draw_raylib({0, 0}, ghost_color);

    for (auto &block : other_blocks)
    {
      Rectangle rect = block.get_rect();

      DrawRectangleRec(rect, block_color);
    }

    draw_grid();

    // display next block
    next_t.draw_raylib({game_width - grid_size_p, grid_size_p * 5},
                       block_color);

    DrawText(std::to_string(lines_deleted).c_str(), window_width - 64,
             grid_size_p, 32, block_color);

    if (lost)
    {

      std::string msg_text = "lines deleted: ";
      msg_text += std::to_string(lines_deleted) + " ";

      int result =
          GuiMessageBox({(float)window_width / 2 - 100,
                         (float)window_height / 2 - 80, 100 * 2, 80 * 2},
                        "game end", msg_text.c_str(), "close");
      if (result == 1)
      {
        should_exit = true;
      }
    }
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
