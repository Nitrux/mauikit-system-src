#include "operation.h"

namespace QPipeWireAudio
{
BackendOperation::BackendOperation(void *operation)
    : m_operation(operation)
{
}

BackendOperation::~BackendOperation() = default;

BackendOperation &BackendOperation::operator=(void *operation)
{
    m_operation = operation;
    return *this;
}

bool BackendOperation::operator!()
{
    return !m_operation;
}

void *&BackendOperation::operator*()
{
    return m_operation;
}

BackendOperation::operator bool()
{
    return m_operation != nullptr;
}

} // namespace QPipeWireAudio
