#ifndef BLINKWRITER_H
#define BLINKWRITER_H

#include <QtCore/QObject>
#include <QtCore/QString>

class QFile;
class QTextStream;

class BlinkWriter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString outputPath READ getOutputPath WRITE setOutputPath)
    Q_PROPERTY(QString outputFormat READ getOutputFormat WRITE setOutputFormat)

    QString m_outputPath;
    QString m_outputFormat;

    QString m_format_beforeID;
    QString m_format_beforeLink;
    QString m_format_rest;
    QFile *m_outputFile;
    QTextStream *m_outputStream;

public:
    explicit BlinkWriter(QObject *parent = 0);
    ~BlinkWriter();
    bool open();
    void close();

    QString getOutputPath() const { return m_outputPath; }
    QString getOutputFormat() const { return m_outputFormat; }

public slots:
    void setOutputPath(QString arg) { m_outputPath = arg; }
    void setOutputFormat(QString arg) { m_outputFormat = arg; }
    void write(const QString& id, const QString& link);
};

#endif // BLINKWRITER_H
