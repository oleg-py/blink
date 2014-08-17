#ifndef BLINKCORE_H
#define BLINKCORE_H

#include <QObject>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QString>
#include <QtNetwork/QNetworkRequest>
#include "logic/blinkwriter.h"
#include "logic/blinkparser.h"

class QNetworkAccessManager;
class QThread;

class BlinkCore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString username READ username WRITE setUsername())
    Q_PROPERTY(QString animeOutPath READ getAnimeOutPath WRITE setAnimeOutPath STORED false)
    Q_PROPERTY(QString mangaOutPath READ getMangaOutPath WRITE setMangaOutPath STORED false)
    Q_PROPERTY(bool processAnimelist READ animelist WRITE processAnimelist)
    Q_PROPERTY(bool processMangalist READ mangalist WRITE processMangalist)

    QString m_username;
    QMap<QString, QString> m_formats;
    BlinkWriter *m_animelistWriter;
    BlinkParser *m_animelistParser;
    BlinkWriter *m_mangalistWriter;
    BlinkParser *m_mangalistParser;
    QNetworkAccessManager *m_manager;
    QThread *m_backgroundThread;
    bool m_processAnimelist;
    bool m_processMangalist;
    int m_finishesNeeded;

    static const QString UrlTemplate;
    static const QString UrlAnimePart;
    static const QString UrlMangaPart;

public:
    explicit BlinkCore(QObject *parent = 0);
    ~BlinkCore();
    QStringList availableSelectorTypes() { return m_formats.keys(); }
    QString getAnimeOutPath() const { return m_animelistWriter->getOutputPath(); }
    QString getMangaOutPath() const { return m_mangalistWriter->getOutputPath(); }
    bool animelist() const { return m_processAnimelist; }
    bool mangalist() const { return m_processMangalist; }
    QString username() const { return m_username; }

signals:
    void parseAnimelist(QNetworkReply*);
    void parseMangalist(QNetworkReply*);
    void error(QString msg);
    void animelistTotal(int);
    void animelistProcessed(int);
    void mangalistTotal(int);
    void mangalistProcessed(int);
    void finished();

public slots:
    void startProcessing();
    void setAnimeOutPath(QString arg) { m_animelistWriter->setOutputPath(arg); }
    void setMangaOutPath(QString arg) { m_mangalistWriter->setOutputPath(arg); }
    void setAnimeOutFormat(QString arg) { m_animelistWriter->setOutputFormat(m_formats[arg]); }
    void setMangaOutFormat(QString arg) { m_mangalistWriter->setOutputFormat(m_formats[arg]); }
    void processAnimelist(bool arg) { m_processAnimelist = arg; }
    void processMangalist(bool arg) { m_processMangalist = arg; }
    void setUsername(QString arg) { m_username = arg; }

private:
    inline QNetworkRequest wrap(QString url)
    {
        QNetworkRequest rq{QUrl(url)};
        rq.setHeader(QNetworkRequest::UserAgentHeader,
                     QStringLiteral("api-indiv-C37FBE47E15088A9F336C0CE9F55C8B4"));
        rq.setHeader(QNetworkRequest::ContentTypeHeader,
                     QStringLiteral("application/x-www-form-urlencoded"));
        return rq;
    }

private slots:
    void onConverterFinished();
    void abortOnError(QString msg);
};

#endif // BLINKCORE_H
