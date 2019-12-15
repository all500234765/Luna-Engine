struct VelocityBuff {
    #include "Velocity.h"
};

struct TransformBuff {
    #include "Transform.h"
};

struct CameraBuff {
    #include "Camera.h"
};

struct VelocityComponent: ECSComponent<VelocityComponent> {
    #include "Velocity.h"
};

struct TransformComponent: ECSComponent<TransformComponent> {
    #include "Transform.h"
};

struct CameraComponent: ECSComponent<CameraComponent> {
    #include "Camera.h"
};

// Mesh
struct MeshComponent {
    VertexBuffer mVBPosition;
    VertexBuffer mVBTexcoord;
    VertexBuffer mVBNormal;
    //VertexBuffer mVBQuaternion; // TODO: GPU Pro 3; Replace tangent and bitangent with quats

    IndexBuffer mIndexBuffer;
};

struct MeshStaticComponent: ECSComponent<MeshStaticComponent>, MeshComponent { };

struct MeshAnimatedComponent: ECSComponent<MeshAnimatedComponent>, MeshComponent {
    VertexBuffer mVBWeights;
    VertexBuffer mVBJoints;

};

// 
/*struct: ECSComponent<> {

};

struct : ECSComponent<> {

};*/
