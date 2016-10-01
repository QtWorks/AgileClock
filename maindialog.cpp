#include "maindialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QTimer>
#include <QFont>
#include <QPainter>
#include <QImage>
#include <QtSvg/QSvgRenderer>
#include <QGraphicsDropShadowEffect>

#include <stdio.h>

#include <QDebug>

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
{
    setContentsMargins(0,0,0,0);
//    setAttribute(Qt::WA_TranslucentBackground);
    setWindowOpacity(0.8);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    QGraphicsDropShadowEffect* minEffect = new QGraphicsDropShadowEffect (this);
    minEffect->setOffset(QPoint(0,0));
    minEffect->setColor (QColor (0, 0, 0, 128));
    minEffect->setBlurRadius (16);
    QGraphicsDropShadowEffect* secEffect = new QGraphicsDropShadowEffect (this);
    secEffect->setOffset(QPoint(0,0));
    secEffect->setColor (QColor (0, 0, 0, 128));
    secEffect->setBlurRadius (16);
    QGraphicsDropShadowEffect* sepEffect = new QGraphicsDropShadowEffect (this);
    sepEffect->setOffset(QPoint(0,0));
    sepEffect->setColor (QColor (0, 0, 0, 128));
    sepEffect->setBlurRadius (16);

    miMinutes = 2, miSeconds = 0;

    QVBoxLayout *lpGlobalVLay = new QVBoxLayout;
    lpGlobalVLay->setContentsMargins(0,0,0,0);
    lpGlobalVLay->setSpacing(0);

    QHBoxLayout *lpGlobalHLay = new QHBoxLayout;
    lpGlobalVLay->setContentsMargins(0,0,0,0);
    lpGlobalVLay->setSpacing(0);

    mpMinFrame = new QFrame(this); mpMinFrame->setContentsMargins(0,0,0,0);
    QHBoxLayout *mpMinFrameLay = new QHBoxLayout(this); mpMinFrameLay->setContentsMargins(0,0,0,0); mpMinFrameLay->setSpacing(0);
    mpSecFrame = new QFrame(this); mpSecFrame->setContentsMargins(0,0,0,0);
    QHBoxLayout *mpSecFrameLay = new QHBoxLayout(this); mpSecFrameLay->setContentsMargins(0,0,0,0); mpSecFrameLay->setSpacing(0);

    mpMinutes = new QLabel(this);
    mpMinutes->setContentsMargins(0,0,0,0);
    mpMinutes->setText(QString::number (miMinutes).rightJustified(2,QChar('0')));
    mpMinutes->setObjectName("timeLabel");
    mpMinutes->setGraphicsEffect (minEffect);

    mpSeconds = new QLabel(this);
    mpSeconds->setContentsMargins(0,0,0,0);
    mpSeconds->setText(QString::number (miSeconds).rightJustified(2,QChar('0')));
    mpSeconds->setObjectName("timeLabel");
    mpSeconds->setGraphicsEffect (secEffect);

    mpMinFrameLay->addWidget(mpMinutes);
    mpMinFrame->setLayout(mpMinFrameLay);

    mpSecFrameLay->addWidget(mpSeconds);
    mpSecFrame->setLayout(mpSecFrameLay);

    separator = new QLabel(":",this);
    separator->setContentsMargins(0,0,0,0);
    separator->setObjectName("timeLabel");
    separator->setGraphicsEffect (sepEffect);

    lpGlobalHLay->addWidget(mpMinFrame);
    lpGlobalHLay->addWidget(separator);
    lpGlobalHLay->addWidget(mpSecFrame);

    QLabel *lpCloseButton = new QLabel(this); lpCloseButton->setContentsMargins(0,0,0,0);
    lpCloseButton->setPixmap (QPixmap(":/images/resources/img_close.png"));

    lpGlobalVLay->addWidget(lpCloseButton);
    lpGlobalVLay->addLayout(lpGlobalHLay);

    setLayout(lpGlobalVLay);

    mpTimer = new QTimer(this);
    mpTimer->setInterval(1000);

    connect(mpTimer, SIGNAL(timeout()), this, SLOT(secLess()));

}

MainDialog::~MainDialog()
{

}

void MainDialog::wheelEvent(QWheelEvent * event)
{
    if(mpTimer->isActive()) resetClock(false);

    if(mpMinFrame->geometry().contains(event->pos()))
    {
        //timer stops and reset minute and second values

        int mins = mpMinutes->text ().toInt();
        if(event->delta() < 0) mins = qMax(--mins,0);
        else mins = qMin(++mins,99);
        miMinutes = mins;
        mpMinutes->setText(QString::number (mins).rightJustified(2,QChar('0')));
    }
    else if(mpSecFrame->geometry().contains(event->pos()))
    {
        //timer stops and reset minute and second values

        int secs = mpSeconds->text ().toInt();
        if(event->delta() < 0)
        {
            secs -= 1;
            if(secs < 0) secs = 59;
        }
        else
        {
            secs += 1;
            if(secs > 59) secs = 0;
        }
        miSeconds = secs;
        mpSeconds->setText(QString::number (secs).rightJustified(2,QChar('0')));
    }
}

void MainDialog::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        oldPosition= event->pos();
        acumDespl = QPoint(0,0);
        mbTimerOn = true;
    }
}

void MainDialog::mouseMoveEvent(QMouseEvent * event)
{
    if(event->buttons() != Qt::LeftButton) return;

    QPoint position = event->pos();
    QPoint despl = position - oldPosition;

    move(pos () + despl);
    acumDespl += despl;
}

void MainDialog::mouseReleaseEvent(QMouseEvent * event)
{
    if(event->button() == Qt::RightButton)
    {
        QDialog *dlg = new QDialog(this);
        dlg->setWindowFlags(dlg->windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        dlg->setAttribute (Qt::WA_DeleteOnClose);
        dlg->show();
        return;
    }

    if(event->button() == Qt::RightButton) return;
    if(acumDespl.manhattanLength() < 3 && mbTimerOn)
        resetClock(true);
}

void MainDialog::mouseDoubleClickEvent(QMouseEvent * event)
{
    mpMinutes->setText(QString::number (miMinutes).rightJustified(2,QChar('0')));
    mpSeconds->setText(QString::number (miSeconds).rightJustified(2,QChar('0')));
    resetClock(false);
}

void MainDialog::resetClock(bool lbTimerActive)
{
    mpMinutes->setText(QString::number (miMinutes).rightJustified(2,QChar('0')));
    mpSeconds->setText(QString::number (miSeconds).rightJustified(2,QChar('0')));

    mbTimeout = false;

    qDebug () << "New state: " << ((lbTimerActive)?"Started":"Stoped");

    if (lbTimerActive) mpTimer->start ();
    else mpTimer->stop();

    mbTimerOn = lbTimerActive;
}

void MainDialog::secLess()
{
    int mins = mpMinutes->text ().toInt();
    int secs = mpSeconds->text ().toInt() + ((mbTimeout)? 1 : - 1);
    if(secs < 0 || secs > 59)
    {
        mins += ((mbTimeout)? 1 : - 1);
        if (mins < 0)
        {
            mins = 0;
            mbTimeout = true;
            secs = 1;
        }
        else
        {
            if(mbTimeout && secs > 59) secs = 0;
            else secs = 59;
        }
    }
    mpMinutes->setText(QString::number (mins).rightJustified(2,QChar('0')));
    mpSeconds->setText(QString::number (secs).rightJustified(2,QChar('0')));
}