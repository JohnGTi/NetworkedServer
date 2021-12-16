#pragma once

#include "AddressClient.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <cstdlib>

// Hardcoded scope of play; the default player window size.
constexpr auto WIDTH = 1200;
constexpr auto HEIGHT = 675;
// 
// A more dynamic game window posed opportunity to develop a more robust playing area (game level).
// This functionality was cut from the NetworkedApplication, and left as a note for future development.


class ServerConsole
{
private:
	// The network IP address and port number for the server.
	//
	sf::IpAddress SERVERIP = sf::IpAddress::None;
	unsigned short SERVERPORT = 4444;
	
	// Clock object for monitoring the elapsed game time (server time),
	// and a 'countdown' timer, so that the server closes after a period of inactivity.
	sf::Clock serverClock, activeClock;
	//
	float inactiveLimit;


	// This server's dedicated UDP socket.
	sf::UdpSocket UDPServerSocket;

	// A worker thread for managing incoming packets.
	std::thread* incomingManager;
	//
	// Mutex for protection of resources shared across threads.
	std::mutex memberMutex;


	// Store the local time at which the last outgoing message batch was sent.
	float outgoingTimeStamp;

	// The constant rate at which messages are communicated to a client.
	const float sendRate;

	// Byte sizes of object to send.
	unsigned int appendageBytes, positionalBytes, timeScaleBytes;

	// A map of clients active in the current game.
	std::map<std::string, AddressClient> activeClients;



	// Read address and port data for socket binding and communication.
	void ReadAddressFromFile();

	// The incoming packet manager thread function.
	void IncomingManager();
	
	// Receive data, according to message type.
	void ReceivePositional(sf::Packet packet, AddressClient &client);

	// Randomise a new player's starting position.
	void DeterminePlayerStartPosition(float& positionX, float& positionY);

public:
	ServerConsole();
	~ServerConsole();


	// Bind the dedicated UDP socket to the server port.
	void BindServer();

	// Initiate the blocking receive worker thread.
	void InitiateSelection();

	// Update the server console state, handle all outgoing messages.
	void Update();
};