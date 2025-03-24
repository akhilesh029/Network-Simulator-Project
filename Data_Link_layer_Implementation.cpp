#include <graphics.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

// Frame structure
struct Frame {
    string srcMAC;
    string destMAC;
    string data;
    int seqNum; // For sliding window
    int checksum; // For error detection
};

// Switch class
class Switch {
private:
    map<string, int> macTable; // MAC address table (MAC -> Port)
    int numPorts;
    vector<string> deviceMACs; // List of device MACs
    vector<bool> channelBusy; // To simulate CSMA/CD

    // Error control: Checksum function
    int calculateChecksum(string data) {
        int sum = 0;
        for (char c : data) {
            sum += c;
        }
        return sum % 256; // Modulo to limit checksum value
    }

    // Simulate collision detection and backoff for CSMA/CD
    bool csmaCD(int port) {
        if (channelBusy[port - 1]) {
            cout << "\nCollision detected on Port " << port << ". Retrying with backoff..." << endl;
            int backoffTime = rand() % 5 + 1; // Random backoff
            delay(backoffTime * 100); // Delay for backoff
            return false;
        }
        return true;
    }

public:
    Switch(int ports) {
        numPorts = ports;
        channelBusy.resize(ports, false);
        srand(time(0));
    }

    // Learn MAC address and update table
    void learnMAC(string mac, int port) {
        if (macTable.find(mac) == macTable.end()) {
            macTable[mac] = port;
            drawMACTable();
        }
    }

    // Forward frame based on MAC address learning and add protocol functionalities
    void forwardFrame(Frame frame, int port) {
        cout << "\nSending Frame: " << frame.data << " | Source: " << frame.srcMAC
             << " | Destination: " << frame.destMAC << endl;

        // Learn source MAC
        learnMAC(frame.srcMAC, port);

        // Check for errors using checksum
        if (frame.checksum != calculateChecksum(frame.data)) {
            cout << "Error detected in frame. Frame discarded." << endl;
            return;
        }

        drawFrame(frame, port);

        // CSMA/CD for collision detection
        if (!csmaCD(port)) {
            cout << "Collision resolved. Retrying..." << endl;
            forwardFrame(frame, port);
            return;
        }

        // Check if destination MAC is known
        if (macTable.find(frame.destMAC) != macTable.end()) {
            int outPort = macTable[frame.destMAC];
            if (outPort == port) {
                cout << "Frame dropped (same port)." << endl;
            } else {
                cout << "Unicasting frame to Port " << outPort << endl;
                channelBusy[outPort - 1] = true; // Mark channel busy
                drawArrow(port, outPort, GREEN);
                sendACK(frame.destMAC, frame.srcMAC, outPort, port);
                channelBusy[outPort - 1] = false; // Release channel
            }
        } else {
            cout << "Destination MAC unknown. Broadcasting frame." << endl;
            broadcastFrame(frame, port);
        }
    }

    // Broadcast frame to all ports except source
    void broadcastFrame(Frame frame, int srcPort) {
        for (int i = 1; i <= numPorts; i++) {
            if (i != srcPort) {
                drawArrow(srcPort, i, YELLOW);
                cout << "Broadcasting to Port " << i << endl;
            }
        }
    }

    // Send ACK frame back to source after successful delivery
    void sendACK(string destMAC, string srcMAC, int destPort, int srcPort) {
        cout << "Sending ACK from Port " << destPort << " to Port " << srcPort << endl;
        Frame ackFrame = {destMAC, srcMAC, "ACK", 0, 0};
        drawArrow(destPort, srcPort, MAGENTA); // ACK shown with magenta arrow
    }

    // Draw MAC table with updated entries
    void drawMACTable() {
        setcolor(WHITE);
        rectangle(850, 350, 1150, 550);
        outtextxy(870, 360, "MAC Table:");
        int y = 380;
        for (auto entry : macTable) {
            string entryText = "MAC: " + entry.first + " -> Port " + to_string(entry.second);
            outtextxy(860, y, const_cast<char *>(entryText.c_str()));
            y += 20;
        }
    }

    // Draw devices and switch dynamically
    void drawNetwork(vector<string> macs) {
        deviceMACs = macs;
        setcolor(WHITE);
        rectangle(500, 300, 600, 350);
        outtextxy(520, 320, "Switch");

        for (int i = 1; i <= numPorts; i++) {
            int x = 1200 / (numPorts + 1) * i;
            circle(x, 200, 30);
            string label = "D" + to_string(i);
            outtextxy(x - 10, 180, const_cast<char *>(label.c_str()));
            line(x, 200, 550, 325);
        }
    }

    // Draw frame movement towards switch
    void drawFrame(Frame frame, int srcPort) {
        int srcX = 1200 / (numPorts + 1) * srcPort;
        setcolor(BLUE);
        line(srcX, 200, 550, 325);
        delay(500);
    }

