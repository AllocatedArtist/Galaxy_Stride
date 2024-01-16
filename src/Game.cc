#include "Game.h"
#include <algorithm>
#include <iostream>

Game::Game(LevelEditor& editor) {
  BoundingBox flag_bounds = editor
    .GetAsset(kFlagModelIndex)
    .model_
    .GetBoundingBox();

  Vector3 flag_bound_size = Vector3Subtract(flag_bounds.max, flag_bounds.min);
  
  flag_shape_ = physics_.CreateBoxShape(flag_bound_size);
  camera_ = FlyCamera({ 0.0, 2.0, -5.0 }, 0.1, 5.0);

  coin_pickup_sfx_ = LoadSound("assets/sounds/coin.wav");
  loaded_ = false;
}

void Game::Setup(LevelEditor& editor) {
  player_movement_.ResetStamina();
  DisableCursor();

  for (const LevelMesh& mesh : meshes_) {
    ModelComponent& model = editor.GetAsset(mesh.index_).model_;
    BoundingBox bounds = model.GetBoundingBox();

    Vector3 size = Vector3Subtract(bounds.max, bounds.min);
    mesh_colliders_.emplace_back(physics_.CreateBoxShape(size));

    std::unique_ptr<btCollisionShape>& box = mesh_colliders_.back();
        
    mesh_bodies_.emplace_back(physics_.CreateRigidBody(
      mesh.pos_,
      box,
      mesh.rotation_,
      0.f
    ));
  }

  for (LevelCoin& coin : coins_) {
    ModelComponent& model = editor.GetAsset(coin.index_).model_;
    BoundingBox bounds = model.GetBoundingBox();

    Vector3 size = Vector3Subtract(bounds.max, bounds.min);
    coin_colliders_.emplace_back(physics_.CreateBoxShape(size));

    std::unique_ptr<btCollisionShape>& box = coin_colliders_.back();
        
    coin_bodies_.emplace_back(physics_.CreateRigidBody(
      coin.pos_,
      box,
      coin.rotation_,
      0.f
    ));

    RigidBody& body = coin_bodies_.back();
    body.rigid_body_->setCollisionFlags(
      btCollisionObject::CF_NO_CONTACT_RESPONSE |
      btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK
    );
    body.rigid_body_->setUserIndex(PhysicsLayer::kCoinLayer);
    body.rigid_body_->setUserPointer(&coins_[coin_bodies_.size() - 1]);
  }

  flag_body_ = physics_.CreateRigidBody(
    flag_.flag_position_, 
    flag_shape_, 
    flag_.flag_rotation_, 
    0.f
  );

  flag_body_.rigid_body_->setCollisionFlags(
    btCollisionObject::CF_NO_CONTACT_RESPONSE |
    btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK
  );
  flag_body_.rigid_body_->setUserIndex(PhysicsLayer::kFlagLayer);
  flag_body_.rigid_body_->setUserPointer(&flag_);

  player_ = physics_.CreateController(
    0.25, 
    1.5,
    0.1, 
    editor.GetPlayerPosition()
  );

  player_.ghost_object_->setCollisionFlags(
    player_.ghost_object_->getCollisionFlags() |
    btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK
  );

  player_.ghost_object_->setUserIndex(PhysicsLayer::kPlayerLayer);

  camera_.GetCamera().SetPitch(0.f);
  camera_.GetCamera().SetYaw(editor.GetPlayerYaw());
  loaded_ = true;
}

void Game::Unload() {
  mesh_colliders_.clear();
  coin_colliders_.clear();

  for (RigidBody& body : mesh_bodies_) {
    physics_.ReleaseBody(&body);
  }

  for (RigidBody& body : coin_bodies_) {
    physics_.ReleaseBody(&body);
  }

  if (flag_body_.rigid_body_ != nullptr) {
    physics_.ReleaseBody(&flag_body_);
  }

  for (LevelCoin& coin : coins_) {
    coin.collected_ = false;
  }

  previous_score_ = 0;
  flag_.is_touched_ = false;

  mesh_bodies_.clear();
  coin_bodies_.clear();

  if (player_.ghost_object_ != nullptr) {
    physics_.ReleaseController(&player_);
  }

  EnableCursor();

  loaded_ = false;
}

void Game::Update(const LevelEditor& editor) {
  if (!loaded_) {
    return;
  }
  physics_.Update(1.0 / 60.0);
  camera_.LookAround();
  player_movement_.Update(player_, camera_);

  if (camera_.GetCamera().GetPosition().y <= -10.f) {
    player_movement_.ResetStamina();

    btTransform transform;
    transform.setIdentity();

    Vector3 player_pos = editor.GetPlayerPosition();
    transform.setOrigin(btVector3(player_pos.x, player_pos.y, player_pos.z));

    player_.ghost_object_->setWorldTransform(transform);

    camera_.GetCamera().SetPitch(0.f);
    camera_.GetCamera().SetYaw(editor.GetPlayerYaw());

    previous_score_ = 0;

    for (LevelCoin& coin : coins_) {
      coin.collected_ = false;
    }
  }   

  current_score_ = std::count_if(
    coins_.cbegin(), 
    coins_.cend(), 
    [](const LevelCoin& coin){
      return coin.collected_;
    }
  );

  if (previous_score_ != current_score_) {
    previous_score_ = current_score_;
    PlaySound(coin_pickup_sfx_);
  }
}

void Game::DrawUI() {
  DrawStamina(player_movement_);
  DrawText(TextFormat("SCORE: %d", current_score_), 20, 90, 32, WHITE);
}

Camera Game::GetCamera() {
  return camera_.GetCamera().GetCamera();
}

Game::~Game() {
  UnloadSound(coin_pickup_sfx_);

  mesh_colliders_.clear();
  coin_colliders_.clear();

  flag_shape_.reset();

  for (RigidBody& body : mesh_bodies_) {
    physics_.ReleaseBody(&body);
  }

  for (RigidBody& body : coin_bodies_) {
    physics_.ReleaseBody(&body);
  }

  if (flag_body_.rigid_body_ != nullptr) {
    physics_.ReleaseBody(&flag_body_);
  }

  mesh_bodies_.clear();
  coin_bodies_.clear();

  if (player_.controller_ != nullptr) {
    physics_.ReleaseController(&player_);
  }
}

Flag& Game::GetFlag() {
  return flag_;
}

std::vector<LevelMesh>& Game::GetMeshes() {
  return meshes_;
}

std::vector<LevelCoin>& Game::GetCoins() {
  return coins_;
}

std::string Game::NextLevel() {
  assert(!level_filenames_.empty());
  std::string filename = level_filenames_.back();
  level_filenames_.pop_back();
  return filename;
}

void Game::SetLevels(const std::vector<std::string>& levels) {
  level_filenames_ = levels;

  //usually input is { "level_0", "level_1", "level_2" ... }
  //level vector is popped, which is why it is reversed here so that
  //level_0 is the first to be removed
  std::reverse(level_filenames_.begin(), level_filenames_.end());
}

const bool Game::IsGameOver() const {
  return level_filenames_.empty();
}

const int Game::GetScore() const {
  return current_score_;
}
