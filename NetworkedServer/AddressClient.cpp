#include "AddressClient.h"


AddressClient::AddressClient(sf::IpAddress address, unsigned short port)
	: activeClientIP(address), activeClientPort(port)

	, networkStability(NetworkStability::Good)
	, networkInstabilityFlag(false)

	, sendRate(0.1f)
	, historicalBounds(6)
{}

AddressClient::AddressClient()
	: activeClientIP("sf::IpAddress::None4444"), activeClientPort(4444)

	, networkStability(NetworkStability::Good)
	, networkInstabilityFlag(false)

	, sendRate(0.1f)
	, historicalBounds(6)
{}


bool AddressClient::ReceievedIsCurrent(const PositionalData &receivedPositional, float sentTimeStamp)
{
	// Verify that there is a history necessary to compare against.
	if (!positionalHistory.empty()) {

		// Test that the received message was sent at a time at least a period of the client send rate
		// further than the most up-to-date historical message.
		if (sentTimeStamp > positionalHistory.back().sentTimeStamp + sendRate) {
			
			// As the test was successful, this data can be considered up-to-date.
			positionalHistory.push_back(PositionalPair(receivedPositional, sentTimeStamp));


			// Historical data outside the inactivity bounds is discarded. The last N historical messages
			// required is determined by the period that the server might be required to time travel.

			if (positionalHistory.size() > historicalBounds) {
				// Iterate through and erase all elements prior to the back N.
				//
				for (auto it = positionalHistory.begin(); positionalHistory.size() != historicalBounds;) {

					it = positionalHistory.erase(it);
				}
			}
		}
		else {

			// Here, the data might be antiquated (more up-to-date data exists) or a duplicate (the
			// timestamp is not at least a send rate period of difference from the historical data).

			// The send rate difference provides a correct window for which a piece of data can be
			// considered equal, forgiving any floating point imprecision that would come with an
			// equality comparison (==) of the respective timestamps.

			return false;
		}
	}
	else
	{
		// There is no comparison to make between historical data. This data is essential.
		positionalHistory.push_back(PositionalPair(receivedPositional, sentTimeStamp));
	}
	return true;
}


void AddressClient::NetworkStabilityAssessment()
{
	// The network assessment functionality is honestly imperfect as a function of forethought (and time).
	// Below is an example of a system lacking somewhat in economy. A critical note on the switch resolution
	// is noted in the NetworkedApplication's 'ClientConsole::Update(float deltaTime)' send commentary.

	if (networkInstabilityFlag && networkStability != NetworkStability::Unstable) {

		switch (networkStability) {
		case (NetworkStability::Good): networkStability = NetworkStability::Poor; break;
		case (NetworkStability::Poor): networkStability = NetworkStability::Unstable; break;
		};
		networkInstabilityFlag = false;
	}
	else if (networkInstabilityFlag && networkStability == NetworkStability::Unstable){
		// Kill Client.
	}
	else { networkInstabilityFlag = true; }
}

void AddressClient::ReaffirmNetworkStability()
{
	// The severity of the assessment of network stability has not been properly actualised.
	// The assessment of a strong connection on a single successful send may be generous.

	if (networkStability != NetworkStability::Good || !networkInstabilityFlag) {
		//
		networkStability = NetworkStability::Good;
		networkInstabilityFlag = false;
	}
}


bool AddressClient::operator==(const AddressClient& receivedClient) const
{
	// The two AddressClient instances refer to the same client if both the IP address
	// and port number are the same.

	return (activeClientIP == receivedClient.activeClientIP &&
		activeClientPort == receivedClient.activeClientPort);
}

bool AddressClient::operator!=(const AddressClient& receivedClient) const
{
	// The two AddressClient instances do not refer to the same client if either the
	// IP address or port number are not the same.

	return (activeClientIP != receivedClient.activeClientIP ||
		activeClientPort != receivedClient.activeClientPort);
}