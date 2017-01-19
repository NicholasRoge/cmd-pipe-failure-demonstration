#include "Terminal.h"

#include <cassert>
#include <tchar.h>


const tstring PROMPT = L"ATOTALLYRANDOMSTRING";

void Terminal::exec(astring command, OutputCallback callback, bool buffer)
{
	command.append("\r\n");
	
	this->write(command);

	this->readUntilPrompt([&callback, &command] (tstring data) {
		if (!callback) {
			return;
		}

		// Remove the prompt from the string
		data.erase(data.begin(), data.begin() + command.length());

		callback(data);
	}, buffer);
}

Terminal::Terminal()
{
	this->initPipes();
	this->initProcess();
}

Terminal::~Terminal()
{
	this->terminateProcess();
	this->terminatePipes();
}

void Terminal::initPipes()
{
	SECURITY_ATTRIBUTES attr;


	attr.nLength = sizeof(attr);
	attr.bInheritHandle = TRUE;
	attr.lpSecurityDescriptor = nullptr;

	if(!CreatePipe(&this->pipes.read.in, &this->pipes.write.in, &attr, 0))
	{
		throw std::exception("Failed to create stdin pipe.");
	}

	if(!SetHandleInformation(this->pipes.write.in, HANDLE_FLAG_INHERIT, 0))
	{
		throw std::exception("Failed to unset stdin pipe inheritance flag.");
	}

	if(!CreatePipe(&this->pipes.read.out, &this->pipes.write.out, &attr, 0))
	{
		throw std::exception("Failed to create stdout pipe.");
	}

	if(!SetHandleInformation(this->pipes.read.out, HANDLE_FLAG_INHERIT, 0))
	{
		throw std::exception("Failed to unset stdout pipe inheritance flag.");
	}

	if(!CreatePipe(&this->pipes.read.err, &this->pipes.write.err, &attr, 0))
	{
		throw std::exception("Failed to create stderr pipe.");
	}

	if(!SetHandleInformation(this->pipes.read.err, HANDLE_FLAG_INHERIT, 0))
	{
		throw std::exception("Failed to unset stderr pipe inheritance flag.");
	}
}

void Terminal::initProcess()
{
	tstring     command;
	STARTUPINFO startup;


#ifdef UNICODE
	command.append(L"cmd /U /K \"prompt ");
	command.append(PROMPT);
	command.append(L"\"");
#else
	command.append("cmd /A /K \"prompt ");
	command.append(PROMPT);
	command.append("\"");
#endif

	ZeroMemory(&this->process, sizeof(this->process));

	ZeroMemory(&startup, sizeof(startup));
	startup.cb = sizeof(startup);
	startup.dwFlags |= STARTF_USESTDHANDLES;
	startup.hStdInput = this->pipes.read.in;
	startup.hStdOutput = this->pipes.write.out;
	startup.hStdError = this->pipes.write.err;
	startup.dwFlags |= STARTF_USESHOWWINDOW;
	startup.wShowWindow = SW_HIDE;

	auto created = CreateProcess(
		nullptr,
		_tcsdup(command.c_str()),
		nullptr,
		nullptr,
		TRUE,
		0,
		nullptr,
		nullptr,
		&startup,
		&this->process
	);
	if (!created) {
		throw std::exception("Failed to create process.");
	}
}

void Terminal::read(OutputCallback callback)
{
	this->readUntil(L"", callback);
}

void Terminal::readLine(OutputCallback callback, bool buffer)
{
	this->readUntil(L"\n", callback, buffer);
}

void Terminal::readUntil(const tstring& terminator, OutputCallback callback, bool buffer)
{
	auto terminatorIter = terminator.cbegin();
	auto terminatorEnd = terminator.cend();
	auto bufferIter = this->bufferedData.begin();
	auto bufferEnd = this->bufferedData.end();
	do {
		DWORD  bytesAvailable = 0;


		if (!PeekNamedPipe(this->pipes.read.out, nullptr, 0, nullptr, &bytesAvailable, nullptr)) {
			throw std::exception("Failed to peek command input pipe.");
		}

		if (bytesAvailable)
		{
			DWORD  bytesRead;
			tchar* data;
			

			data = new tchar[bytesAvailable / sizeof(tchar)];
			if (!ReadFile(this->pipes.read.out, data, bytesAvailable, &bytesRead, nullptr)) {
				throw std::exception("ReadFile failed.");
			}
			assert(bytesRead == bytesAvailable);

			auto iterDistance = bufferIter - this->bufferedData.begin();
			this->bufferedData.append(data, bytesRead / sizeof(tchar));
			bufferIter = this->bufferedData.begin() + iterDistance;
			bufferEnd = this->bufferedData.end();
		}

		if (terminator.empty()) {
			if (!this->bufferedData.empty())
			{
				bufferIter = bufferEnd;
				terminatorIter = terminatorEnd;
			}
		} else {
			while(bufferIter != bufferEnd && terminatorIter != terminatorEnd) {
				if (*bufferIter == *terminatorIter) {
					++terminatorIter;
				} else {
					terminatorIter = terminator.begin();
				}
				++bufferIter;
			}
		}

		if (!buffer || terminatorIter == terminatorEnd) {
			callback(tstring(this->bufferedData.begin(), bufferIter - terminator.length()));

			this->bufferedData.erase(this->bufferedData.begin(), bufferIter);
		}
	} while (terminatorIter != terminatorEnd);
}

void Terminal::readUntilPrompt(OutputCallback callback, bool buffer)
{
	this->readUntil(PROMPT, callback, buffer);
}

void Terminal::terminatePipes()
{
	if (this->pipes.read.err) {
		CloseHandle(this->pipes.read.err);
	}

	if (this->pipes.write.err) {
		CloseHandle(this->pipes.write.err);
	}

	if (this->pipes.read.out) {
		CloseHandle(this->pipes.read.out);
	}

	if (this->pipes.write.out) {
		CloseHandle(this->pipes.write.out);
	}

	if (this->pipes.read.in) {
		CloseHandle(this->pipes.read.in);
	}

	if (this->pipes.write.in) {
		CloseHandle(this->pipes.write.in);
	}
}

void Terminal::terminateProcess()
{
	if (this->process.hProcess) {
		CloseHandle(this->process.hProcess);
	}
}

void Terminal::write(const astring& data)
{
	DWORD byteCount;
	DWORD bytesWritten;


	byteCount = data.length();

	if (!WriteFile(this->pipes.write.in, data.c_str(), byteCount, &bytesWritten, nullptr)) {
		throw std::exception("WriteFile failed.");
	}
	assert(bytesWritten == byteCount);
}
