#ifndef PHYSICS_WORLD_H_
#define PHYSICS_WORLD_H_

#include <btBulletDynamicsCommon.h>

#include <raymath.h>

#include <vector>
#include <memory>

class RigidBody {
public:
  std::unique_ptr<btDefaultMotionState> motion_state_;
  std::unique_ptr<btRigidBody> rigid_body_;

  Vector3 GetPosition();
  Quaternion GetRotation();
};


class PhysicsWorld {
public:
  PhysicsWorld();

  void Update(float timestep);

  void SetGravity(Vector3 gravity);

  std::unique_ptr<btCollisionShape> CreateBoxShape(Vector3 size);
  std::unique_ptr<btCollisionShape> CreateSphereShape(float radius);
 
  RigidBody CreateRigidBody(
    Vector3 position, 
    std::unique_ptr<btCollisionShape>& shape,
    Quaternion rotation, 
    float mass
  );
 
  void ReleaseBody(RigidBody* body);
private:
  std::unique_ptr<btDefaultCollisionConfiguration> config_;
  std::unique_ptr<btCollisionDispatcher> dispatcher_;
  std::unique_ptr<btBroadphaseInterface> overlapping_pair_cache_; 
  std::unique_ptr<btSequentialImpulseConstraintSolver> solver_;
  std::unique_ptr<btDiscreteDynamicsWorld> world_;
};

#endif
