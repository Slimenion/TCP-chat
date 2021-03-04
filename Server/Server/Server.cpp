#include "stdafx.h"
#include "vcruntime_exception.h"
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string> 

#pragma warning(disable: 4996)

//пременные для соединений
SOCKET Connections[100];
int Counter = 0;

struct Users {
	int id = -1;
	std::string name;
};

Users users[100];

bool checkUser(std::string username) {
	for (int i = 0; i < 100; i++) {
		if (users[i].name == username) return false;
	}
	return true;
}


enum Packet {//константы отвечающие за тип пакета
	P_ChatMessage,
	P_PersonalMessage,
	P_Test,
	P_userId,
	P_NewUser
};

bool ProcessPacket(int index, Packet packettype) {//функиця обработки пакетов
	switch (packettype) {
	case P_NewUser:
	{
		int username_size;
		recv(Connections[index], (char*)&username_size, sizeof(int), NULL);
		char* username = new char[username_size + 1];

		username[username_size] = '\0';
		recv(Connections[index], username, username_size, NULL);
		std::string str_username = std::string(username);
		if (checkUser(str_username)) {
			for (int i = 0; i < 100; i++) {
				if (users[i].id == -1) {
					users[i].id = i;
					users[i].name = str_username;
					break;
				}
			}
		}

		break;

	}
	case P_PersonalMessage:
	{

		int msg_size;
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];

		msg[msg_size] = '\0';
		recv(Connections[index], msg, msg_size, NULL);


		int indexOfUser_size;
		recv(Connections[index], (char*)&indexOfUser_size, sizeof(int), NULL);
		char* indexOfUser = new char[indexOfUser_size + 1];
		recv(Connections[index], indexOfUser, indexOfUser_size, NULL);
		int i = 1001;
		std::string new_index = std::string(indexOfUser, indexOfUser_size);
		for (int k = 0; k < 100; k++) {
			if (new_index == users[k].name) {
				i = k;
			}
		}

		Packet msgtype = P_PersonalMessage;
		send(Connections[i], (char*)&msgtype, sizeof(Packet), NULL);
		send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[i], msg, msg_size, NULL);
		delete[] msg;
		break;

	}

	case P_ChatMessage:
	{
		int msg_size = 0;
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];

		msg[msg_size] = '\0';
		recv(Connections[index], msg, msg_size, NULL);
		std::string teamCheck;
		teamCheck = msg;
		if (teamCheck != "/exit") {
			if (teamCheck != "") {
				for (int i = 0; i < Counter; i++) {
					if (i == index) {
						continue;
					}


					Packet msgtype = P_ChatMessage;
					send(Connections[i], (char*)&msgtype, sizeof(Packet), NULL);
					send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
					send(Connections[i], msg, msg_size, NULL);

				}
				delete[] msg;
				break;
			}
		}
		else {
			std::cout << "Client has disconnect\n";
			while (true) {
				if (shutdown(Connections[index], 2) == 0) {
					std::cout << "Successful\n";
					break;
				}

			}
		}



	}
	default:
		if (packettype == 0) {
			shutdown(Connections[index], 2);
		}
		else {
			std::cout << "Unrecognized packet: " << packettype << std::endl;
		}
		break;
	}

	return true;
}

void ClientHandler(int index) {	//функция принимающая соединение в сокет массиве
	Packet packettype;
	while (true) {
		recv(Connections[index], (char*)&packettype, sizeof(Packet), NULL);

		if (!ProcessPacket(index, packettype)) {
			break;
		}
	}
	closesocket(Connections[index]);
}

int main(int argc, char* argv[]) {
	std::cout << "Enter ip host from hamachi\n";
	std::string ip;
	std::cin >> ip;

	//загружаем библиотеки
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	//проверка на загрузку библиотеки
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	//заполняем информацию об адрессе сокета
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	//ip адресс
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	//сокет
	addr.sin_port = htons(2252);
	addr.sin_family = AF_INET;

	//прослушиваем на прием данных с сокета
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	//привязка адресса сокету
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	//слушаем
	listen(sListen, SOMAXCONN);

	//сокет для клиента, теперь addr содержит ip клиента
	SOCKET newConnection;
	std::string Username[100];
	for (int i = 0; i < 100; i++) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		//проверка на то подключен ли пользователь
		if (newConnection == 0) {
			std::cout << "Error #2\n";
		}
		else {
			std::cout << "Client Connected!\n Client id: " << i << "\n";



			std::string msg;
			int msg_size = msg.size();
			Packet msgtype = P_ChatMessage;

			//записываем соединение в массив
			Username[i] = std::to_string(i);
			Connections[i] = newConnection;
			Counter++;
			//включаем работу 2 потоков,чтобы в функции main принимались новые соеденения , а в процедуре ClientHandler будут ожидаться и отправлятся сообщения клиентам
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);
		}
	}


	system("pause");
	return 0;
}