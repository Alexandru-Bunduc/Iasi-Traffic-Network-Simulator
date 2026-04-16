# Iasi Traffic Network Simulator
A Linux-based client-server application simulating a real-time traffic monitoring and navigation system for the city of Iași. The project focuses on low-level network programming, utilizing TCP sockets and I/O multiplexing to handle multiple concurrent users.
# Key Technical Features
1) Socket Programming: Implemented a robust communication layer using AF_INET TCP Sockets.

2) I/O Multiplexing: Utilized the select() system call to monitor multiple file descriptors (network sockets and stdin) simultaneously without blocking.

3) Custom Application Protocol: Defined a structured communication protocol (pachet_generic) to handle various command types like authentication, location updates, and real-time alerts.

4) Graph-based Routing: The server manages a simplified map of Iași, allowing users to navigate between nodes (e.g., Piata Unirii, Copou, Podu Ros) while monitoring speed limits.

5) Real-time Broadcasts: Implemented an event-driven system where reporting an accident triggers an immediate alert to all connected clients.
