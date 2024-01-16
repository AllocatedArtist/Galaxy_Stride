#ifndef GAME_H_
#define GAME_H_

#include "PhysicsWorld.h"
#include "LevelEditor.h"
#include "PlayerMovement.h"
#include "FlyCamera.h"

class Game {
public:
  Game(LevelEditor& editor);

  void SetLevels(const std::vector<std::string>& levels);

  Camera GetCamera();

  void Setup(LevelEditor& editor);
  void Unload();

  void Update(const LevelEditor& editor);

  void DrawUI();

  Flag& GetFlag();
  std::vector<LevelMesh>& GetMeshes();
  std::vector<LevelCoin>& GetCoins();

  std::string NextLevel();

  const int GetScore() const;
  const bool IsGameOver() const;

  ~Game();
private:
  bool loaded_ = false;

  std::vector<std::string> level_filenames_;

  Sound coin_pickup_sfx_;

  FlyCamera camera_; 
   
  PhysicsWorld physics_;

  std::vector<RigidBody> mesh_bodies_;
  std::vector<std::unique_ptr<btCollisionShape>> mesh_colliders_;

  std::vector<RigidBody> coin_bodies_;
  std::vector<std::unique_ptr<btCollisionShape>> coin_colliders_;

  RigidBody flag_body_;
  std::unique_ptr<btCollisionShape> flag_shape_;

  CharacterController player_;
  PlayerMovement player_movement_;

  std::vector<LevelMesh> meshes_;
  std::vector<LevelCoin> coins_;

  Flag flag_; 

  int current_score_;
  int previous_score_;
};


#endif
