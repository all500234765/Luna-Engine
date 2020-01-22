#include "pc.h"

struct VelocityBuff {
    #include "Velocity.h"
};

struct TransformBuff {
    #include "Transform.h"
};

struct CameraBuff {
    #include "Camera.h"
};

struct MaterialBuff {
    #include "Material.h"
};

struct WorldLightBuff {
    #include "WorldLight.h"
};

struct AmbientLightBuff {
    #include "AmbientLight.h"
};

struct SpotLightBuff {
    #include "SpotLight.h"
};

struct PointLightBuff {
    #include "PointLight.h"
};

struct BasicFogBuff {
    #include "BasicFog.h"
};

#include "Engine/Input/Mouse.h"
#include "Engine/Input/Gamepad.h"
#include "Engine/Input/Keyboard.h"
#include "Engine/DirectX/VertexBuffer.h"
#include "Engine/DirectX/IndexBuffer.h"

struct VolumetricLightComponent: ECSComponent<VolumetricLightComponent> {
    uint32_t mEnabled = true;
};

struct ShadowLightComponent: ECSComponent<ShadowLightComponent> {};
struct SpotLightComponent: ECSComponent<SpotLightComponent> {
    #include "SpotLight.h"
};

struct PointLightComponent: ECSComponent<PointLightComponent> {
    #include "PointLight.h"

    PointLightComponent(const PointLightBuff& buff) {
        memcpy((void*)&this->_LightColor.x, &buff._LightColor.x, sizeof(PointLightBuff));
    }

    PointLightBuff GetBuff() const {
        PointLightBuff b{};
            memcpy(&b._LightColor.x, (void*)&this->_LightColor.x, sizeof(PointLightBuff));
        return b;
    }

    void Bind(ConstantBuffer* cb, uint32_t types, uint32_t slot) const {
        {
            // Update CB
            ScopeMapConstantBufferCopy<PointLightBuff> q(cb, (void*)&this->_LightColor.x);
        }

        cb->Bind(types, slot);
    }
};

struct AreaLightComponent: ECSComponent<AreaLightComponent> {
    //#include "AreaLight.h"
};

struct AmbientLightComponent: ECSComponent<AmbientLightComponent> {
    #include "AmbientLight.h"

    void Bind(ConstantBuffer* cb, uint32_t types, uint32_t slot) {
        {
            // Update CB
            ScopeMapConstantBufferCopy<AmbientLightBuff> q(cb, (void*)&this->_AmbientLightColor);
        }

        cb->Bind(types, slot);
    }
};

struct WorldLightComponent: ECSComponent<WorldLightComponent> {
    #include "WorldLight.h"

    void Bind(ConstantBuffer* cb, uint32_t types, uint32_t slot) {
        {
            // Update CB
            ScopeMapConstantBufferCopy<WorldLightBuff> q(cb, (void*)&this->_WorldLightPosition);
        }

        cb->Bind(types, slot);
    }
};

struct TransformComponent: ECSComponent<TransformComponent> {
    #include "Transform.h"
    #include "Velocity.h"

    void Bind(ConstantBuffer* cb, uint32_t types, uint32_t slot) {
        {
            // Update CB
            ScopeMapConstantBufferCopy<TransformBuff> q(cb, (void*)&this->mWorld);
        }

        cb->Bind(types, slot);
    }

    void Build() {
        using namespace DirectX;
        mWorld = XMMatrixIdentity();

        mWorld *= XMMatrixRotationRollPitchYaw(XMConvertToRadians(vRotation.x), 
                                               XMConvertToRadians(vRotation.y), 
                                               XMConvertToRadians(vRotation.z));
        mWorld *= XMMatrixScaling(vScale.x, vScale.y, vScale.z);
        mWorld *= XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
    }
};

struct CameraComponent: ECSComponent<CameraComponent> {
    #include "Camera.h"

