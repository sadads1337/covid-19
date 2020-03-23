#pragma once

#include <QMainWindow>
#include <QTimer>

#include <memory>

#include "Subject.h"
#include "ui_MainWindow.h"

namespace cvd {

class MainWindow final : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

private:
  struct Params final {
    size_t number;
    float sickPercentage;
    float radius;
    float sickTime;
    float minimalSpeed;
  };

private:
  [[nodiscard]] static Subjects generateSubjects(const Params &params,
                                                 const QRect &rect);
  void recreateSubjects();
  void updateSubjects();

private slots:
  void updateRenderArea();
  void updateNumber(int value);
  void updateSickPercentage(int value);
  void updateRadius(int value);
  void updateSickTime(int value);
  void updateSpeed(int value);

private:
  std::unique_ptr<Ui::MainWindow> ui_;
  Params params_;
  std::shared_ptr<Subjects> subjects_;
  QTimer timer_;
};

} // namespace cvd
