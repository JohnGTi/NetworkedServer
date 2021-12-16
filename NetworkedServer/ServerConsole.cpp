#include "ServerConsole.h"

#include <string>


ServerConsole::ServerConsole()
    : incomingManager(NULL)

    , inactiveLimit(120.f)
    , outgoingTimeStamp(0.f)
    , sendRate(0.1f)

    , appendageBytes(sizeof(sf::Uint8) + sizeof(float))
    , positionalBytes(sizeof(PositionalData))
    , timeScaleBytes(sizeof(TimeScaleData))
{}

ServerConsole::~ServerConsole()
{
    // Wait for the worker thread to finish, and destroy its private reference.
    incomingManager->join();
    delete incomingManager;
}



void ServerConsole::ReadAddressFromFile()
{
    // Specifiy the file to read from, and the file access mode 'r.'
    FILE* addressFile;
    fopen_s(&addressFile, "Readable/AddressandPortData.txt", "r");

    // If the file failed to load, communicate the error and signal an abort.
    if (addressFile == NULL) {
        //
        fputs("Error opening server/client address file.\n", stderr); abort();
    }

    // A string variable to construct/assign to a sf::IpAddress.
    std::string serverIP;
    //
    // Perform fscanf and extrapolate the required contents of the address data text file.
    fscanf_s(addressFile, "The server network IP address and port, respectively.%*c");
    fscanf_s(addressFile, "%16s %hu%*c", serverIP.c_str(), 16, &SERVERPORT);

    // Assign a string representation of an IP, to be parsed.
    SERVERIP = serverIP;
}

void ServerConsole::BindServer()
{
    // Read in server ip address and port number from a directory text file.
    ReadAddressFromFile();

    // Bind the dedicated UDP socket to the server port.
    // //
    if (UDPServerSocket.bind(SERVERPORT) == sf::UdpSocket::Done)
    {
        // Communicate binding success to console.
        printf("Successfully bound to server port %d at address %s.\n",
            UDPServerSocket.getLocalPort(), SERVERIP.toString().c_str());
    }
    else {
        // Communicate binding failure to console.
        printf("Failed to bind the socket to the dedicated server port.\n");

        // Attempt to bind the socket to any port.
        while (UDPServerSocket.bind(sf::UdpSocket::AnyPort) != sf::UdpSocket::Done) {
            //
            printf("Failed to bind to an available port.\n");
        }

        // The loop of binding failure has been escaped.
        printf("Successfully bound to server port %d at address %s.\n",
            UDPServerSocket.getLocalPort(), SERVERIP.toString().c_str());
    }

    // Truly begin the clock, after server socket binding.
    serverClock.restart();
    //
    // The server facilitates a continuous game level, where clients may
    // join and leave at their convenience.
}



void ServerConsole::Update()
{
    // Limit the rate at which outgoing messages are sent to an individual client.
    //
    if (serverClock.getElapsedTime().asSeconds() >= (outgoingTimeStamp + sendRate)) {

        // Iterate through the active clients container and deliver the contents of their outgoing storage.
        for (auto recipient = activeClients.begin(); recipient != activeClients.end();) {

            // Assign local access to the positional history container.

            // Determine a client to be idle, if there has been no contact after a period of two seconds.
            if (recipient->second.GetPositionalHistory()->back().sentTimeStamp < serverClock.getElapsedTime().asSeconds() - 2.f) {
                
                // As opposed to an immediate sever, further development might consider delivering a closing message, so that
                //  a client might more promptly and handily transition to a state in which they can reach back out to the server.

                recipient = activeClients.erase(recipient);
                continue;
            }

            // Assign the outgoing message to a local pointer.
            sf::Packet* sendPacket = recipient->second.GetOutgoingMessage();
            
            // Verify whether or not there are messages to send.
            if (sendPacket->getData() != NULL)
            {
                for (unsigned int attempts = 0; attempts < 3; attempts++) {

                        if (UDPServerSocket.send(*sendPacket, recipient->second.ActiveClientIP(), recipient->second.ActiveClientPort()) != sf::UdpSocket::Done) {

                            // Upon a third consequetive unsuccessful attempt, flag concern over the quality of communication.
                            if (attempts == 2) {
                                //
                                // An unstable connection will be recognised later in the application.
                                recipient->second.NetworkStabilityAssessment();
                            }

                            // Where packet loss occurs (a failure to send) in the case of an unknown client, the client is
                            // left to their own devices, in a sense. The client is not added to the map of activeClients,
                            // and has the opportunity to attempt to establish their join in the future.
                        }
                        else {
                            // A successful send re-establishes the house understanding of network stability.
                            recipient->second.ReaffirmNetworkStability(); break;
                        }
                }
                // After processing, the message backlog can be cleared.
                sendPacket->clear();
            }
            ++recipient;
        }



        // The server has completed handling a batch of outgoing messages, and so the current
        // time is stamped for the reference of the send frequency validation.
        outgoingTimeStamp = serverClock.getElapsedTime().asSeconds();
    }
}