    void Bind(ConstantBuffer* cb, uint32_t types, uint32_t slot) {
        {
            // Update CB
            ScopeMapConstantBufferCopy<CameraBuff> q(cb, (void*)&this->mView);
        }

        cb->Bind(types, slot);
    }
};

struct MaterialComponent: ECSComponent<MaterialComponent> {
#define _MATERIAL_EXT_
    #include "Material.h"
    #include "MaterialTextures.h"
#undef _MATERIAL_EXT_

    void Bind(ConstantBuffer* cb, uint32_t types, uint32_t slot, uint32_t flags) {
        if( (flags & RendererFlags::DepthPass) == 0 ) {
            _Alb   |= _AlbedoTex           != nullptr;
            _Metal |= _MetallicTex         != nullptr;
            _Rough |= _RoughnessTex        != nullptr;
            _Emis  |= _EmissionTex         != nullptr;
            _AO    |= _AmbientOcclusionTex != nullptr;
        }

        {
            // Update CB
            ScopeMapConstantBufferCopy<MaterialBuff> q(cb, (void*)&this->_IsTransparent);
        }

        // Material params
        cb->Bind(types, slot);

        // Textures
        if( flags & RendererFlags::DontBindTextures ) return;
        Shader::ShaderType type = Shader::ShaderType(types);

        if( _AlbedoTex ) _AlbedoTex->Bind(type, 0);
        if( (flags & RendererFlags::DontBindSamplers) == 0 && _AlbedoSampl ) _AlbedoSampl->Bind(type, 0);

        if( (flags & RendererFlags::DepthPass) == 0 ) {
            if( _NormalTex           ) _NormalTex          ->Bind(type, 1);
            if( _MetallicTex         ) _MetallicTex        ->Bind(type, 2);
            if( _RoughnessTex        ) _RoughnessTex       ->Bind(type, 3);
            if( _EmissionTex         ) _EmissionTex        ->Bind(type, 4);
            if( _AmbientOcclusionTex ) _AmbientOcclusionTex->Bind(type, 5);

            if( flags & RendererFlags::DontBindSamplers ) return;
            if( _AlbedoSampl           ) _AlbedoSampl          ->Bind(type, 0);
            if( _NormalSampl           ) _NormalSampl          ->Bind(type, 1);
            if( _MetallicSampl         ) _MetallicSampl        ->Bind(type, 2);
            if( _RoughnessSampl        ) _RoughnessSampl       ->Bind(type, 3);
            if( _EmissionSampl         ) _EmissionSampl        ->Bind(type, 4);
            if( _AmbientOcclusionSampl ) _AmbientOcclusionSampl->Bind(type, 5);
        }
    }
};

struct InputControl {
    // Keyboard
    bool bKeyboard = false;
    uint32 mKeyboardKey;

    // Mouse
    bool bMouse = false;
    MouseButton mMouseButton;

    // Gamepad
    bool bGamepad = false;
    int mGamepadIndex = 0;
    GamepadButtonState mGamepadButton;

    // Is camera controller
    bool bOrientationDependent = false;
    bool bOrientationUpdate = false;

    bool bDisabled = false;

    // Keyboard only
    InputControl(uint32 key): bKeyboard(true), mKeyboardKey(key) {};

    // Mouse only
    InputControl(MouseButton mb): bMouse(true), mMouseButton(mb) {};

    // Gamepad only
    InputControl(GamepadButtonState b, int gpIndex = 0): bGamepad(true), mGamepadButton(b), mGamepadIndex(gpIndex) {};

    // Keyboard + Mouse
    InputControl(uint32 key, MouseButton mb): bKeyboard(true), mKeyboardKey(key), bMouse(true), mMouseButton(mb) {};

    // Keyboard + Gamepad
    InputControl(uint32 key, GamepadButtonState b, int gpIndex = 0): bKeyboard(true), mKeyboardKey(key),
        bGamepad(true), mGamepadButton(b), mGamepadIndex(gpIndex) {
    };

