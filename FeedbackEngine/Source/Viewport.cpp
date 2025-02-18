#include "Viewport.h"
#include "Window.h"
#include "DeviceContext.h"

HRESULT 
Viewport::init(const Window& window) {
  if (!window.m_hWnd) {
    ERROR("Viewport", "init", "Window handle (m_hWnd) is nullptr");
    return E_POINTER;
  }
  if (window.m_width == 0 || window.m_height == 0) {
    ERROR("Viewport", "init", "Width or height is zero");
    return E_INVALIDARG;
  }

  m_viewport.Width = static_cast<float>(window.m_width);
  m_viewport.Height = static_cast<float>(window.m_height);
  m_viewport.MinDepth = 0.0f;
  m_viewport.MaxDepth = 1.0f;
  m_viewport.TopLeftX = 0;
  m_viewport.TopLeftY = 0;

  return S_OK;
}

HRESULT Viewport::init(unsigned int width, unsigned int height) {
  if (width == 0 || height == 0) {
    ERROR("Viewport", "init", "Width or height is zero");
    return E_INVALIDARG;
  }

  m_viewport.Width = static_cast<float>(width);
  m_viewport.Height = static_cast<float>(height);
  m_viewport.MinDepth = 0.0f;
  m_viewport.MaxDepth = 1.0f;
  m_viewport.TopLeftX = 0;
  m_viewport.TopLeftY = 0;

  return S_OK;
}

void Viewport::update() {
  // Método vacío, pero se puede usar para manejar cambios dinámicos en el viewport
}

void Viewport::render(DeviceContext& deviceContext) {
  if (!deviceContext.m_deviceContext) {
    ERROR("Viewport", "render", "DeviceContext is nullptr");
    return;
  }

  deviceContext.RSSetViewports(1, &m_viewport);
}

void Viewport::destroy() {
  // No hay recursos dinámicos que liberar, pero se mantiene el método para futura expansión.
}
