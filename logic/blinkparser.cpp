#include "blinkparser.h"
#include <QtCore/QXmlStreamReader>
#include <QtNetwork/QNetworkReply>
#include <QThread>

BlinkParser::BlinkParser(QObject *parent) :
    QObject(parent)
{
    m_reader = new QXmlStreamReader();
    m_reply = nullptr;
}

BlinkParser::~BlinkParser()
{
    delete m_reader;
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
    }
}


void BlinkParser::fire(QNetworkReply *reply)
{
    // reset the state
    m_reply = reply;
    connect(m_reply, &QNetworkReply::finished,
            this, &BlinkParser::checkResultCode);
    m_total = m_current = 0;
    m_atUserInfo = false;
    m_reader->clear();
    m_reader->addData(reply->readAll());
    connect(m_reply, &QNetworkReply::readyRead,
            this, &BlinkParser::onReplyReadyRead);
    parseXml();
}

void BlinkParser::abort()
{
    qDebug("Aborting");
   if (m_reply != nullptr) {
       disconnect(m_reply);
       m_reply->abort();
       m_reply->deleteLater();
       m_reply = nullptr;
   }
}

void BlinkParser::onReplyReadyRead()
{
    if (m_reply != nullptr) {
        m_reader->addData(m_reply->readAll());
        parseXml();
    }
}

void BlinkParser::checkResultCode()
{
    qDebug("Check rescode");
    if (m_reply != nullptr) {
        int resultCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (resultCode != 200) {
            emit writingError(tr("Network error: reported \'%1\'").arg(
                                  m_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()));
        }
    }
}

void BlinkParser::parseXml()
{
    while (!m_reader->atEnd()) {
        switch(m_reader->readNext()) {
        case QXmlStreamReader::StartElement:
            if (m_reader->name().endsWith(QStringLiteral("db_id"))) {
                state = State::ID;
            } else if (m_reader->name() == QStringLiteral("series_image")) {
                state = State::ImgLink;
            } else if (m_reader->name() == QStringLiteral("myinfo")) {
                m_atUserInfo = true;
            } else if (m_atUserInfo) {
                // here we rip off the total progress that we need
                if (m_reader->name() == QStringLiteral("user_watching")
                        || m_reader->name() == QStringLiteral("user_reading")
                        || m_reader->name() == QStringLiteral("user_completed")
                        || m_reader->name() == QStringLiteral("user_onhold")
                        || m_reader->name() == QStringLiteral("user_dropped")
                        || m_reader->name() == QStringLiteral("user_plantowatch")
                        || m_reader->name() == QStringLiteral("user_plantoread")) {
                    m_total += m_reader->readElementText().toInt();
                }
            } else if (m_reader->name() == "error") {
                emit writingError(m_reader->readElementText());
                return;
            }
            break;
        case QXmlStreamReader::Characters:
            if (state == State::ID) {
                m_currentId += m_reader->text();
            } else if (state == State::ImgLink) {
                m_currentImgLink += m_reader->text();
            }
        case QXmlStreamReader::EndElement:
            if (m_reader->name().endsWith(QStringLiteral("db_id"))
                || m_reader->name() == QStringLiteral("series_image")) {
                state = State::Nothing;
            }
            if (m_reader->name() == QStringLiteral("anime")
                || m_reader->name() == QStringLiteral("manga")) {
                emit write(m_currentId, m_currentImgLink);
                emit currentProgress(++m_current);
                m_currentId.clear();
                m_currentImgLink.clear();
            }
            if (m_reader->name() == QStringLiteral("myinfo")) {
                m_atUserInfo = false;
                emit totalCount(m_total);
            }
            break;
        case QXmlStreamReader::EndDocument:
            m_reply->deleteLater();
            m_reply = nullptr;
            emit writingFinished();
        default:
            break;
        }
    }
    if (Q_UNLIKELY(m_reader->hasError()
                   && m_reader->error()
                    != QXmlStreamReader::PrematureEndOfDocumentError)) {
        emit writingError(m_reader->errorString());
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}
