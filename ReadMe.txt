# NetworkedServer


A UDP server application; part of a fast-action networked computer game using the Simple and Fast
Multimedia Library.

Gabriel Tierney - 1803088

***

This server application facilitates play between multiples of its partner application NetworkApplication.
The server runs automated processes to handle a client(NetworkApplication instance)'s outgoing messages.
It works to synchronise all known clients, to process and redistribute all of their communications.

Simply run the ready-to-run executable, and start any amount of NetworkApplication executables, to join.
	(Instructions on the user side, client application can be found in that applications own ReadMe).

***


Please find the required Demonstration Video linked below on Microsoft Stream, also a part of this submission.
	https://web.microsoftstream.com/video/9a2ce13f-33f4-4485-8bf6-686be00af134


Please also find the complete project, accessible as a GitHub repository, linked below.
	https://github.com/JohnGTi/NetworkedServer


***


Simple and Fast Multimedia Library (2018) SFML Team (2.5.1) [cross-platform software development library].
Available at: https://www.sfml-dev.org/download/sfml/2.5.1/ (Accessed: 05 November 2021).


	Please download the SFML 2.5.1 Visual C++ 15 (2017) - 32-bit.


Store the 'SFML-2.5.1' folder in X:\...\TierneyGabriel_1803088_CMP303\Server\NetworkedServer\NetworkedServer .

This solution was built with dynamically linked libraries.
Follow the official tutorial for creating and configuring an SFML project at
					https://www.sfml-dev.org/tutorials/2.5/start-vc.php .

For your convenience, the Debug and Release modules, respecitvely (for dynamic linking):

sfml-graphics-d.lib;sfml-window-d.lib;sfml-audio-d.lib;sfml-network-d.lib;sfml-system-d.lib;
sfml-graphics.lib;sfml-window.lib;sfml-audio.lib;sfml-network.lib;sfml-system.lib;

.