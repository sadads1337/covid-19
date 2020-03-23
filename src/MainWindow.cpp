#include "MainWindow.h"

#include <chrono>
#include <random>

namespace {

[[nodiscard]] auto generateRandomSpeed(const QRect &boundingBox,
                                       const float minimalSpeed) {
  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  assert(boundingBox.width() == boundingBox.height());
  const auto speed_limit = static_cast<float>(boundingBox.height()) / 50.f;
  std::uniform_real_distribution<> dist(speed_limit,
                                        speed_limit * minimalSpeed);
  return static_cast<float>(dist(generator));
}

[[nodiscard]] auto generateRandomPosition(const QRect &boundingBox) {
  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  assert(boundingBox.width() == boundingBox.height());
  std::uniform_real_distribution<> dist(
      0.f, static_cast<float>(boundingBox.height()));
  return QPointF{
      dist(generator),
      dist(generator),
  };
}

[[nodiscard]] auto generateRandomDirection(const QRect &boundingBox) {
  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  assert(boundingBox.width() == boundingBox.height());
  std::uniform_real_distribution<> dist(0.f, 1.f);
  return QVector2D{
      static_cast<float>(dist(generator)),
      static_cast<float>(dist(generator)),
  }
      .normalized();
}

constexpr auto gDeltaT = 0.1f;
constexpr auto gSickTime = 1.f;

} // namespace

