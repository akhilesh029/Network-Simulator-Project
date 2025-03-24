#include <iostream>
#include <graphics.h>
#include <conio.h>
#include <vector>
#include <string>
#include <math.h>

using namespace std;

// Constants
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 800;
const int HUB_X = 500, HUB_Y = 400;
const int DEVICE_RADIUS = 25;  // Increased device radius for visibility
const int HUB_HOLE_RADIUS = 6; // Radius of hub holes

// Device Class
class Device {
public:
    int id;
    int x, y;
    string name;

    Device(int id, string name, int x, int y) {
        this->id = id;
        this->name = name;
        this->x = x;
        this->y = y;
    }

    void drawDevice() {
        setcolor(WHITE);
        circle(x, y, DEVICE_RADIUS);
        setfillstyle(SOLID_FILL, YELLOW);
        floodfill(x, y, WHITE);
        outtextxy(x - 10, y + 30, (char*)name.c_str());
    }

    void sendDataToHub(string data) {
        cout << name << " sent data to Hub: \"" << data << "\"" << endl;
        setcolor(RED);
        line(x, y, HUB_X, HUB_Y);
        delay(800);
        drawArrow(x, y, HUB_X, HUB_Y);
        outtextxy((x + HUB_X) / 2 - 30, (y + HUB_Y) / 2 - 15, (char*)data.c_str());
        delay(1000);
        clearLabel((x + HUB_X) / 2 - 30, (y + HUB_Y) / 2 - 15);
        clearLine(x, y, HUB_X, HUB_Y);
    }

    void receiveAck(Device& sender, bool isReceiver) {
        clearLabel(x - 15, y - 40);
        if (isReceiver) {
            setcolor(GREEN);
            drawTick(x, y);
            cout << "ACK received by " << name << " from Hub" << endl;
            outtextxy(x - 15, y - 40, "ACK Received");
            delay(500);
        } else if (id != sender.id) {
            setcolor(RED);
            drawCross(x, y);
            cout << name << " ignored the data." << endl;
            outtextxy(x - 20, y - 40, "Data Ignored");
            delay(500);
        }
    }

    void receiveData(Device& sender, string data, bool isReceiver) {
        setcolor(CYAN);
        line(HUB_X, HUB_Y, x, y);
        delay(800);
        drawArrow(HUB_X, HUB_Y, x, y);
        outtextxy((HUB_X + x) / 2, (HUB_Y + y) / 2 - 15, (char*)data.c_str());
        delay(800);
        clearLine(HUB_X, HUB_Y, x, y);

        clearLabel(x - 15, y - 40);
        if (isReceiver) {
            setcolor(GREEN);
            drawTick(x, y);
            cout  << name << " received data: \"" << data << "\"" << endl;
            outtextxy(x - 15, y - 40, "Data Received");
            sendAckToHub();
        }
        receiveAck(sender, isReceiver);
    }

    void sendAckToHub() {
        setcolor(GREEN);
        line(x, y, HUB_X, HUB_Y);
        delay(800);
        drawArrow(x, y, HUB_X, HUB_Y);
        outtextxy((x + HUB_X) / 2, (y + HUB_Y) / 2 - 15, "ACK");
        delay(800);
        clearLine(x, y, HUB_X, HUB_Y);
        cout << "ACK sent from " << name << " to Hub." << endl;
    }

    void receiveAckFromHub(Device& sender) {
        setcolor(GREEN);
        line(HUB_X, HUB_Y, sender.x, sender.y);
        delay(800);
        drawArrow(HUB_X, HUB_Y, sender.x, sender.y);

        string ackMessage = "ACK from Hub to " + sender.name;
        outtextxy((HUB_X + sender.x) / 2 - 40, (HUB_Y + sender.y) / 2 - 15, (char*)ackMessage.c_str());

        delay(2500);
        clearLabel((HUB_X + sender.x) / 2 - 40, (HUB_Y + sender.y) / 2 - 15);
        delay(500);
        clearLine(HUB_X, HUB_Y, sender.x, sender.y);

        cout << "ACK delivered to " << sender.name << " from Hub." << endl;
    }

private:
    void drawArrow(int x1, int y1, int x2, int y2) {
        int dx = x2 - x1, dy = y2 - y1;
        float angle = atan2(dy, dx);
        int arrowLength = 8;

        line(x2, y2, x2 - arrowLength * cos(angle - 0.3), y2 - arrowLength * sin(angle - 0.3));
        line(x2, y2, x2 - arrowLength * cos(angle + 0.3), y2 - arrowLength * sin(angle + 0.3));
    }

