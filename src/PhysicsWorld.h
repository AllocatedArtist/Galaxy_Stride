#ifndef PHYSICS_WORLD_H_
#define PHYSICS_WORLD_H_

#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <raymath.h>

#include <vector>
#include <memory>

struct RigidBody {
  std::unique_ptr<btDefaultMotionState> motion_state_;
  std::unique_ptr<btRigidBody> rigid_body_;
};

struct CharacterController {
  std::unique_ptr<btPairCachingGhostObject> ghost_object_;
  std::unique_ptr<btKinematicCharacterController> controller_;
  std::unique_ptr<btConvexShape> convex_;
};

namespace conv {
  const Vector3 GetVec3(btVector3 vec);
  const Quaternion GetQuat(btQuaternion quat);
  const Vector3 PosFromBody(const RigidBody& body);
  const Quaternion RotFromBody(const RigidBody& body);
  const Vector3 PosFromController(const CharacterController& controller);
  const Quaternion RotFromController(const CharacterController& controller);

}


class PhysicsWorld {
public:
  PhysicsWorld();

  void Update(float timestep);

  void SetGravity(Vector3 gravity);

  std::unique_ptr<btCollisionShape> CreateBoxShape(Vector3 size);
  std::unique_ptr<btCollisionShape> CreateSphereShape(float radius);

  CharacterController CreateController(
    float radius, 
    float height, 
    float step_height,
    Vector3 position
  );
 
  RigidBody CreateRigidBody(
    Vector3 position, 
    std::unique_ptr<btCollisionShape>& shape,
    Quaternion rotation, 
    float mass
  );
 
  void ReleaseBody(RigidBody* body);
  void ReleaseController(CharacterController* controller);
private:
  std::unique_ptr<btDefaultCollisionConfiguration> config_;
  std::unique_ptr<btCollisionDispatcher> dispatcher_;
  std::unique_ptr<btBroadphaseInterface> overlapping_pair_cache_; 
  std::unique_ptr<btGhostPairCallback> ghost_pair_callback_;
  std::unique_ptr<btSequentialImpulseConstraintSolver> solver_;
  std::unique_ptr<btDiscreteDynamicsWorld> world_;
};

#endif
