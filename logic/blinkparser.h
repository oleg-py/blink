#ifndef BLINKPARSER_H
#define BLINKPARSER_H

#include <QObject>

class QNetworkReply;
class QXmlStreamReader;

class BlinkParser : public QObject
{
    Q_OBJECT

    QNetworkReply *m_reply;
    QXmlStreamReader *m_reader;

    bool m_atUserInfo;
    int m_total;
    int m_current;
    QString m_currentId;
    QString m_currentImgLink;

public:
    explicit BlinkParser(QObject *parent = 0);
    ~BlinkParser();

public slots:
    void fire(QNetworkReply *reply);
    void abort();

private:
    void parseXml();

private slots:
    void onReplyReadyRead();
    void checkResultCode();

signals:
    void writingFinished();
    void write(const QString& id, const QString& link);
    void writingError(QString err);
    void totalCount(int);
    void currentProgress(int);
};

#endif // BLINKPARSER_H
