#include "Terminal.h"


int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	Terminal terminal;


	terminal.readUntilPrompt([] (tstring startupInfo) {
		MessageBox(nullptr, startupInfo.c_str(), L"Startup Information", MB_OK);
	});

	// This works
	terminal.exec("dir", [] (tstring result) {
		MessageBox(nullptr, result.c_str(), L"dir", MB_OK);
	});

	// This doesn't (no output)
	terminal.exec("bash -c \"ls\"", [] (tstring result) {
		MessageBox(nullptr, result.c_str(), L"bash -c \"ls\"", MB_OK);
	});

	return 0;
}

