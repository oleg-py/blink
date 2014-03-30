#ifndef BLINKWRITER_H
#define BLINKWRITER_H

#include <QtCore/QObject>
#include <QtCore/QString>

class QFile;
class QTextStream;
class QNetworkReply;
class QXmlStreamReader;

class BlinkConverter : public QObject
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

    QNetworkReply *m_reply;
    QXmlStreamReader *m_reader;

    bool m_atUserInfo;
    int m_total;
    int m_current;
    QString m_currentId;
    QString m_currentImgLink;

    void write();
    void parseXml();

public:
    explicit BlinkConverter(QObject *parent = 0);
    bool open();
    void close();
    void fire(QNetworkReply *reply);

    QString getOutputPath() const { return m_outputPath; }
    QString getOutputFormat() const { return m_outputFormat; }

public slots:
    void setOutputPath(QString arg) { m_outputPath = arg; }
    void setOutputFormat(QString arg) { m_outputFormat = arg; }

private slots:
    void onReplyReadyRead();

signals:
    void writingFinished();
    void writingAborted(QString err);
    void totalCount(int);
    void currentProgress(int);
};

#endif // BLINKWRITER_H
