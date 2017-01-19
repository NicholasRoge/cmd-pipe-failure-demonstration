#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <Windows.h>


using wstring = std::wstring;
using astring = std::string;
#ifdef UNICODE
using tstring = wstring;
using tchar = wchar_t;
#else
using tstring = astring;
using tchar = char;
#endif


/**
 * Represents an instance of a terminal process with piped in, out, and err
 * handles.
 */
class Terminal
{
public:
	using OutputCallback = std::function<void(tstring)>;


	/**
	 * Terminal constructor.  
	 */
	Terminal();
	
	/**
	 * Terminal destructor.
	 */
	~Terminal();

	/**
	 * Executes the specified command.  If a callback is specified, the output
	 * be passed as the first argument.
	 * 
	 * @param command
	 * @param callback
	 * @param buffer   If specified, the callback parameter will be called as
	 * output is available until the command is complete.
	 */
	void exec(astring command, OutputCallback callback = nullptr, bool buffer = true);

	/**
	 * Reads from the terminal, calling the specified callback as soon as any
	 * output is available.
	 *
	 * @param callback
	 */
	void read(OutputCallback callback);

	/**
	 * Reads from the terminal, calling the specified callback upon reaching a
	 * newline.
	 *
	 * @param callback
	 * @param buffer   If specified, causes the callback to be called as output
	 * is available until a newline is reached.
	 */
	void readLine(OutputCallback callback, bool buffer = true);

	/**
	 * Reads from the terminal, calling the specified callback upon reaching
	 * the specified terminator.
	 *
	 * @param terminator
	 * @param callback
	 * @param buffer     If specified, causes the callback to be called as
	 * output is available until the specified terminator is reached.
	 */
	void readUntil(const tstring& terminator, OutputCallback callback, bool buffer = true);

	/**
	 * Read from the terminal, calling the specified callback upon reaching the
	 * prompt.
	 *
	 * @param callback
	 * @param buffer   If specified, causes the callback to be called as output
	 * is available until the specified prompt is reached.
	 */
	void readUntilPrompt(OutputCallback callback, bool buffer = true);

	/**
	 * Writes the specified text to the terminal.
	 *
	 * @param data
	 */
	void write(const astring& data);

private:
	struct
	{
		struct
		{
			HANDLE in;
			HANDLE out;
			HANDLE err;
		} read, write;
	} pipes;

	tstring bufferedData;

	PROCESS_INFORMATION process;


	void initPipes();

	void initProcess();

	void terminatePipes();

	void terminateProcess();

};
