// Minimal headless smoke test for the Phantom style. Fails on:
// - crash when a style entry point is called with widget == nullptr
//   (QStyle allows a null widget; see the PE_IndicatorToolBarSeparator fix)
// - style entry points that leave the QPainter state modified
//   (pen, brush, render hints; see the CC_ScrollBar thumb fix)
// - any regression that breaks painting of basic widgets
#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPushButton>
#include <QStyleFactory>
#include <QStyleOptionButton>
#include <QStyleOptionSlider>
#include <QTableWidget>
#include <cstdio>

#include "phantomstyle.h"

static int failures = 0;

// Force an even thickness so the centered handle starts at y == -1 regardless
// of the fonts installed on the machine running the test.
class FixedThicknessPhantomStyle : public PhantomStyle {
 public:
  int pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option,
                  const QWidget* widget) const override {
    if (metric == QStyle::PM_SliderThickness)
      return 16;
    return PhantomStyle::pixelMetric(metric, option, widget);
  }
};

static void require(bool ok, const char* what) {
  if (!ok) {
    fprintf(stderr, "FAIL: %s\n", what);
    ++failures;
  }
}

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  app.setStyle(new PhantomStyle);
  require(QApplication::style()->inherits("PhantomStyle"), "style instantiated");

  {
    QTableWidget table(8, 3);
    for (int r = 0; r < 8; ++r)
      for (int c = 0; c < 3; ++c)
        table.setItem(r, c, new QTableWidgetItem(QString("%1,%2").arg(r).arg(c)));
    table.resize(400, 300);
    QImage img = table.grab().toImage();
    require(!img.isNull() && img.size() == QSize(400, 300), "table rendered");

    QPushButton button("OK");
    button.resize(120, 32);
    img = button.grab().toImage();
    require(!img.isNull(), "button rendered");
  }

  {
    QImage img(220, 240, QImage::Format_ARGB32_Premultiplied);
    img.fill(0xffffffff);
    QPainter p(&img);
    QStyle* style = app.style();

    QStyleOption opt;
    opt.rect = QRect(0, 0, 200, 30);
    opt.state = QStyle::State_Enabled;
    for (int pe = QStyle::PE_Frame; pe <= QStyle::PE_IndicatorTabTear; ++pe)
      style->drawPrimitive((QStyle::PrimitiveElement)pe, &opt, &p, nullptr);

    QStyleOptionButton bopt;
    bopt.rect = opt.rect;
    bopt.state = opt.state;
    bopt.text = "OK";
    style->drawControl(QStyle::CE_PushButton, &bopt, &p, nullptr);
    style->drawControl(QStyle::CE_CheckBox, &bopt, &p, nullptr);
    style->drawControl(QStyle::CE_RadioButton, &bopt, &p, nullptr);

    QStyleOptionSlider sopt;
    sopt.rect = QRect(0, 0, 20, 200);
    sopt.state = QStyle::State_Enabled | QStyle::State_MouseOver;
    sopt.minimum = 0;
    sopt.maximum = 100;
    sopt.sliderPosition = 40;
    sopt.sliderValue = 40;
    sopt.orientation = Qt::Vertical;
    sopt.subControls = QStyle::SC_All;
    style->drawComplexControl(QStyle::CC_ScrollBar, &sopt, &p, nullptr);
    p.end();
  }

  {
    QImage img(220, 240, QImage::Format_ARGB32_Premultiplied);
    img.fill(0xffffffff);
    QPainter p(&img);
    QStyleOptionSlider sopt;
    sopt.rect = QRect(0, 0, 20, 200);
    sopt.state = QStyle::State_Enabled | QStyle::State_MouseOver;
    sopt.minimum = 0;
    sopt.maximum = 100;
    sopt.sliderPosition = 40;
    sopt.sliderValue = 40;
    sopt.orientation = Qt::Vertical;
    sopt.subControls = QStyle::SC_All;
    const int beforeHints = (int)p.renderHints();
    const int beforePen = (int)p.pen().style();
    const int beforeBrush = (int)p.brush().style();
    app.style()->drawComplexControl(QStyle::CC_ScrollBar, &sopt, &p, nullptr);
    require((int)p.renderHints() == beforeHints &&
                (int)p.pen().style() == beforePen &&
                (int)p.brush().style() == beforeBrush,
            "CC_ScrollBar leaves painter state unchanged");
    p.end();
  }

  {
    // Regression test: the centered handle used to start at -1 for even
    // thicknesses, so its 1 px leading outline was clipped away.
    FixedThicknessPhantomStyle style;
    for (int vertical = 0; vertical < 2; ++vertical) {
      for (int dim : {15, 16}) {
        QStyleOptionSlider sopt;
        sopt.state = QStyle::State_Enabled;
        sopt.rect = vertical ? QRect(0, 0, dim, 120) : QRect(0, 0, 120, dim);
        sopt.minimum = 0;
        sopt.maximum = 100;
        sopt.sliderPosition = 40;
        sopt.sliderValue = 40;
        sopt.orientation = vertical ? Qt::Vertical : Qt::Horizontal;
        sopt.subControls = QStyle::SC_All;
        const QRect handle = style.subControlRect(
            QStyle::CC_Slider, &sopt, QStyle::SC_SliderHandle, nullptr);
        require(handle.top() >= 0 && handle.left() >= 0,
                "slider handle outline is not clipped");
      }
    }
  }

  if (failures) {
    fprintf(stderr, "smoketest: %d failure(s)\n", failures);
    return 1;
  }
  printf("smoketest: all checks passed\n");
  return 0;
}