    // Mouse + Gamepad
    InputControl(MouseButton mb, GamepadButtonState b, int gpIndex = 0):
        bMouse(true), mMouseButton(mb),
        bGamepad(true), mGamepadButton(b), mGamepadIndex(gpIndex) {
    };

    // Keyboard + Mouse + Gamepad
    InputControl(uint32 key, MouseButton mb, GamepadButtonState b, int gpIndex = 0):
        bKeyboard(true), mKeyboardKey(key), bMouse(true), mMouseButton(mb),
        bGamepad(true), mGamepadButton(b), mGamepadIndex(gpIndex) {
    };

    // 
    float3 fValue = { 0.f, 0.f, 0.f };
    inline InputControl& SetValue(float a = 0.f, float b = 0.f, float c = 0.f) { fValue = { a, b, c }; return *this; }

    bool bDirectional = false;
    inline InputControl& SetDirectional(bool b) { bDirectional = b; return *this; }

    bool bCallback = false;
    void(*mCallback)(TransformComponent* T, float dt);
    inline InputControl& SetCallback(void(*callback)(TransformComponent* T, float dt)) {
        mCallback = callback;
        bCallback = true;
        return *this;
    }

    inline InputControl& OrientationDependent() { bOrientationDependent ^= true; return *this; }
    inline InputControl& OrientationUpdate() { bOrientationUpdate ^= true; return *this; }
};

struct MovementControlComponent: ECSComponent<MovementControlComponent> {
    std::vector<InputControl> mAssignedControls;
};

// Mesh
enum class MeshStaticFlags {
    Dynamic = 1,  // Won't be used to bake Radiance

};

struct MeshComponent {
    VertexBuffer *mVBPosition;
    VertexBuffer *mVBTexcoord;
    VertexBuffer *mVBNormal;
    VertexBuffer *mVBTangent;
    //VertexBuffer *mVBQuaternion; // TODO: GPU Pro 3; Replace tangent and bitangent with quats

    IndexBuffer *mIndexBuffer;

    uint32_t mReferenced{};
    uint32_t mInstanceCount{};
};

struct MeshStaticComponent: ECSComponent<MeshStaticComponent>, MeshComponent {
    VertexBuffer *mIndirectLight; // TODO: GI Backing into vertex color
    uint32_t mFlags{};            // MeshStaticFlags

    void Bind() {
        ID3D11Buffer *buffs[4] = { mVBPosition->GetBuffer(), mVBTexcoord->GetBuffer(), 
                                   mVBNormal->GetBuffer(),   mVBTangent->GetBuffer() };

        UINT strides[4] = { mVBPosition->GetStride(), mVBTexcoord->GetStride(), 
                            mVBNormal->GetStride(),   mVBTangent->GetStride() };
        UINT offsets[4] = { 0, 0, 0, 0 };

        gDirectX->gContext->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
        gDirectX->gContext->IASetIndexBuffer(mIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
    }
};

struct MeshAnimatedComponent: ECSComponent<MeshAnimatedComponent>, MeshComponent {
    VertexBuffer *mVBWeights;
    VertexBuffer *mVBJoints;
    
    void Bind() {
        ID3D11Buffer *buffs[6] = { mVBPosition->GetBuffer(), mVBTexcoord->GetBuffer(), 
                                   mVBNormal->GetBuffer(),   mVBTangent->GetBuffer(),
                                   mVBWeights->GetBuffer(),  mVBJoints->GetBuffer() };

        UINT strides[6] = { mVBPosition->GetStride(), mVBTexcoord->GetStride(), 
                            mVBNormal->GetStride(),   mVBWeights->GetStride(), 
                            mVBJoints->GetStride(),   mVBTangent->GetStride() };
        UINT offsets[6] = { 0, 0, 0, 0, 0, 0 };

        gDirectX->gContext->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
        gDirectX->gContext->IASetIndexBuffer(mIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
    }
};

// 
/*struct: ECSComponent<> {

};

struct : ECSComponent<> {

};*/
