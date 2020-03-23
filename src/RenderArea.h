#pragma once

#include <QWidget>

#include <memory>

#include "Subject.h"

namespace cvd {

class RenderArea final : public QWidget {
  Q_OBJECT
public:
  explicit RenderArea(QWidget *parent = nullptr);
  void redraw(const std::shared_ptr<Subjects> &subjects);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  void drawEdges();
  void drawSubject(const QPointF &center, float radius, Subject::Status status);

private:
  std::shared_ptr<Subjects> subjecsRefs_ = nullptr;
};

} // namespace cvd
