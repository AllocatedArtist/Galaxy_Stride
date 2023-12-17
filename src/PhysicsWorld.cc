#include "PhysicsWorld.h"

PhysicsWorld::PhysicsWorld() {
  config_ = std::make_unique<btDefaultCollisionConfiguration>();
  dispatcher_ = std::make_unique<btCollisionDispatcher>(config_.get());
  overlapping_pair_cache_ = std::make_unique<btDbvtBroadphase>();
  solver_ = std::make_unique<btSequentialImpulseConstraintSolver>();
  world_ = std::make_unique<btDiscreteDynamicsWorld>(
    dispatcher_.get(), 
    overlapping_pair_cache_.get(),
    solver_.get(),
    config_.get()
  );

  world_->setGravity(btVector3(0.0, -9.8, 0.0));
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


Vector3 RigidBody::GetPosition() {
  btVector3 pos = rigid_body_->getWorldTransform().getOrigin();
  return Vector3 {
    pos.getX(),
    pos.getY(),
    pos.getZ()
  };
}

Quaternion RigidBody::GetRotation() {
  btQuaternion rot = rigid_body_->getWorldTransform().getRotation();
  return Quaternion {
    rot.getX(),
    rot.getY(),
    rot.getZ(),
    rot.getW()
  };
}