namespace cvd {
MainWindow::MainWindow(QWidget *const parent)
    : QMainWindow{parent}, ui_{std::make_unique<Ui::MainWindow>()},
      params_{100u, 0.1f, 5.f, 10.f, 10.f} {
  ui_->setupUi(this);

  const auto *const renderArea = ui_->renderArea;
  assert(renderArea);
  const auto &rect = renderArea->geometry();
  subjects_ = std::make_shared<Subjects>(generateSubjects(params_, rect));

  timer_.setInterval(std::chrono::milliseconds{100u});
  timer_.setSingleShot(false);
  connect(&timer_, SIGNAL(timeout()), this, SLOT(updateRenderArea()));
  timer_.start();

  connect(ui_->sliderNumber, SIGNAL(valueChanged(int)), this,
          SLOT(updateNumber(int)));
  connect(ui_->sliderSpeed, SIGNAL(valueChanged(int)), this,
          SLOT(updateSpeed(int)));
  connect(ui_->sliderSickPercentage, SIGNAL(valueChanged(int)), this,
          SLOT(updateSickPercentage(int)));
  connect(ui_->sliderRadius, SIGNAL(valueChanged(int)), this,
          SLOT(updateRadius(int)));
  connect(ui_->sliderSickTime, SIGNAL(valueChanged(int)), this,
          SLOT(updateSickTime(int)));
}

Subjects MainWindow::generateSubjects(const Params &params, const QRect &rect) {
  Subjects result;
  result.reserve(params.number);
  for (auto i = 0u; i < params.number; ++i) {
    assert(params.sickPercentage >= 0.f && params.sickPercentage <= 1.f);
    bool sick = i < static_cast<size_t>(params.sickPercentage * params.number);
    result.push_back(Subject{
        generateRandomPosition(rect),
        generateRandomDirection(rect),
        generateRandomSpeed(rect, params.minimalSpeed),
        params.radius,
        sick ? Subject::Status::Sick : Subject::Status::Healthy,
        sick ? params.sickTime : -1.f,
    });
  }

  return result;
}

void MainWindow::updateSubjects() {
  assert(subjects_);
  for (auto subject_it = subjects_->begin(); subject_it < subjects_->end();
       ++subject_it) {
    auto &pos = subject_it->pos;
    auto &direction = subject_it->direction;
    const auto &speed = subject_it->speed;
    const auto &radius = subject_it->radius;
    auto &status = subject_it->status;
    auto &sickTimeRemaining = subject_it->sickTimeRemaining;

    if (status == Subject::Status::Sick) {
      assert(sickTimeRemaining > 0.);
      sickTimeRemaining -= gDeltaT;
      if (sickTimeRemaining < 0.) {
        status = Subject::Status::Recovered;
      }
    }

    auto newPos = QPointF{
        pos.x() + direction.x() * speed * gDeltaT,
        pos.y() + direction.y() * speed * gDeltaT,
    };

    auto *renderArea = ui_->renderArea;
    assert(renderArea);
    const auto &rect = renderArea->geometry();

    //! Detect edges collisions
    {
      if (newPos.x() - radius <= rect.left() ||
          newPos.x() + radius >= rect.right()) {
        direction.setX(-1.f * direction.x());
        newPos.rx() += direction.x() * speed * gDeltaT * 2.f;
      }

      if (newPos.y() - radius <= rect.top() ||
          newPos.y() + radius >= rect.bottom()) {
        direction.setY(-1.f * direction.y());
        newPos.ry() += direction.y() * speed * gDeltaT * 2.f;
      }
    }

    //! Detect subjects collisions
    {
      for (auto other_subject_it = subjects_->begin();
           other_subject_it < subjects_->end(); ++other_subject_it) {
        const auto other_subject_id =
            std::distance(subjects_->begin(), other_subject_it);
        const auto subject_id = std::distance(subjects_->begin(), subject_it);
        if (other_subject_id == subject_id) {
          continue;
        }
        auto &otherPos = other_subject_it->pos;
        auto &otherDirection = other_subject_it->direction;
        auto deltaPos = newPos - otherPos;
        auto distanceBetweenCenters = std::sqrt(deltaPos.x() * deltaPos.x() +
                                                deltaPos.y() * deltaPos.y());
        if (distanceBetweenCenters <= 2.f * radius) {
          direction.setX(-1.f * direction.x());
          direction.setY(-1.f * direction.y());
          newPos.rx() += direction.x() * speed * gDeltaT * 2.f;
          newPos.ry() += direction.y() * speed * gDeltaT * 2.f;
          pos = newPos;
          otherDirection.setX(-1.f * otherDirection.x());
          otherDirection.setY(-1.f * otherDirection.y());

          auto &otherStatus = other_subject_it->status;
          auto &otherSickTimeRemaining = other_subject_it->sickTimeRemaining;
          if (status == Subject::Status::Sick ||
              otherStatus == Subject::Status::Sick) {
            if (status != Subject::Status::Sick &&
                status != Subject::Status::Recovered) {
              status = Subject::Status::Sick;
              sickTimeRemaining = gSickTime;
            }
            if (otherStatus != Subject::Status::Sick &&
                otherStatus != Subject::Status::Recovered) {
              otherStatus = Subject::Status::Sick;
              otherSickTimeRemaining = gSickTime;
            }
          }
        }
        //! \todo: if debug only
        deltaPos = newPos - otherPos;
        distanceBetweenCenters = std::sqrt(deltaPos.x() * deltaPos.x() +
                                           deltaPos.y() * deltaPos.y());
        // assert(distanceBetweenCenters > 2.f * radius);
      }
    }
    pos = newPos;
  }
}

void MainWindow::updateRenderArea() {
  updateSubjects();
  auto *renderArea = ui_->renderArea;
  renderArea->redraw(subjects_);
}

void MainWindow::updateSpeed(const int value) {
  params_.minimalSpeed = static_cast<float>(value);

  recreateSubjects();
}

void MainWindow::updateSickPercentage(const int value) {
  const auto *const slider = ui_->sliderSickPercentage;
  const auto capacity =
      static_cast<float>(slider->maximum() - slider->minimum());
  params_.sickPercentage = static_cast<float>(value) / capacity;

  recreateSubjects();
}

void MainWindow::updateNumber(const int value) {
  params_.number = static_cast<size_t>(value);

  recreateSubjects();
}

void MainWindow::updateRadius(int value) {
  params_.radius = static_cast<float>(value);

  recreateSubjects();
}

void MainWindow::updateSickTime(int value) {
  params_.sickTime = gSickTime * static_cast<float>(value);
  recreateSubjects();
}

void MainWindow::recreateSubjects() {
  const auto *const renderArea = ui_->renderArea;
  assert(renderArea);
  const auto &rect = renderArea->geometry();
  subjects_ = std::make_shared<Subjects>(generateSubjects(params_, rect));
}

} // namespace cvd