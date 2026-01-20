#pragma once

#include "Graphics/GPUDevice.h"

class Application
{
public:
    void Startup();
    void Shutdown();

    void Present();
    void Simulate();
    void Resize();

private:
    GPUDevice m_gpuDevice;
};

