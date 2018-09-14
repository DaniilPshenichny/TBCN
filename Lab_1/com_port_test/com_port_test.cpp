#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>
using namespace std;


class COM {

private:

	HANDLE hVirtualCOM;
	LPCTSTR sPortName = L"COM1";
	DCB ComDCM;
	int TIMEOUT = 200;
	int baudrate = CBR_9600;

public:

	COM()
	{
		memset(&ComDCM, 0, sizeof(ComDCM));
		ComDCM.DCBlength = sizeof(DCB);
	}

	HANDLE COM::getHandle() {
		return hVirtualCOM;
	}

	void COM::disconnect()
	{
		if (hVirtualCOM != 0) {
			CloseHandle(hVirtualCOM);
			hVirtualCOM = 0;
		}
	}

	void COM::Init()
	{
		hVirtualCOM = CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hVirtualCOM == INVALID_HANDLE_VALUE) {
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				cout << "com-port does't exist\n";
			cout << "Error\n";
		}


		//устанавливаем размеры входного и выходного буферов
		SetupComm(hVirtualCOM, 1500, 1500);
		//настраиваем тайм-ауты для текущего устройства
		COMMTIMEOUTS CommTimeOuts;
		CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
		CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
		CommTimeOuts.ReadTotalTimeoutConstant = TIMEOUT;
		CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
		CommTimeOuts.WriteTotalTimeoutConstant = TIMEOUT;
		if (!SetCommTimeouts(hVirtualCOM, &CommTimeOuts)) {
			hVirtualCOM = 0;
			exit(0);
		}

		//настраиваем параметры работы для текущего устройства
		GetCommState(hVirtualCOM, &ComDCM); // текущие значения
		ComDCM.BaudRate = DWORD(baudrate); // скорость в бодах
		ComDCM.ByteSize = 8; // размер байта
		ComDCM.Parity = NOPARITY; // паритет
		ComDCM.StopBits = ONESTOPBIT; // количество стоп бит
		ComDCM.fAbortOnError = TRUE;
		ComDCM.fDtrControl = DTR_CONTROL_DISABLE; // сброс DTR бита

												  // автоустановка RTS бита
		ComDCM.fRtsControl = RTS_CONTROL_TOGGLE;
		ComDCM.fBinary = TRUE; //бинарный режим всегда
		ComDCM.fParity = FALSE; //паритет
		ComDCM.fInX = ComDCM.fOutX = FALSE;
		ComDCM.XonChar = 0;
		ComDCM.XoffChar = uint8_t(0xff);
		ComDCM.fErrorChar = FALSE;
		ComDCM.fNull = FALSE;
		ComDCM.fOutxCtsFlow = FALSE;
		ComDCM.fOutxDsrFlow = FALSE;
		ComDCM.XonLim = 128;
		ComDCM.XoffLim = 128;
		//установка параметров работы для текущего устройства
		if (!SetCommState(hVirtualCOM, &ComDCM)) {
			CloseHandle(hVirtualCOM);
			hVirtualCOM = 0;
			exit(0);
		}
	}

	void COM::Write(const char* data, int datasize) {
		if (hVirtualCOM == 0)
			exit(0);
		DWORD feedback;
		if (!WriteFile(hVirtualCOM, data, datasize, &feedback, 0) || feedback != datasize) {
			CloseHandle(hVirtualCOM);
			hVirtualCOM = 0;
			cout << "Error in WRITE";
			system("pause");
			exit(0);
		}
		cout << "DataSize = " << datasize << " BytesWritten = " << feedback << endl;
	}

	int COM::Read(char* data, int bufsize) {
		int full = bufsize;
		DWORD feedback = 0;
		int attempts = 3; //количество запросов на чтение
		while (bufsize && attempts) {
			if (attempts)
				attempts--;
			if (!ReadFile(hVirtualCOM, data, bufsize, &feedback, NULL)) {
				CloseHandle(hVirtualCOM);
				hVirtualCOM = 0;
				exit(0);
			}
			bufsize -= feedback;
			data += feedback;
		}

		DWORD temp;
		COMSTAT comstat;
		ClearCommError(getHandle(), &temp, &comstat);

		if (comstat.cbInQue > 0)
			cout << "Read successfully. Byte in que" << comstat.cbInQue;
		else
			cout << "Read successfully.";
		return full - bufsize;
	}

	

	
	
	int COM::speed(int speed) {
		baudrate = speed;
		ComDCM.BaudRate = DWORD(baudrate); // скорость в бодах

		//установка параметров работы для текущего устройства
		if (!SetCommState(hVirtualCOM, &ComDCM)) {
			CloseHandle(hVirtualCOM);
			hVirtualCOM = 0;
			exit(0);
		}

	}
	

	int COM::CurrentSpeed() {
		return ComDCM.BaudRate;
	}

	void COM::Setport(const wchar_t buffer[3]) {
		sPortName = buffer;
	}
};


int main(int argc, _TCHAR* argv[])

{
	string str;
	COM tempclass;


	char temp;
	cin >> temp;
	switch (temp) {
	case '1':
		tempclass.Init();
		break;
	case '2':
		tempclass.Setport(L"COM2");
		tempclass.Init();
		break;
	default:
		tempclass.Init();
	}

	cout << "1 - Write" << endl;
	cout << "2 - Read" << endl;
	cout << "3 - Cur. speed" << endl;
	cout << "4 -  100" << endl;
	cout << "5 -  9600" << endl;
	cout << "6 -  256000" << endl;

	while (1)
	{
		char temp;
		cin >> temp;
		switch (temp) {
		case '1': {
			cout << "Write messsage" << endl;
			cin.get();
			getline(cin, str);
			const int lenght = str.length();
			char* data = new char[lenght];
			strcpy(data, str.c_str());
			tempclass.Write(data, lenght);
			break;
		}
		case '2': {
			char data1[10];
			int readByte;
			readByte = tempclass.Read(data1, sizeof(data1));
			cout << " BytesRead = " << readByte << endl;
			for (int i = 0; i < readByte; i++)
				cout << data1[i];
			cout << endl;
			break;
		}
		case '3':
			cout << "Current Speed = " << tempclass.CurrentSpeed() << endl;
			break;
		case '4':
			tempclass.speed(100);
			break;
		case '5':
			tempclass.speed(9600);
			break;
		case '6':
			tempclass.speed(25600);
			break;
		default:
			return(1);
		}
	}


	return 0;
}