void ServerConsole::DeterminePlayerStartPosition(float &positionX, float &positionY)
{
    do {
        // Randomly determine a player's start position by screen space.
        positionX = rand() % WIDTH + 1;
        positionY = rand() % HEIGHT + 1;

        // Loop until the newly determined position does not conflict with another player's.
        // Collision detection functionality - ran out of time.
    } while (false);
}


void ServerConsole::ReceivePositional(sf::Packet packet, AddressClient &client)
{
    // Protect class member variables by mutex lock within current scope.
    std::unique_lock<std::mutex> lock(memberMutex);

    // A local positional type instance, for extracting the positional data.
    PositionalData receivedPositional;
    float sentTimeStamp;


    // Test the extraction operation for the boolean returned, so as to protect against
    // the packet failing.

    if (packet >> receivedPositional >> sentTimeStamp)
    {
        // Construct a client ID; for the identification and reference of a known client.
        std::string determinedID = client.ActiveClientIP().toString() + std::to_string(client.ActiveClientPort());
        //
        // The initial implementation also incorporated time into the unique identifier, though this only served to confuse a whether
        // an identified client actually was known (their identificatin could be differentiated by a particular message).
                    // + std::to_string(sentTimeStamp); ...



        // For the client to be known, they must be found within the active clients container.

        if (activeClients.find(receivedPositional.playerIdentifier) == activeClients.end())
        {
            // Insert the sender into the active clients container.
            //
            activeClients.insert({ {determinedID, AddressClient(client.ActiveClientIP(), client.ActiveClientPort())} });

            // Construct a SimulationTimeScale type data packet, so that the client can truly set their
            // initial position, and use the elapsed server time to secure a simulated time scale.
            sf::Packet sendPacket;
            TimeScaleData startClient;
            //
            startClient.overwriteIdentifier = determinedID;
            receivedPositional.playerIdentifier = determinedID;
            //
            DeterminePlayerStartPosition(startClient.overwritePositionX, startClient.overwritePositionY);
            startClient.clientTimeStamp = sentTimeStamp;

            // A more accurate assignment, below, would account for the single trip time, subtracted.
            // For the purposes of escaping the idle client check, this solution is temporarily suitable, but not robust.
            sentTimeStamp = serverClock.getElapsedTime().asSeconds();


            if (activeClients.at(determinedID).GetOutgoingMessage()->getDataSize()
                <= sf::UdpSocket::MaxDatagramSize - (appendageBytes + timeScaleBytes)) {
                //
                *(activeClients.at(determinedID).GetOutgoingMessage()) << static_cast<sf::Uint8>(ToUnderlying(MessageType::SimulationTimeScale))
                    << startClient << serverClock.getElapsedTime().asSeconds();
            }

            /*
            sf::Socket::Status sendStatus = UDPServerSocket.send(sendPacket, client.ActiveClientIP(), client.ActiveClientPort());
            while (sendStatus == sf::UdpSocket::Partial) {

                // A partial send failure must be resolved to avoid corruption.
                sendStatus = UDPServerSocket.send(sendPacket, client.ActiveClientIP(), client.ActiveClientPort());


                // Where packet loss occurs (a failure to send) in the case of an unknown client, the client is
                // left to their own devices, in a sense. The client is not added to the map of activeClients,
                // and has the opportunity to attempt to establish their join in the future.

                if (sendStatus != sf::UdpSocket::Partial) { return; }
            }*/

        }
        else {
            // Assign to the determined ID, the historical/communicated identifier.
            determinedID = receivedPositional.playerIdentifier;
        }


        // Distribution of the recieved data is only a valuable operation if the data is neither irrelevant or redundant.
        
        if (activeClients.at(determinedID).ReceievedIsCurrent(receivedPositional, sentTimeStamp))
        {   
            // Iterate through the active clients container and update their respective outgoing packet.
            //
            for (auto& recipient : activeClients) {

                // Distribute to all bar the original sender.
                if (recipient.second == client) { continue; }

                // Assign the outgoing message pointer, locally.
                sf::Packet* sendPacket = recipient.second.GetOutgoingMessage();

                // Determine that the packet is not already full.
                //
                if (sendPacket->getDataSize() <= sf::UdpSocket::MaxDatagramSize - (appendageBytes + positionalBytes))
                {
                    // Re-package the positional data for distribution.
                    *sendPacket << static_cast<sf::Uint8>(ToUnderlying(MessageType::Positional)) << receivedPositional << sentTimeStamp;
                }
            }
        }
    }
}