    void clearLine(int x1, int y1, int x2, int y2) {
        setcolor(WHITE);
        line(x1, y1, x2, y2);
    }

    void drawTick(int x, int y) {
        line(x - 5, y + 5, x, y + 10);
        line(x, y + 10, x + 10, y - 5);
    }

    void drawCross(int x, int y) {
        line(x - 5, y - 5, x + 5, y + 5);
        line(x - 5, y + 5, x + 5, y - 5);
    }

    void clearLabel(int x, int y) {
        setcolor(BLACK);
        setfillstyle(SOLID_FILL, BLACK);
        bar(x - 5, y - 5, x + 60, y + 15);
    }
};

// Hub Class
class Hub {
public:
    vector<Device*> connectedDevices;

    void connectDevice(Device* device) {
        connectedDevices.push_back(device);
    }

    void receiveData(Device& sender, string data, int receiverId) {
        broadcastData(sender, data, receiverId);
    }

    void broadcastData(Device& sender, string data, int receiverId) {
        for (auto& device : connectedDevices) {
            bool isReceiver = device->id == receiverId;
            if (device->id != sender.id) {
                device->receiveData(sender, data, isReceiver);
            }
        }

        for (auto& device : connectedDevices) {
            if (device->id == receiverId) {
                device->sendAckToHub();
                delay(800);
                sender.receiveAckFromHub(sender);
                break;
            }
        }
    }

    void drawHub() {
        setcolor(WHITE);
        rectangle(HUB_X - 30, HUB_Y - 30, HUB_X + 30, HUB_Y + 30);
        setfillstyle(SOLID_FILL, LIGHTGRAY);
        floodfill(HUB_X, HUB_Y, WHITE);
        outtextxy(HUB_X - 15, HUB_Y + 40, "HUB");
        drawHolesInHub();
    }

    void drawHolesInHub() {
        int angleStep = 360 / connectedDevices.size();
        for (int i = 0; i < connectedDevices.size(); i++) {
            int holeX = HUB_X + 20 * cos(i * angleStep * M_PI / 180);
            int holeY = HUB_Y + 20 * sin(i * angleStep * M_PI / 180);
            setcolor(WHITE);
            circle(holeX, holeY, HUB_HOLE_RADIUS);
            setfillstyle(SOLID_FILL, BLACK);
            floodfill(holeX, holeY, WHITE);
        }
    }
};

// Main Simulation
int main() {
    initwindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Network Simulator with Hub, ACK & Holes");

    Device deviceA(1, "A", 350, 250);
    Device deviceB(2, "B", 650, 250);
    Device deviceC(3, "C", 350, 550);
    Device deviceD(4, "D", 650, 550);
    Device deviceE(5, "E", 500, 600);

    deviceA.drawDevice();
    deviceB.drawDevice();
    deviceC.drawDevice();
    deviceD.drawDevice();
    deviceE.drawDevice();

    Hub hub;
    hub.connectDevice(&deviceA);
    hub.connectDevice(&deviceB);
    hub.connectDevice(&deviceC);
    hub.connectDevice(&deviceD);
    hub.connectDevice(&deviceE);
    hub.drawHub();

    for (auto& device : hub.connectedDevices) {
        line(device->x, device->y, HUB_X, HUB_Y);
    }

    string senderName, receiverName, message;
    cout << "Enter sender device name (A/B/C/D/E): ";
    cin >> senderName;
    cout << "Enter receiver device name (A/B/C/D/E): ";
    cin >> receiverName;
    cout << "Enter message: ";
    cin.ignore();
    getline(cin, message);

    Device* sender = nullptr;
    Device* receiver = nullptr;

    vector<Device*> devices = {&deviceA, &deviceB, &deviceC, &deviceD, &deviceE};
    for (auto& device : devices) {
        if (device->name == senderName) {
            sender = device;
        }
        if (device->name == receiverName) {
            receiver = device;
        }
    }

    if (sender && receiver) {
        sender->sendDataToHub(message);
        hub.receiveData(*sender, message, receiver->id);
    } else {
        cout << "Invalid sender or receiver name." << endl;
    }

    getch();
    closegraph();
    return 0;
}
