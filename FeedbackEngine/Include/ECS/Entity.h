#pragma once
#include "Prerequisites.h"
#include "Component.h"
class DeviceContext;

class 
Entity {
public:
  /**
   * @brief Destructor virtual.
   */
  virtual
  ~Entity() = default;

  /**
   * @brief Método virtual puro para actualizar la entidad.
   * @param deltaTime El tiempo transcurrido desde la última actualización.
   * @param deviceContext Contexto del dispositivo para operaciones gráficas.
   */
  virtual void
  update(float deltaTime, DeviceContext& deviceContext) = 0;

  /**
   * @brief Método virtual puro para renderizar la entidad.
   * @param deviceContext Contexto del dispositivo para operaciones gráficas.
   */
  virtual void
  render(DeviceContext& deviceContext) = 0;

  /**
   * @brief Agrega un componente a la entidad.
   * @tparam T Tipo del componente, debe derivar de Component.
   * @param component Puntero compartido al componente que se va a agregar.
   */
  template <typename T>
  void addComponent(EngineUtilities::TSharedPointer<T> component) {
    static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
    m_components.push_back(component.template dynamic_pointer_cast<Component>());
  }

  /**
   * @brief Obtiene un componente de la entidad.
   * @tparam T Tipo del componente que se va a obtener.
   * @return Puntero compartido al componente, o nullptr si no se encuentra.
   */
  template<typename T>
  EngineUtilities::TSharedPointer<T>
    getComponent() {
    for (auto& component : m_components) {
      EngineUtilities::TSharedPointer<T> specificComponent = component.template dynamic_pointer_cast<T>();
      if (specificComponent) {
        return specificComponent;
      }
    }
    return EngineUtilities::TSharedPointer<T>();
  }

protected:
  bool m_isActive;

  int m_id;

  std::vector<EngineUtilities::TSharedPointer<Component>> m_components;
};