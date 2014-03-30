#include "blinkwriter.h"
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>

BlinkWriter::BlinkWriter(QObject *parent) :
    QObject(parent)
{
    m_outputFile = nullptr;
    m_outputStream = nullptr;
}

BlinkWriter::~BlinkWriter()
{
    delete m_outputFile;
    delete m_outputStream;
}

bool BlinkWriter::open()
{
    m_outputFile = new QFile(m_outputPath);
    if (!m_outputFile->open(QIODevice::WriteOnly
                            | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "BlinkWriter: Couldn't open file at" << m_outputPath;
        delete m_outputFile;
        m_outputFile = nullptr;
        return false;
    }
    m_outputStream = new QTextStream(m_outputFile);

    m_format_beforeID = m_outputFormat.section('~', 0, 0);
    m_format_beforeLink = m_outputFormat.section('~', 1, 1);
    m_format_rest = m_outputFormat.section('~', 2);
    qDebug() << "BlinkWriter: opening complete";
    return true;
}

void BlinkWriter::close()
{
    delete m_outputStream;
    delete m_outputFile;
    m_outputStream = nullptr;
    m_outputFile = nullptr;
    qDebug() << "BlinkWriter: closed";
}


void BlinkWriter::write(const QString& id, const QString& link)
{
    *m_outputStream << m_format_beforeID
                    << id
                    << m_format_beforeLink
                    << link
                    << m_format_rest
                    << '\n';
}
