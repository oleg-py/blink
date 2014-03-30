#include "blinkconverter.h"
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
#include <QtCore/QXmlStreamReader>
#include <QtNetwork/QNetworkReply>

BlinkConverter::BlinkConverter(QObject *parent) :
    QObject(parent)
{
    m_reader = new QXmlStreamReader();
    m_outputFile = m_outputStream = m_reply = nullptr;
}

bool BlinkConverter::open()
{
    m_outputFile = new QFile(m_outputPath);
    if (!m_outputFile->open(QIODevice::WriteOnly
                            | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "BlinkConverter: Couldn't open file at" << m_outputPath;
        delete m_outputFile;
        return false;
    }
    m_outputStream = new QTextStream(m_outputFile);

    m_format_beforeID = m_outputFormat.section('%', 0, 0);
    m_format_beforeLink = m_outputFormat.section('%', 1, 1);
    m_format_rest = m_outputFormat.section('%', 2);
    qDebug() << "BlinkConverter: opening complete";
    qDebug() << "BlinkConverter: result example: "
             << m_format_beforeID << "XXXXX" << m_format_beforeLink
             << "/000/000/000.jpg" << m_format_rest;
}

void BlinkConverter::close()
{
    delete m_outputStream;
    delete m_outputFile;
    m_outputStream = m_outputFile = nullptr;
    qDebug() << "BlinkWriter: closed";
}

void BlinkConverter::fire(QNetworkReply *reply)
{
    // reset the state
    m_total = m_current = 0;
    m_atUserInfo = false;
    m_reply = reply;
    m_reader->clear();
    m_reader->addData(reply->readAll());
    connect(m_reply, &QNetworkReply::readyRead,
            this, &BlinkConverter::onReplyReadyRead);
}

void BlinkConverter::onReplyReadyRead()
{
    m_reader->addData(m_reply->readAll());
}

void BlinkConverter::write()
{
    *m_outputStream << m_format_beforeID
                    << m_currentId
                    << m_format_beforeLink
                    << m_currentImgLink
                    << m_format_rest
                    << '\n';
}

void BlinkConverter::parseXml()
{
    while (!m_reader->atEnd()) {
        switch(m_reader->readNext()) {
        case QXmlStreamReader::StartElement:
            if (m_reader->name() == "series_animedb_id"
                    || m_reader->name() == "series_mangadb_id") {
                m_currentId = m_reader->readElementText();
            } else if (m_reader->name() == "series_image") {
                m_currentImgLink = m_reader->readElementText();
                write();
                emit currentProgress(++m_current);
            } else if (m_reader->name() == "myinfo") {
                m_atUserInfo = true;
            } else if (m_atUserInfo) {
                // here we rip off the total progress that we need
                if (m_reader->name() == "user_watching"
                        || m_reader->name() == "user_completed"
                        || m_reader->name() == "user_onhold"
                        || m_reader->name() == "user_dropped"
                        || m_reader->name() == "user_plantowatch") {
                    m_total += m_reader->readElementText().toInt();
                }
            }
            break;
        case QXmlStreamReader::EndElement:
            if (m_reader->name() == "myinfo") {
                m_atUserInfo = false;
                emit totalCount(m_total);
            }
            break;
        case QXmlStreamReader::EndDocument:
            m_reply->deleteLater();
            emit writingFinished();
        default:
            break;
        }
    }
    if (m_reader->hasError()
            && m_reader->error() != QXmlStreamReader::PrematureEndOfDocumentError) {
        emit writingAborted(m_reader->errorString());
        m_reply->abort();
        m_reply->deleteLater();
    }
}
