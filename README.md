# LDID Central server daemon

General system description
==========================

The LDID system (don't remember exactly what the initials stand for) was a set of small programs created to solve a situation in which there were the need to install several PCs as servers using ISP connections with dinamic IPs, and those servers must be available at all times at IP level to clients.
An obvious solution was to use one of several dynamic DNS services available in the market (comercials or not), but as the company I was working for (and the one that will use the system) had a set of real IPs available, and in those days I was really inspired in the creation of communication services, it was easier for me, and more secure for the own company in the long run, to design our own dynamic dns system.

The system was composed by three programs:

1- A central daemon running in a server with real IP in the company headquarters.

2- A daemon for the server that will run with a dynamic IP.

3- A program for the clients that needed to locate the dynamic IP servers.

It's functioning was quite simple: the daemons in the dynamic servers ran an infinite cycle sending messages with their IP address periodically to the central server. The later one, then kept a table updated with that information (indexed by a known key for each server). When the clients needed to connect to a dynamic server they requested the IP address of this one using the key of the server they wanted to access.


This daemon description
=======================

This is the daemon for the central server. It could be installed in any linux server with a real IP address. It worked totally auto, so it does not need any configuration file. It was provided, for convinience, with a start script and with a .deb package.