    // Draw arrows to represent frame forwarding and ACK
    void drawArrow(int srcPort, int destPort, int color) {
        int srcX = 1200 / (numPorts + 1) * srcPort;
        int destX = 1200 / (numPorts + 1) * destPort;
        setcolor(color);
        line(550, 325, destX, 200);
        delay(500);
    }

    // Sliding Window Protocol: Simulate sending multiple frames
    void slidingWindowSend(vector<Frame> frames, int windowSize) {
        int n = frames.size();
        for (int i = 0; i < n;) {
            cout << "\nSliding Window [Window Size: " << windowSize << "]\n";
            for (int j = i; j < i + windowSize && j < n; j++) {
                cout << "Sending frame " << frames[j].seqNum << " | Data: " << frames[j].data << endl;
                forwardFrame(frames[j], 1);
            }

            for (int j = i; j < i + windowSize && j < n; j++) {
                if (rand() % 10 < 8) { // Simulate 80% ACK success
                    cout << "ACK received for frame " << frames[j].seqNum << "." << endl;
                } else {
                    cout << "Frame " << frames[j].seqNum << " lost. Retransmitting..." << endl;
                    forwardFrame(frames[j], 1);
                }
            }
            i += windowSize;
        }
    }

    // Run automated test cases
    void runTests() {
        cout << "\nRunning Test Cases...\n";

        // Test Case 1: Single switch with 5 devices
        cout << "\nTest Case 1: Single Switch with 5 Devices" << endl;
        vector<string> macs = generateMACs(5);
        drawNetwork(macs);
        drawMACTable();
        Frame frame1 = {macs[0], macs[3], "Hello", 1, calculateChecksum("Hello")};
        forwardFrame(frame1, 1);

        // Test Case 2: Error control (checksum mismatch)
        cout << "\nTest Case 2: Error Control (Checksum Mismatch)" << endl;
        Frame frame2 = {macs[1], macs[4], "Error", 2, 123};
        forwardFrame(frame2, 2);

        // Test Case 3: CSMA/CD with collision simulation
        cout << "\nTest Case 3: CSMA/CD (Collision Detection)" << endl;
        channelBusy[1] = true; // Force busy channel
        Frame frame3 = {macs[2], macs[0], "Retry", 3, calculateChecksum("Retry")};
        forwardFrame(frame3, 3);
        channelBusy[1] = false;

        // Test Case 4: Sliding Window Protocol
        cout << "\nTest Case 4: Sliding Window Protocol" << endl;
        vector<Frame> frames;
        for (int i = 0; i < 5; i++) {
            frames.push_back({macs[0], macs[1], "Data" + to_string(i), i + 1, calculateChecksum("Data" + to_string(i))});
        }
        slidingWindowSend(frames, 3);

        cout << "\nAll test cases executed successfully!\n";
    }

    // Generate MAC addresses for devices
    vector<string> generateMACs(int numDevices) {
        vector<string> macs;
        for (int i = 1; i <= numDevices; i++) {
            string mac = "AA:BB:CC:DD:0" + to_string(i);
            macs.push_back(mac);
        }
        return macs;
    }

    // Handle user input to send frames dynamically
    void sendFrames() {
        int numFrames;
        cout << "Enter the number of frames to send: ";
        cin >> numFrames;

        for (int i = 0; i < numFrames; i++) {
            int srcPort, destPort;
            string data;

            cout << "\nEnter source port (1-" << numPorts << "): ";
            cin >> srcPort;
            cout << "Enter destination port (1-" << numPorts << "): ";
            cin >> destPort;
            cout << "Enter frame data: ";
            cin.ignore();
            getline(cin, data);

            if (srcPort < 1 || srcPort > numPorts || destPort < 1 || destPort > numPorts) {
                cout << "Invalid port! Try again." << endl;
                continue;
            }

            Frame frame = {deviceMACs[srcPort - 1], deviceMACs[destPort - 1], data, 0, calculateChecksum(data)};
            forwardFrame(frame, srcPort);
        }
    }
};

// Main function
int main() {
    int numSwitches;
    cout << "Enter the number of switches to connect: ";
    cin >> numSwitches;

    for (int i = 0; i < numSwitches; i++) {
        int numDevices;
        cout << "\nEnter the number of devices for Switch " << i + 1 << ": ";
        cin >> numDevices;

        int gd = DETECT, gm;
        initwindow(1200, 700, ("Switch Simulation " + to_string(i + 1)).c_str());

        Switch mySwitch(numDevices);

        int choice;
        cout << "\nChoose Mode for Switch " << i + 1 << ":\n";
        cout << "1. Manual Simulation\n";
        cout << "2. Run Test Cases\n";
        cin >> choice;

        if (choice == 1) {
            mySwitch.sendFrames();
        } else {
            mySwitch.runTests();
        }

        getch();
        closegraph();
    }

    return 0;
}
