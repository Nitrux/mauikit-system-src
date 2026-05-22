#include "port.h"

namespace QPipeWireAudio
{
Port::Port(QObject *parent)
    : Profile(parent)
{
}

Port::~Port() = default;

} // namespace QPipeWireAudio
