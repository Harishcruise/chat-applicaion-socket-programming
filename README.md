# Chat Application

This is a simple client-server chat application written in C++. It allows multiple clients to connect to a server and exchange messages.

## Table of Contents
- [Chat Application](#chat-application)
  - [Table of Contents](#table-of-contents)
  - [Features](#features)
  - [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Installation](#installation)
    - [Usage](#usage)
    - [Contributing](#contributing)
    - [Acknowledgments](#acknowledgments)

## Features
- Multiple clients can connect to the server.
- Clients can send messages to the server, and the server broadcasts messages to all connected clients.
- Private messages can be sent between clients.
- Server handles new client registration and notifies clients about join and leave events.

## Getting Started
### Prerequisites
- C++ compiler
- Git

### Installation
1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/chat-application.git
   
   cd chat-application

   g++ -o server server.cpp -pthread
   g++ -o client client.cpp

### Usage
1. Start the server:

    ```bash
    ./server
The server will be running on port 5208.

2. Connect clients to the server:

    ```bash
    ./client YourName
Replace "YourName" with the desired name for the client.

3. Send and receive messages:

Type messages in the client terminal to send them to the server.
The server will broadcast messages to all connected clients.
Use Quit or quit to exit the client.

### Contributing
We welcome contributions to improve this project. Feel free to open an issue or submit a pull request. Follow these steps to contribute:

1. Fork the project.
2. Create a new branch: git checkout -b feature/your-feature-name
3. Make your changes and commit: git commit -m 'Add new feature'
4. Push to the branch: git push origin feature/your-feature-name
5. Open a pull request.


### Acknowledgments
* This project was developed by Harish Kumaravel.
* Special thanks to Dr. Prashanth C. Ranga.
