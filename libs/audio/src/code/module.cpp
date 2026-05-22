#include "module.h"

namespace QPipeWireAudio
{
Module::Module(QObject *parent)
    : AudioObject(parent)
{
}

QString Module::name() const
{
    return m_name;
}

void Module::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    Q_EMIT nameChanged();
}

QString Module::argument() const
{
    return m_argument;
}

void Module::setArgument(const QString &argument)
{
    if (m_argument == argument) {
        return;
    }
    m_argument = argument;
    Q_EMIT argumentChanged();
}

} // namespace QPipeWireAudio
