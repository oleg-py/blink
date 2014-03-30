#include "blinkcore.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtCore/QThread>

const QString BlinkCore::UrlTemplate { QStringLiteral("http://myanimelist.net/malappinfo.php?u=%1&status=all&type=%2") };
const QString BlinkCore::UrlAnimePart { QStringLiteral("anime") };
const QString BlinkCore::UrlMangaPart { QStringLiteral("manga") };

BlinkCore::BlinkCore(QObject *parent) :
    QObject(parent),
    m_animelistWriter(this), m_animelistParser(new BlinkParser),
    m_mangalistWriter(this), m_mangalistParser(new BlinkParser),
    m_manager(new QNetworkAccessManager(this)), m_parsingThread(new QThread(this)),
    m_processAnimelist(false), m_processMangalist(false), m_finishesNeeded(0)
{
    m_formats["more"] = "#more~{background-image:url(\'~\')}";
    m_formats["more:before"] = "#more~{background-image:url(\'*\')}";
    m_formats["more:after"] = "#more~:after{background-image:url(\'~\')}";
    m_formats["animetitle"] = ".animetitle[href*=\"/~/\"]{background-image: url(\'~\')}";
    m_formats["animetitle:before"] = ".animetitle[href*=\"/~/\"]:before{background-image: url(\'~\')}";
    m_formats["animetitle:after"] = ".animetitle[href*=\"/~/\"]:after{background-image: url(\'~\')}";

    m_animelistParser->moveToThread(m_parsingThread);
    m_mangalistParser->moveToThread(m_parsingThread);

    connect(m_animelistParser, SIGNAL(write(QString,QString)), &m_animelistWriter, SLOT(write(QString,QString)));
    connect(m_mangalistParser, SIGNAL(write(QString,QString)), &m_mangalistWriter, SLOT(write(QString,QString)));

    connect(this, SIGNAL(parseAnimelist(QNetworkReply*)), m_animelistParser, SLOT(fire(QNetworkReply*)));
    connect(this, SIGNAL(parseMangalist(QNetworkReply*)), m_mangalistParser, SLOT(fire(QNetworkReply*)));

    connect(m_animelistParser, SIGNAL(totalCount(int)),this, SIGNAL(animelistTotal(int)));
    connect(m_animelistParser, SIGNAL(currentProgress(int)), this, SIGNAL(animelistProcessed(int)));
    connect(m_animelistParser, SIGNAL(writingAborted(QString)), this, SIGNAL(error(QString)));
    connect(m_animelistParser, SIGNAL(writingFinished()), this, SLOT(onConverterFinished()));

    connect(m_mangalistParser, SIGNAL(totalCount(int)), this, SIGNAL(mangalistTotal(int)));
    connect(m_mangalistParser, SIGNAL(currentProgress(int)), this, SIGNAL(mangalistProcessed(int)));
    connect(m_mangalistParser, SIGNAL(writingAborted(QString)), this, SIGNAL(error(QString)));
    connect(m_mangalistParser, SIGNAL(writingFinished()), this, SLOT(onConverterFinished()));
    m_parsingThread->start();
}

BlinkCore::~BlinkCore()
{
    m_parsingThread->exit();
    delete m_manager;
    delete m_parsingThread;
    delete m_animelistParser;
    delete m_mangalistParser;
}

void BlinkCore::startProcessing()
{
    m_finishesNeeded = 0;
    if (m_processAnimelist) {
        m_finishesNeeded++;
        if (!m_animelistWriter.open()) {
            emit error("Failed to launch the animelist writer");
        }
        emit parseAnimelist(m_manager->get(wrap(UrlTemplate.arg(m_username, UrlAnimePart))));
    }
    if (m_processMangalist) {
        m_finishesNeeded++;
        if (!m_mangalistWriter.open()) {
            emit error("Failed to launch the mangalist writer");
        }
        emit parseMangalist(m_manager->get(wrap(UrlTemplate.arg(m_username, UrlMangaPart))));
    }
}

void BlinkCore::onConverterFinished()
{
    m_finishesNeeded--;
    if (m_finishesNeeded == 0) {
        if (m_processAnimelist) m_animelistWriter.close();
        if (m_processMangalist) m_mangalistWriter.close();
        emit finished();
    }
}
