#pragma once

namespace pendule_pi {

class Encoder {
public:
  Encoder(/* */);
  virtual ~Encoder() = default;

  const int& getSteps() const;
private:

  int count;
};

}
