#pragma once

#include <QVector2D>

#include <vector>

namespace cvd {

struct Subject final {
  enum class Status {
    Healthy,
    Sick,
    Recovered,
  };

  QPointF pos;
  QVector2D direction;
  float speed;
  float radius;
  Status status;
  float sickTimeRemaining;
  bool freezed;

  Subject() = delete;
};

using Subjects = std::vector<Subject>;

} // namespace cvd
