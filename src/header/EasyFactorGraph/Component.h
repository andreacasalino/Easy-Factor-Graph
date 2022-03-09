/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

namespace EFG {
class Component {
public:
  virtual ~Component() = default;

protected:
  Component() = default;
};

class NonCopiableComponent : virtual public Component {
public:
  virtual ~NonCopiableComponent() override = default;

  NonCopiableComponent(const NonCopiableComponent &) = delete;
  NonCopiableComponent &operator==(const NonCopiableComponent &) = delete;

protected:
  NonCopiableComponent() = default;
};

class NonMovableComponent : virtual public Component {
public:
  virtual ~NonMovableComponent() override = default;

  NonMovableComponent(NonMovableComponent &&) = delete;
  NonMovableComponent &operator==(NonMovableComponent &&) = delete;

protected:
  NonMovableComponent() = default;
};

class NonCopiableAndMovableComponent : public NonCopiableComponent,
                                       public NonMovableComponent {
protected:
  NonCopiableAndMovableComponent() = default;
};
} // namespace EFG
