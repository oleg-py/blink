#ifndef BLINKWRITER_H
#define BLINKWRITER_H

#include <QObject>

class BlinkWriter : public QObject
{
    Q_OBJECT
public:
    explicit BlinkWriter(QObject *parent = 0);

signals:

public slots:

};

#endif // BLINKWRITER_H
