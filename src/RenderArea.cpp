#include "RenderArea.h"

#include <QPainter>

namespace cvd {

RenderArea::RenderArea(QWidget *const parent) : QWidget{parent} {
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);
}

void RenderArea::paintEvent([[maybe_unused]] QPaintEvent *const event) {
  drawEdges();
  if (subjecsRefs_) {
    for (const auto &subject : *subjecsRefs_) {
      drawSubject(subject.pos, subject.radius, subject.status);
    }
  }
}

void RenderArea::drawEdges() {
  const auto &rect = geometry();

  QPainter painter{this};
  painter.setPen(palette().dark().color());
  painter.setBrush(Qt::SolidPattern);
  painter.setRenderHint(QPainter::Antialiasing, true);

  painter.drawLine(rect.bottomLeft(), rect.topLeft());
  painter.drawLine(rect.topLeft(), rect.topRight());
  painter.drawLine(rect.topRight(), rect.bottomRight());
  painter.drawLine(rect.bottomRight(), rect.bottomLeft());
}

void RenderArea::drawSubject(const QPointF &center, const float radius,
                             const Subject::Status status) {
  const auto &rect = geometry();

  QPainter painter{this};
  switch (status) {
  case Subject::Status::Healthy:
    painter.setBrush(QBrush{Qt::green, Qt::SolidPattern});
    break;
  case Subject::Status::Sick:
    painter.setBrush(QBrush{Qt::red, Qt::SolidPattern});
    break;
  case Subject::Status::Recovered:
    painter.setBrush(QBrush{Qt::blue, Qt::SolidPattern});
    break;
  default:
    assert(false);
  }
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.drawEllipse(center, radius, radius);
}

void RenderArea::redraw(const std::shared_ptr<Subjects> &subjects) {
  subjecsRefs_ = subjects;
  update();
}

} // namespace cvd
