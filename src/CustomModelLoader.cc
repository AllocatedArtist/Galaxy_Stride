#include "CustomModelLoader.h"

#include <glad.h>
#include <raylib-physfs.h>
#include <raymath.h>

#include <iostream>

void CustomModel::LoadFromMemory(const char* filename) {
  CustomModel loaded_model {};

  tinygltf::TinyGLTF loader;
  unsigned int data_size = 0;
  unsigned char* data = LoadFileDataFromPhysFS(
    TextFormat("assets/models/%s", filename), &data_size
  );

  tinygltf::Model model;
  std::string error, warning;
  loader.LoadBinaryFromMemory(&model, &error, &warning, data, data_size);

  if (!error.empty()) {
    std::cout << "ERROR: " << error << std::endl;
  }
  if (!warning.empty()) {
    std::cout << warning << std::endl;
  }

  for (const tinygltf::Node& node : model.nodes) {
    ProcessNodes(node, model);
    for (const int& child : node.children) {
      ProcessNodes(model.nodes[child], model);
    }
  }  
}

void CustomModel::ProcessNodes(
  const tinygltf::Node& node, 
  const tinygltf::Model& model
) {
  Matrix transform = MatrixIdentity();

  if (node.translation.size() == 3) {
    transform = MatrixMultiply(
      transform, 
      MatrixTranslate(
        node.translation[0], 
        node.translation[1], 
        node.translation[2]
      )
    );
  }

  if (node.rotation.size() == 4) {
    transform = MatrixMultiply(
      transform, 
      QuaternionToMatrix(
        Quaternion {
          (float)node.rotation[0], 
          (float)node.rotation[1], 
          (float)node.rotation[2],
          (float)node.rotation[3]
        }
      )
    );
  }

  if (node.scale.size() == 3) {
    transform = MatrixMultiply(
      transform, 
      MatrixScale(
        node.scale[0], 
        node.scale[1], 
        node.scale[2]
      )
    );
  }

  ProcessMesh(model.meshes[node.mesh], model, transform);
}

void CustomModel::ProcessMesh(
  const tinygltf::Mesh& mesh, 
  const tinygltf::Model& model,
  const Matrix& transform
) {
  CustomMesh loaded_mesh;
  loaded_mesh.transform_ = transform;

  for (const tinygltf::Primitive& primitive : mesh.primitives) {
    const tinygltf::Material& material = model.materials[primitive.material];

    std::vector<double> base_color { 0.5, 0.0, 0.5 };

    if (!material.pbrMetallicRoughness.baseColorFactor.empty()) {
      base_color = material.pbrMetallicRoughness.baseColorFactor;
    }

    Vector3 base_color_mesh;

    base_color_mesh.x = base_color[0];
    base_color_mesh.y = base_color[1];
    base_color_mesh.z = base_color[2];

    loaded_mesh.base_color_.push_back(base_color_mesh);

    const tinygltf::Accessor& indices_accessor = 
      model.accessors[primitive.indices];

    loaded_mesh.index_count_.push_back(indices_accessor.count);

    const tinygltf::BufferView& indices_view = 
      model.bufferViews[indices_accessor.bufferView];
    const tinygltf::Buffer& indices_buffer = 
      model.buffers[indices_view.buffer];

    std::vector<unsigned char> indices_data(
      indices_buffer.data.cbegin() + indices_view.byteOffset,
      indices_buffer.data.cbegin() + 
      indices_view.byteOffset + 
      indices_view.byteLength
    );

    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
      GL_ELEMENT_ARRAY_BUFFER, 
      indices_data.size(), 
      &indices_data[0], 
      GL_STATIC_DRAW
    );

    for (auto attribute : primitive.attributes) {
      if (attribute.first == "POSITION")  {
        const tinygltf::Accessor& pos_accessor = 
          model.accessors[attribute.second];

        loaded_mesh.mins_.push_back
        (
          Vector3 
          { 
            (float)pos_accessor.minValues[0], 
            (float)pos_accessor.minValues[1], 
            (float)pos_accessor.minValues[2]
          }
        );

      loaded_mesh.maxs_.push_back
        (
          Vector3 
          { 
            (float)pos_accessor.maxValues[0], 
            (float)pos_accessor.maxValues[1], 
            (float)pos_accessor.maxValues[2]
          }
        );


        const tinygltf::BufferView& pos_view = 
          model.bufferViews[pos_accessor.bufferView];

        const tinygltf::Buffer& pos_buffer = 
          model.buffers[pos_view.buffer];


        std::vector<unsigned char> position(
          pos_buffer.data.cbegin() + pos_view.byteOffset,
          pos_buffer.data.cbegin() + 
          pos_view.byteOffset + 
          pos_view.byteLength
        );

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
          GL_ARRAY_BUFFER,
          position.size(),
          &position[0],
          GL_STATIC_DRAW
        );

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
        glEnableVertexAttribArray(0);
      }
    }

    

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    loaded_mesh.vao_.push_back(vao);
    loaded_mesh.vbo_.push_back(vbo);
    loaded_mesh.ebo_.push_back(ebo);
  }


  meshes_.push_back(loaded_mesh);  
}

