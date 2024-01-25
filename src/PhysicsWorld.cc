#include "PhysicsWorld.h"

PhysicsWorld::PhysicsWorld() {
  config_ = std::make_unique<btDefaultCollisionConfiguration>();
  dispatcher_ = std::make_unique<btCollisionDispatcher>(config_.get());

  ghost_pair_callback_ = std::make_unique<btGhostPairCallback>();

  overlapping_pair_cache_ = std::make_unique<btDbvtBroadphase>();
  overlapping_pair_cache_
    ->getOverlappingPairCache()
    ->setInternalGhostPairCallback(ghost_pair_callback_.get());

  solver_ = std::make_unique<btSequentialImpulseConstraintSolver>();
  world_ = std::make_unique<btDiscreteDynamicsWorld>(
    dispatcher_.get(), 
    overlapping_pair_cache_.get(),
    solver_.get(),
    config_.get()
  );


  world_->setGravity(btVector3(0.0, -9.8, 0.0));

  gContactAddedCallback = OnContactAdded;
}

void PhysicsWorld::Update(float timestep) {
  world_->stepSimulation(timestep, 10);
}

RigidBody PhysicsWorld::CreateRigidBody(
  Vector3 position, 
  std::unique_ptr<btCollisionShape>& shape,
  Quaternion rotation, 
  float mass
) {
  btTransform transform;
  transform.setIdentity();
  transform.setOrigin(btVector3(position.x, position.y, position.z));
  transform.setRotation(btQuaternion(
    rotation.x, 
    rotation.y, 
    rotation.z, 
    rotation.w)
  );

  RigidBody final_rigid_body;
 
  btVector3 inertia;
  inertia.setZero();

  if (mass != 0.0) {
    shape->calculateLocalInertia(mass, inertia);
  }

  final_rigid_body.motion_state_ = 
    std::make_unique<btDefaultMotionState>(transform);
  
  btRigidBody::btRigidBodyConstructionInfo info = 
    btRigidBody::btRigidBodyConstructionInfo(
      mass, 
      final_rigid_body.motion_state_.get(), 
      shape.get(), 
      inertia
    );

  final_rigid_body.rigid_body_ = std::make_unique<btRigidBody>(info);
  world_->addRigidBody(final_rigid_body.rigid_body_.get());

  return final_rigid_body; 
}

CharacterController PhysicsWorld::CreateController(
  float radius, 
  float height,
  float step_height,
  Vector3 position
) {
  CharacterController controller;
  controller.convex_ = std::make_unique<btCapsuleShape>(radius, height);

  controller.ghost_object_ = std::make_unique<btPairCachingGhostObject>();
 
  controller.ghost_object_->setCollisionShape(controller.convex_.get());
  controller.ghost_object_->setCollisionFlags(
    btCollisionObject::CollisionFlags::CF_CHARACTER_OBJECT
  );

  controller.controller_ = std::make_unique<btKinematicCharacterController>(
    controller.ghost_object_.get(),
    controller.convex_.get(),
    step_height
  );

  controller.controller_->setGravity(world_->getGravity());
  
  world_->addCollisionObject(
    controller.ghost_object_.get(), 
    btBroadphaseProxy::CharacterFilter, 
    btBroadphaseProxy::AllFilter
  );

  world_->addAction(controller.controller_.get());

  btTransform transform;
  transform.setIdentity();  
  transform.setOrigin(btVector3(position.x, position.y, position.z));
  controller.ghost_object_->setWorldTransform(transform);
 
  return controller;
}

void PhysicsWorld::ReleaseController(CharacterController* controller) {
  world_->removeCollisionObject(controller->ghost_object_.get());
  world_->removeAction(controller->controller_.get());
  controller->controller_.reset();
  controller->convex_.reset();
  controller->ghost_object_.reset();
}

std::unique_ptr<btCollisionShape> PhysicsWorld::CreateBoxShape(Vector3 size) {
  std::unique_ptr<btCollisionShape> shape = std::make_unique<btBoxShape>(
    btVector3(
      size.x / 2.0, 
      size.y / 2.0, 
      size.z / 2.0
    )
  );
  return shape;
}

std::unique_ptr<btCollisionShape> PhysicsWorld::CreateSphereShape(
  float radius
) {
  std::unique_ptr<btCollisionShape> shape = 
    std::make_unique<btSphereShape>(radius);
  return shape;
}

void PhysicsWorld::ReleaseBody(RigidBody* body) {
  world_->removeRigidBody(body->rigid_body_.get());

  body->rigid_body_.reset();
  body->motion_state_.reset();
}

void PhysicsWorld::SetGravity(Vector3 gravity) {
  world_->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
}

bool OnContactAdded(
  btManifoldPoint& cp, 
  const btCollisionObjectWrapper* colObj0Wrap, 
  int partId0, 
  int index0, 
  const btCollisionObjectWrapper* colObj1Wrap, 
  int partId1, 
  int index1
) {
  const btCollisionObject* obj1 = colObj0Wrap->getCollisionObject();
  const btCollisionObject* obj2 = colObj1Wrap->getCollisionObject();

  if (obj1->getUserIndex() == PhysicsLayer::kCoinLayer) {
    if (obj2->getUserIndex() == PhysicsLayer::kPlayerLayer) {
      LevelCoin* coin = (LevelCoin*)obj1->getUserPointer();
      coin->collected_ = true;
    }
  } else if (obj1->getUserIndex() == PhysicsLayer::kFlagLayer) {
    if (obj2->getUserIndex() == PhysicsLayer::kPlayerLayer) {
      Flag* flag = (Flag*)obj1->getUserPointer();
      flag->is_touched_ = true;
    }
  }

  return false;
}


namespace conv {

const Vector3 GetVec3(btVector3 vec) {
  return Vector3 {
    vec.getX(),
    vec.getY(),
    vec.getZ()
  };
}

const Quaternion GetQuat(btQuaternion quat) {
  return Quaternion {
    quat.getX(),
    quat.getY(),
    quat.getZ(),
    quat.getW()
  };
}

const Vector3 PosFromBody(const RigidBody& body) {
  return GetVec3(body.rigid_body_->getWorldTransform().getOrigin());
}

const Quaternion RotFromBody(const RigidBody& body) {
  return GetQuat(body.rigid_body_->getWorldTransform().getRotation());
}

const Vector3 PosFromController(const CharacterController& controller) {
  return GetVec3(controller.ghost_object_->getWorldTransform().getOrigin());
}

const Quaternion RotFromController(const CharacterController& controller) {
  return GetQuat(controller.ghost_object_->getWorldTransform().getRotation());
}

}
