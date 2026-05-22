#pragma once

#include <QObject>

namespace QPipeWireAudio
{
class Sink;
class Source;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);

    Sink *defaultSink() const;
    void setDefaultSink(Sink *sink);

    Source *defaultSource() const;
    void setDefaultSource(Source *source);

    void reset();
    bool isPipeWire() const;

Q_SIGNALS:
    void defaultSinkChanged(Sink *sink);
    void defaultSourceChanged(Source *source);
    void updated();

private:
    Sink *m_defaultSink = nullptr;
    Source *m_defaultSource = nullptr;
};
}
