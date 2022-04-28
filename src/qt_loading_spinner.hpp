#pragma once

#include <QtWidgets/QtWidgets>
#include <QtCore/QtCore>
#include <QtGui/QtGui>

#include <chrono>

typedef std::chrono::steady_clock Time;
typedef Time::time_point Timestamp;

class QLoadingSpinner : public QWidget {
    Q_OBJECT
private:
    double scale = 1.0;
    double speed = 1.0;
    QColor color = QColor(255, 255, 255);

    QTimer* timer = nullptr;
    Timestamp lastRotationAt;
    double rotation = 0.0; // radians
    int drawAreaSize;
    bool checkmark = false;

public:
    QLoadingSpinner(QWidget* parent, double scale = 0.5, double speed = 1.0, QColor color = QColor(255, 255, 255)): QWidget(parent) {
        this->scale = scale;
        this->speed = speed;
        this->color = color;

        // and one initial rotation to get things setup
        this->lastRotationAt = Time::now();
        this->rotate();

        // then update at approximately 144 hz
        this->timer = new QTimer(this);
        this->timer->setInterval(1000 / 144);
        this->connect(this->timer, &QTimer::timeout, this, &QLoadingSpinner::rotate);
        this->timer->start();
    }

    void recenter() {
        this->drawAreaSize = std::min(
            this->parentWidget()->width(),
            this->parentWidget()->height()
        ) * this->scale + 0.5;
        
        this->resize(drawAreaSize * 2, drawAreaSize * 2);

        this->move(
            this->parentWidget()->width() / 2 - drawAreaSize,
            this->parentWidget()->height() / 2 - drawAreaSize
        );
    }

    void setCheckmark(bool value) {
        if (value == this->checkmark) return;

        this->checkmark = value;
        if (this->checkmark) {
            this->timer->stop();
            this->repaint();
        }
        else {
            this->repaint();
            this->timer->start();
        }
    }

private slots:
    void rotate() {
        Timestamp now = Time::now();
        double secondsSinceLastRotation = (now - this->lastRotationAt).count() / 1000000000.0;
        this->lastRotationAt = now;

        this->rotation -= secondsSinceLastRotation * this->speed * 6.283185307179586;
        this->rotation = fmod(this->rotation, 6.283185307179586);

        this->repaint();
    }

protected:
    double pixel(double f) {
        return (f / 2.0 + 1.0) * this->drawAreaSize;
    }

    QPointF point(double x, double y) {
        return QPointF(this->pixel(x), this->pixel(y));
    }

    QRectF rectangle(double x1, double y1, double x2, double y2) {
        x1 = this->pixel(x1), y1 = this->pixel(y1);
        x2 = this->pixel(x2), y2 = this->pixel(y2);

        return QRectF(
            x1,
            y1,
            x2 - x1,
            y2 - y1
        );
    }

    double radians(double degrees) {
        return degrees * 0.017453292519943295;
    }

    double degrees(double radians) {
        return radians * 57.29577951308232;
    }

    int angle(double radians)
    {
        return degrees(radians) * 16.0 + 0.5; // +0.5 for rounding to int
    }

    void paintSpinner(QPainter& painter) {
        painter.setPen(QPen(this->color, 2.5));
        painter.drawArc(
            this->rectangle(-1, -1, 1, 1),
            this->angle(this->rotation),
            this->angle(1.5707963267948966)
        );
        painter.drawArc(
            this->rectangle(-1, -1, 1, 1),
            this->angle(this->rotation + 3.141592653589793),
            this->angle(1.5707963267948966)
        );
    }

    void paintCheckmark(QPainter& painter) {
        painter.setPen(QPen(this->color, 2.5));

        const QPointF a = this->point(-0.85, -0.1), b = this->point(-0.15, 0.75), c = this->point(0.85, -0.6);
        painter.drawLine(a, b);
        painter.drawLine(b, c);
    }

    void paintEvent(QPaintEvent* paintEvent) {
        this->recenter();
        QPainter painter(this);
        painter.fillRect(this->rect(), Qt::transparent);
        painter.setRenderHint(QPainter::Antialiasing, true);

        if (this->checkmark) {
            this->paintCheckmark(painter);
        }
        else {
            this->paintSpinner(painter);
        }
    }
};