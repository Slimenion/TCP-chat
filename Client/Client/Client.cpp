#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4996)
SOCKET Connection;

//создаем пакет
enum Packet {//константы отвечающая за тип пакета
	P_ChatMessage,
	P_PersonalMessage,
	P_Test,
	P_userId,
	P_NewUser
};

bool ProcessPacket(Packet packettype) {//функция отвечающая за обработку пакетов
	switch (packettype) {
	case P_PersonalMessage:
	{
		int msg_size;
		recv(Connection, (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connection, msg, msg_size, NULL);
		std::cout << msg << std::endl;
		delete[] msg;
		break;
	}
	case P_ChatMessage:
	{
		int msg_size;
		recv(Connection, (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connection, msg, msg_size, NULL);
		std::cout << msg << std::endl;
		delete[] msg;
		break;
	}
	case P_Test:
		std::cout << "Test packet.\n";
		break;
	case P_userId:
	{
		int userId_size;
		recv(Connection, (char*)&userId_size, sizeof(int), NULL);
		char* userId = new char[userId_size + 1];
		userId[userId_size] = '\0';
		recv(Connection, userId, userId_size, NULL);
		std::string lol = userId;
		std::string msgID = "my username -> " + lol;

		std::string msg1 = "username: " + msgID;
		int msg_size = msg1.size();
		Packet packettype = P_ChatMessage;//теперь перед отправкой берем тип пакета
		send(Connection, (char*)&packettype, sizeof(Packet), NULL);
		send(Connection, (char*)&msg_size, sizeof(int), NULL);
		send(Connection, msg1.c_str(), msg_size, NULL);
		Sleep(10);

		break;
	}
	default:
		std::cout << "Unrecognized packet: " << packettype << std::endl;
		break;
	}

	return true;
}

void ClientHandler() {//функция по распознаванию пакета
	Packet packettype;
	while (true) {
		recv(Connection, (char*)&packettype, sizeof(Packet), NULL);

		if (!ProcessPacket(packettype)) {
			break;
		}
	}
	closesocket(Connection);
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

	//заполняем информацию об адрессе сокет
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	//ip адресс
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	//сокет
	addr.sin_port = htons(2252);
	addr.sin_family = AF_INET;

	//делаем ник для пользователя
	std::cout << "Enter your username" << std::endl;
	std::string username;
	std::cin >> username;


	//сокет для соединения с сервером
	Connection = socket(AF_INET, SOCK_STREAM, NULL);
	//проверка подключены ли мы к серверу
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		std::cout << "Error: failed connect to server.\n";
		return 1;
	}
	std::cout << "Succesful connection to the server!\n";



	//запуск функции на чтение с сервера в новом потоке
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

	Packet packettype = P_NewUser;
	int username_size = username.size();
	send(Connection, (char*)&packettype, sizeof(Packet), NULL);
	send(Connection, (char*)&username_size, sizeof(int), NULL);
	send(Connection, username.c_str(), username_size, NULL);
	//отправим сообщение о том что мы присоеденились
	std::string msg1;
	msg1 = username + " has joined the server!";
	int msg_size = msg1.size();
	packettype = P_ChatMessage;//теперь перед отправкой берем тип пакета
	send(Connection, (char*)&packettype, sizeof(Packet), NULL);
	send(Connection, (char*)&msg_size, sizeof(int), NULL);
	send(Connection, msg1.c_str(), msg_size, NULL);
	Sleep(10);

	//блок с отправлениием сообщения
	std::cout << "Serever: To write a private message, write /send\n";
	while (true) {


		std::getline(std::cin, msg1);
		if (msg1 != "/exit") {

			if (msg1 == "")
			{
				Sleep(10);
			}
			else {
				if (msg1 == "/send") {
					std::cout << "Enter the username wich you want to send the message\n";
					std::string sendUsername;
					std::cin >> sendUsername;
					std::cout << "Enter the message: ";
					std::string personalMessage;
					std::getline(std::cin, msg1);
					std::getline(std::cin, msg1);
					msg1 = "User ->" + username + "<- send you a private message: " + msg1;
					int msg_size = msg1.size();
					int indexOfUser_size = sendUsername.size();
					Packet packettype = P_PersonalMessage;//теперь перед отправкой берем тип пакета
					send(Connection, (char*)&packettype, sizeof(Packet), NULL);
					send(Connection, (char*)&msg_size, sizeof(int), NULL);
					send(Connection, msg1.c_str(), msg_size, NULL);
					send(Connection, (char*)&indexOfUser_size, sizeof(int), NULL);
					send(Connection, sendUsername.c_str(), indexOfUser_size, NULL);
					Sleep(10);

				}
				else {


					msg1 = username + ": " + msg1;
					int msg_size = msg1.size();
					Packet packettype = P_ChatMessage;//теперь перед отправкой берем тип пакета
					send(Connection, (char*)&packettype, sizeof(Packet), NULL);
					send(Connection, (char*)&msg_size, sizeof(int), NULL);
					send(Connection, msg1.c_str(), msg_size, NULL);
					Sleep(10);

				}
			}
		}
		else {
			std::cout << "Exit on Server" << std::endl;
			int msg_size = msg1.size();
			Packet packettype = P_ChatMessage;//теперь перед отправкой берем тип пакета
			send(Connection, (char*)&packettype, sizeof(Packet), NULL);
			send(Connection, (char*)&msg_size, sizeof(int), NULL);
			send(Connection, msg1.c_str(), msg_size, NULL);
			Sleep(10);
			exit(1);
		}
	}

	system("pause");
	return 0;
}