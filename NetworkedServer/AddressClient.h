#pragma once

#include <SFML/Network.hpp>

#include "MessageTypes.h"

enum class NetworkStability {
	Unstable
	, Poor
	, Good
};


struct PositionalPair {
	//
	PositionalData positionalData;
	float sentTimeStamp;

	// Constructor overide.
	PositionalPair(PositionalData positional, float time)
		: positionalData(positional) , sentTimeStamp(time) {}
};


class AddressClient
{
private:
	// House an individual client's address and port number, for
	// addressing and identification.

	sf::IpAddress activeClientIP;
	unsigned short activeClientPort;

	// A packet of accumulated, outgoing messages to send. A previous iteration saw a std::vector
	// of sf::Packet type, but a single packet facilitates the convenient grouping of for a single send.
	sf::Packet outgoingMessage;

	// A simple determination of stability over the network, relating to a particular client.
	NetworkStability networkStability;
	bool networkInstabilityFlag;

	// The constant rate at which messages are communicated client-side.
	const float sendRate;

	// Retain a history of received positional data.
	std::vector<PositionalPair> positionalHistory;

	// The depth for which historical data is stored.
	const int historicalBounds;

public:
	// Declare the override constructors.
	AddressClient(sf::IpAddress address, unsigned short port);
	AddressClient();


	// Receive and set functions for client addressing and port.
	sf::IpAddress& ActiveClientIP() { return activeClientIP; }
	unsigned short& ActiveClientPort() { return activeClientPort; }

	// Determine that the received data is neither old nor a duplicate, for processing.
	bool ReceievedIsCurrent(const PositionalData &receivedPositional, float sentTimeStamp);

	// Update a reference point of the communication consitions between this client.
	void NetworkStabilityAssessment();
	void ReaffirmNetworkStability();

	// Access the outgoing packet, for insertion; extraction; to send.
	sf::Packet* GetOutgoingMessage() { return &outgoingMessage; }

	// Return a pointer to the historical collection of received positional data.
	std::vector<PositionalPair>* GetPositionalHistory() { return &positionalHistory; }


	// Operator override for the comparison of the AddressClient object.
	bool operator ==(const AddressClient &receivedClient) const;
	bool operator !=(const AddressClient& receivedClient) const;
};