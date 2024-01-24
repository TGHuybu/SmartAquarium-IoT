# Smart Aquarium
Final project for [HCMUS](https://en.hcmus.edu.vn/)' course on Physics for *Information Technology (Internet of Things)*.

The device is built using the ESP8266 NodeMCU microchip and various sensors. 
The aquarium have Wi-Fi support and can be controlled remotely through a web interface built with [Node-RED](https://nodered.org/).

## Navigate the repository
- `sketch/`: contains the source code for your IoT device (`sketch.ino`).
- `.json` file is the Node-RED diagram for the web interface and flows. Be cautious because the web is not yet optimized :(.
- `.pdf` file is the final report of the project, writen in **VIETNAMESE**.
- `.stl` file is the 3D model of the device.

## Node-RED Guide (*simplified*)
### Installation
- Make sure `npm` is installed!
- Run in terminal: `npm install -g node-red`
### Usage
- Run the following command in your terminal: `node-red`
- Navigate to http://localhost:1880

### Node-RED prerequisites
- From the menu at the top right corner, select `Manage palette`, then click the `Install` tab.
- Find the following nodes to install:
   - node-red
   - node-red-dashboard
   - @gogovega/node-red-contrib-firebase-realtime-database
   - node-red-node-email
