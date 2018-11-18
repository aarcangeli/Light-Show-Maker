#include "Application.h"

#ifdef WIN32

#include "Windows.h"

int WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR nCmdLine, int nCmdShow)
#endif
#ifndef WIN32
int main(int argc, char **argv)
#endif

{
    sm::Application app;
    return app.demoWindow();
    return app.runLoop();
}
