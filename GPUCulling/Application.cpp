#include "stdafx.h"
#include "Application.h"

void Application::Startup()
{
    m_gpuDevice.Initialize();
}

void Application::Shutdown()
{
    m_gpuDevice.Release();
}

void Application::Present()
{
}

void Application::Simulate()
{
}

void Application::Resize()
{
}
