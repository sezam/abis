#ifdef _WIN32
#include <winsock2.h>
#define HAVE_STRUCT_TIMESPEC
#else
#include <arpa/inet.h>
#define SOCKET int
#endif //  WIN32

#include <pthread.h> 
#include <semaphore.h> 
#include <stdio.h>
#include "AbisRest.h"
#include "ebsclient.h"

int loglevel = debug;

sem_t mkey1;
sem_t mx_logger;
sem_t mkey3;
sem_t mx_socket;
sem_t mx_ports[4];

FILE *logFile;

char* getLogName(int log_level)
{
	if (log_level == error) return (char*)"[ERROR] ";
	if (log_level == user) return (char*)"[USER] ";
	if (log_level == info) return (char*)"[INFO] ";
	if (log_level == debug) return (char*)"[DEBUG] ";
	return (char*)" ";
}

void getTime(char *sTime)
{
	for (int i = 0; i < 64; i++) sTime[i] = 0;
	time_t t = time(NULL);
	struct tm *ltm;
	ltm = localtime(&t);
	char *sTime1 = asctime(ltm);
	for (int i = 0; i < strlen(sTime1); i++) *(sTime + i) = *(sTime1 + i);
	for (int i = 0; i < strlen(sTime1); i++) if (*(sTime + i) == 0x0A)
	{
		*(sTime + i) = 0x0D;
		*(sTime + i + 1) = 0x0A;
	}
}

void saveToLog(int elev, char *log, char *data)
{
	if (elev <= loglevel)
	{
		sem_wait(&mx_logger);
		char *tstr = new char[64];
		getTime(tstr);
		*(tstr + strlen(tstr) - 2) = 0;
		logFile = fopen("../log/gzface.log", "a");
		if (logFile)
		{
			fprintf(logFile, "%s\t%s\t%s %s\n", getLogName(elev), tstr, log, data);
			fclose(logFile);
		}
		delete[] tstr;
		sem_post(&mx_logger);
	}
}

int getPortIndex()
{
	int pNumber = -1;
	sem_wait(&mx_socket);

	do
	{
		for (size_t i = 0; i < 4; i++)
		{
			int value;
			sem_getvalue(&mx_ports[i], &value);

			if (value == 0) {
				pNumber = i;
				break;
			}
		}
		if (pNumber < 0) this_thread::sleep_for(milliseconds(100));
	} while (pNumber < 0);

	sem_post(&mx_socket);
	return pNumber;

}

int get_face_template(const unsigned char *data, unsigned int dataLen, float *fTemplate)
{
	int rs = 0;
	SOCKET myClientSocket;
	sockaddr_in addr;

	int port_index = getPortIndex();
	int current_port = port_index + 10080;

	sem_wait(&mx_ports[port_index]);
	cout << "Mutex lock: " << &mx_ports[port_index] << " port index: " << port_index << endl;

	if ((myClientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		saveToLog(error, (char*)"Error create socket\n", "");
		return 0;
	}


	char ss[50];
	sprintf(ss, "%d", current_port);
	saveToLog(debug, (char*)"Extract from port ", ss);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(current_port);
	//addr1.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_addr.s_addr = inet_addr("10.6.46.147");
	if (connect(myClientSocket, (sockaddr *)&addr, sizeof(addr)))
	{
		saveToLog(error, (char*)"Error connect\n", "");

#ifdef  _WIN32
		closesocket(myClientSocket);
#else
		close(myClientSocket);
#endif //  _WIN32
		sem_post(&mx_ports[port_index]);
		cout << "Mutex free: " << &mx_ports[port_index] << " port index: " << port_index << endl;

		return 0;
	}

	char header[8];
	header[0] = 'r';
	header[1] = 'e';
	header[2] = 'q';
	header[3] = 'f';
	header[7] = (unsigned char)(dataLen & 0xFF);
	header[6] = (unsigned char)((dataLen >> 8) & 0xFF);
	header[5] = (unsigned char)((dataLen >> 16) & 0xFF);
	header[4] = (unsigned char)((dataLen >> 24) & 0xFF);

	send(myClientSocket, header, 0x08, 0);

	send(myClientSocket, (char*)data, dataLen, 0);

	char pData[65536];
	int dl = recv(myClientSocket, pData, 0x08, 0);

	if (pData[0] == 'a' && pData[1] == 'n' && pData[2] == 's' && pData[3] == 'w')
	{
		int dataLen1 = 0;
		unsigned int p1 = (unsigned int)pData[4];
		unsigned int p2 = (unsigned int)pData[5];
		unsigned int p3 = (unsigned int)pData[6];
		unsigned int p4 = (unsigned int)pData[7];
		dataLen1 = p1 * 16777216 + p2 * 65536 + p3 * 256 + p4;

		char *data = new char[dataLen1];
		if (data)
		{
			dl = recv(myClientSocket, data, dataLen1, 0);
			if (dl == 4)
			{
				delete[] data;
				data = NULL;
#ifdef  _WIN32
				closesocket(myClientSocket);
#else
				close(myClientSocket);
#endif //  _WIN32
				sem_post(&mx_ports[port_index]);
				cout << "Mutex free: " << &mx_ports[port_index] << " port index: " << port_index << endl;

				return 0;
			}
			if (dl == dataLen1)
			{
				rs = data[0];
				unsigned char *p;
				p = (unsigned char*)fTemplate;
				for (int i = 0; i < sizeof(float)*FACESIZE; i++) p[i] = data[i + 1];
			}

			delete[] data;
			data = NULL;
			}
		}

#ifdef  _WIN32
	closesocket(myClientSocket);
#else
	close(myClientSocket);
#endif //  _WIN32
	sem_post(&mx_ports[port_index]);
	cout << "Mutex free: " << &mx_ports[port_index] << " port index: " << port_index << endl;

	return rs;
}

void ebs_client_init()
{
	int res = sem_init(&mkey1, 1, 0);
	res = sem_init(&mx_logger, 1, 0);
	res = sem_init(&mkey3, 1, 0);
	res = sem_init(&mx_socket, 1, 0);

	for (size_t i = 0; i < 4; i++)
	{
		res = sem_init(&mx_ports[i], 1, 0);
	}
}

void ebs_client_done()
{
	for (size_t i = 0; i < 4; i++)
	{
		sem_destroy(&mx_ports[i]);
	}

	sem_destroy(&mx_socket);
	sem_destroy(&mkey3);
	sem_destroy(&mx_logger);
	sem_destroy(&mkey1);
}