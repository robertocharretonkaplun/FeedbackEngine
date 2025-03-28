#pragma once
// Librerias STD
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <xnamath.h>
//#include <memory>
#include <thread>

// Librerias DirectX
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "Resource.h"
#include "resource.h"

// Third Parties
#include "Utilities\Memory\TSharedPointer.h"
#include "Utilities\Memory\TWeakPointer.h"
#include "Utilities\Memory\TStaticPtr.h"
#include "Utilities\Memory\TUniquePtr.h"

// MACROS
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ << classObj << "::" << method << " : " << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
   OutputDebugStringW( os_.str().c_str() );  \
}

#define ERROR(classObj, method, errorMSG)                     \
{                                                             \
    try {                                                     \
        std::wostringstream os_;                              \
        os_ << L"ERROR : " << classObj << L"::" << method     \
            << L" : " << errorMSG << L"\n";                   \
        OutputDebugStringW(os_.str().c_str());                \
    } catch (...) {                                           \
        OutputDebugStringW(L"Failed to log error message.\n");\
    }                                                         \
}

// Structures
struct 
SimpleVertex {
  XMFLOAT3 Pos;
  XMFLOAT2 Tex;
};

struct 
CBNeverChanges {
  XMMATRIX mView;
};

struct 
CBChangeOnResize {
  XMMATRIX mProjection;
};

struct 
CBChangesEveryFrame {
  XMMATRIX mWorld;
  XMFLOAT4 vMeshColor;
};

enum ExtensionType {
  DDS = 0,
  PNG = 1,
  JPG = 2
};

enum ShaderType {
  VERTEX_SHADER = 0,
  PIXEL_SHADER = 1
};

enum 
ComponentType {
  NONE = 0,     ///< Tipo de componente no especificado.
  TRANSFORM = 1,///< Componente de transformación.
  MESH = 2,     ///< Componente de malla.
  MATERIAL = 3  ///< Componente de material.
};

struct Camera {
  XMFLOAT3 position;  // Posición de la cámara
  XMFLOAT3 target;    // Punto al que mira

  XMFLOAT3 up;        // Vector hacia arriba
  XMFLOAT3 forward;   // Dirección hacia adelante
  XMFLOAT3 right;     // Dirección hacia la derecha

  float yaw;          // Rotación en el eje Y
  float pitch;        // Rotación en el eje X

  Camera() {
    position = XMFLOAT3(0.0f, 1.6f, -5.0f);
    target = XMFLOAT3(0.0f, 1.6f, 0.0f);
    up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    forward = XMFLOAT3(0.0f, 0.0f, 1.0f);
    right = XMFLOAT3(1.0f, 0.0f, 0.0f);
    yaw = 0.0f;
    pitch = 0.0f;
  }
};

