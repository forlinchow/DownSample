#pragma once
#include <cctype>
#include <windows.h>
#include <iostream>
#include<io.h>
#include <vector>
using std::string;
using std::vector;

namespace forlin {
	// see https://stackoverflow.com/questions/23943728/case-insensitive-standard-string-comparison-in-c
	bool icompare_pred(unsigned char a, unsigned char b);

	// see https://stackoverflow.com/questions/23943728/case-insensitive-standard-string-comparison-in-c

	bool icompare(string const& a, string const& b);
	bool iEndsWith(const string &str, const string &suffix);


	//��ȡ�ļ���С -bytes
	inline LARGE_INTEGER _getFileSize(std::string filename) {
		//std::cout << "calculate file size" << std::endl;
		std::wstring stemp = std::wstring(filename.begin(), filename.end());
		LPCWSTR sw = stemp.c_str();
		HANDLE hFile = CreateFileW(sw, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		LARGE_INTEGER FileSize;
		GetFileSizeEx(hFile, &FileSize);
		CloseHandle(hFile);
		//std::cout << "finish calculation,size:" <<FileSize.QuadPart<< std::endl;
		return FileSize;
	}

	//��ȡ���ú����� real-2
	inline int _getNumOfProcessors() {
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return info.dwNumberOfProcessors-2;
	}
}