void CustomModel::Unload() {
  for (CustomMesh& mesh : meshes_)  {
    for (int i = 0; i < mesh.vao_.size(); ++i) {
      glDeleteVertexArrays(1, &mesh.vao_[i]);
      glDeleteBuffers(1, &mesh.vbo_[i]);
      glDeleteBuffers(1, &mesh.ebo_[i]);
    } 
  }
}


void CustomModel::Draw(
  FlyCamera& camera, 
  Shader shader,
  int model_matrix_loc,
  int view_projection_loc,
  int base_color_loc,
  Vector3 position,
  Quaternion rotation,
  Vector3 scale
) {
  Vector3 cam_position = camera.GetCamera().GetPosition();
  Vector3 cam_forward = camera.GetCamera().GetForward();

  float screen_width = (float)GetScreenWidth();
  float screen_height = (float)GetScreenHeight();
 
  Matrix projection = MatrixPerspective(
    camera.GetCamera().GetFOV() * DEG2RAD,
    screen_width / screen_height, 
    0.01, 
    100.0
  );
  
  Matrix view = MatrixLookAt(
    cam_position, 
    Vector3Add(cam_position, cam_forward), 
    { 0.f, 1.f, 0.f }
  );

  Matrix view_projection = MatrixMultiply(view, projection);

  Matrix offset = MatrixIdentity();

  offset = MatrixMultiply(
    offset, 
    MatrixScale(scale.x, scale.y, scale.z)
  );
  offset = MatrixMultiply(
    offset, 
    QuaternionToMatrix(rotation)
  );
  offset = MatrixMultiply(
    offset, 
    MatrixTranslate(position.x, position.y, position.z)
  );
    
  glUseProgram(shader.id);

  SetShaderValueMatrix(shader, view_projection_loc, view_projection);

  for (int i = 0; i < meshes_.size(); ++i) {
    for (int j = 0; j < meshes_[i].vao_.size(); ++j) {

      SetShaderValueMatrix(
        shader, 
        model_matrix_loc, 
        offset
      );

      SetShaderValue(
        shader, 
        base_color_loc, 
        &meshes_[i].base_color_[j], 
        SHADER_UNIFORM_VEC3
      );

      glBindVertexArray(meshes_[i].vao_[j]);
      glDrawElements(
        GL_TRIANGLES, 
        meshes_[i].index_count_[j],
        GL_UNSIGNED_INT, 
        nullptr
      );
      glBindVertexArray(0);
    }
  }

  glUseProgram(0);
}

const BoundingBox GetMeshBounds(const CustomMesh& mesh) {
  Vector3 min = mesh.mins_[0];
  Vector3 max = mesh.maxs_[0];

  for (int i = 1; i < mesh.mins_.size(); ++i) {
    min = Vector3Min(min, mesh.mins_[i]);
    max = Vector3Max(min, mesh.maxs_[i]);
  }

  return { min, max };
}

const BoundingBox CustomModel::GetBoundingBox() const {
  BoundingBox bounds = { 0 };

  if (!meshes_.empty()) {
    Vector3 temp = { 0 };
    bounds = GetMeshBounds(meshes_[0]);

    for (int i = 1; i < meshes_.size(); ++i) {
      BoundingBox temp_bounds = GetMeshBounds(meshes_[i]);

      bounds.min = Vector3Min(bounds.min, temp_bounds.min);
      bounds.max = Vector3Max(bounds.max, temp_bounds.max);
    }
  }

  return bounds;
}

