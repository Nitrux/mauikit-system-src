#pragma once

namespace QPipeWireAudio
{
class BackendOperation
{
public:
    explicit BackendOperation(void *operation = nullptr);
    ~BackendOperation();

    BackendOperation &operator=(void *operation);

    bool operator!();
    void *&operator*();
    explicit operator bool();

private:
    void *m_operation = nullptr;
};
}