void ServerConsole::IncomingManager()
{
    // Seed the pseudo-random number generator with time.
    srand(time(NULL));

    // Varied reports of the Visual Studio C++ underlying state handling call the extent of rand / srand thread
    // safety into question. Convenient alternatives such as the thread safe rand_r were considered, though I am
    // uncertain as to this functions availability outside linux.
    //
    // Although srand/rand are only called within the same thread in this application, the use of a different
    // random number generator would be a keen further development.


    // Under the condition that the server has not broke the inactivity limit of two minutes...
    //
    while (activeClock.getElapsedTime().asSeconds() < inactiveLimit) {

        // Initialise receive paramaters. The below function will fill their contents.
        sf::Packet receivePacket;
        AddressClient receiveClient;

        if (UDPServerSocket.receive(receivePacket, receiveClient.ActiveClientIP(), receiveClient.ActiveClientPort()) == sf::Socket::Done)
        {
            // Extract the message header, and resolve according to the indicated message type.
            sf::Uint8 messageHeader;

            if (receivePacket >> messageHeader) {
                switch (messageHeader)
                {
                case (ToUnderlying(MessageType::Positional)):
                    //
                    ReceivePositional(receivePacket, receiveClient);
                    break;
                }
            }
            else {
                // Packet data extraction failure might indicate the receipt of an unrecognised message.
                // This also might be an opportunity to recognise packet loss. Consider.
            }

            // Restart the activity clock, on account of activity.
            activeClock.restart();
        }
    }

    // After exiting the while, flag shutdown.
    //
    // For the function to complete, an exit condition must be present (consider when closing the server).
}

void ServerConsole::InitiateSelection()
{
    // Data availability is checked by a worker thread seeking to mitigate the stasis affect of
    // blocking and the period of waiting for receipt of data.

    // In regards to selection: The program, in its current state, does not dedicate an individual socket to
    // each individual client.
    // At the moment, a blocking receive is employed for incoming messages (as opposed to a blocking sf::SocketSelector).

    incomingManager = new std::thread([this] { IncomingManager(); });
}


// Note, that the error (Warning C26812: Prefer 'enum class' over 'enum' (Enum.3)
//                                                      The enum type type-name is unscoped. Prefer 'enum class' over 'enum' (Enum.3))
// has been supressed (instances of the error flagged as a result of external library code, not my own work).