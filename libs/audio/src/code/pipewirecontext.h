#pragma once

#include <QObject>

class PipeWireContext : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool pipeWireAvailable READ pipeWireAvailable CONSTANT FINAL)
    Q_PROPERTY(bool wirePlumberAvailable READ wirePlumberAvailable CONSTANT FINAL)
    Q_PROPERTY(QString stack READ stack CONSTANT FINAL)

public:
    explicit PipeWireContext(QObject *parent = nullptr);
    ~PipeWireContext() override;

    bool pipeWireAvailable() const;
    bool wirePlumberAvailable() const;
    QString stack() const;
};
