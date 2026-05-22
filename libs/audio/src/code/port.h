#pragma once

#include "profile.h"

namespace QPipeWireAudio
{
class Port : public Profile
{
    Q_OBJECT
public:
    explicit Port(QObject *parent = nullptr);
    ~Port() override;
};
}
