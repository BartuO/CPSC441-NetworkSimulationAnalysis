#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


#define BUFFER_SIZE 100
#define BANDWIDTH 5

using namespace std;


//Name: Bartu Okan
//UCID: 30150180
//Tutorial: T04

//To compile: g++ -o sim simulation.cpp
//To run:./sim <TextFile>



//Queue structure
class Queue {
private:
    int front, rear;
    double arr[BUFFER_SIZE];

public:
    Queue() : front(-1), rear(-1) {}

    bool isEmpty() {
        return front == -1 && rear == -1;
    }

    bool isFull() {
        return (rear + 1) % BUFFER_SIZE == front;
    }

    void enqueue(double value) {

        if (isFull()) {
            std::cout << "Queue is full. Cannot enqueue." << std::endl;
            return;
        }


        if (isEmpty()) {

            front = rear = 0;
        } else {
            rear = (rear + 1) % BUFFER_SIZE;
        }

        arr[rear] = value;
    }

    void dequeue() {
        if (isEmpty()) {
            std::cout << "Queue is empty. Cannot dequeue." << std::endl;
            return;
        }

        if (front == rear) {

            front = rear = -1;

        } else {
            front = (front + 1) % BUFFER_SIZE;
        }
    }

    double getFront() {
        if (isEmpty()) {
            std::cout << "Queue is empty." << std::endl;
            return -1; 
        }

        return arr[front];
    }

    
};


//Converts from Megabits per second to bytes per second
double mbpsToBps(double mbps) {
    // 1 byte = 8 bits
    const double bitsPerByte = 8.0;

    double bps = (mbps * 1e6) / bitsPerByte;

    return bps;
}

//Reads a line from file and puts the result into next_arrival
void readLineFromFile(ifstream& inputFile, double (&next_ar)[2]){

    string line;

    if(getline(inputFile, line)){
        std::istringstream ss(line);
        ss >> next_ar[0] >> next_ar[1];
    } else{
        
        next_ar[0] = -1.11;
        next_ar[1] = -1.11;
    }


}



//Main loop
int main(int argc, char* argv[]){

    
    //Command line argument
     if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    string filename = argv[1];

    //Opening files
    ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        std::cerr << "Error opening the file: " << filename << std::endl;
        return 1;
    }


    //Converting bandwidth to use
    const double bandwidth = mbpsToBps(BANDWIDTH);


    //Declaring variables to use in the loop
    Queue buffer;
    Queue timerBuffer;
    int buffer_count = 0;
    

    double next_ar[2]; //Next arrival [0] = TIMESTAMP [1] = BYTES
    double next_dep_time;//Next departure time
    double last_dep_time;
    bool no_next_dep = true;
    bool no_next_arrival = true;



    //Declaring variables to store values
    int packetsArrived = 0;
    int packetsDropped = 0;
    double totalDelay = 0;

    
    //Initial read
    readLineFromFile(inputFile, next_ar);
    no_next_arrival = false;

    next_dep_time = next_ar[0] + next_ar[1] / bandwidth;
    last_dep_time = next_ar[0];
    no_next_dep = false;


    //cout << "bandwith in bps: "<< bandwidth << endl;
    //cout << "next_ar[0]"<< next_ar[0] << endl;
    //cout << "next_ar[1]"<< next_ar[1] << endl;
    //cout << "next_dep_time: "<< next_dep_time << endl;
    

    





    //Main loop, it breaks when no ARRIVALEVENT or DEPARTUREEVENT is left
    while (next_ar[1] >= 0 || no_next_dep == false){ //when both are negative, means there is nothing left

        if((next_ar[0] < next_dep_time && no_next_arrival == false) || no_next_dep == true ){ //ARRIVAL EVENT
            if(!buffer.isFull()){



                //cout << "ARRIVALEVENT" << endl;
                //cout << "next_ar[0]"<< next_ar[0] << endl;
                //cout << "next_ar[1]"<< next_ar[1] << endl;

                if(buffer.isEmpty()){
                    next_dep_time = next_ar[0] + next_ar[1] / bandwidth;
                    last_dep_time = next_ar[0];
                    no_next_dep = false;
                }

                buffer.enqueue(next_ar[1]);
                timerBuffer.enqueue(next_ar[0]);
                buffer_count++;
                packetsArrived++;

                readLineFromFile(inputFile, next_ar);
                if(next_ar[1] < 0){
                    no_next_arrival = true;
                }else{
                    no_next_arrival = false;
                }

            }else{
                packetsDropped++;
                //cout << "DROPEVENT" << endl;
                //cout << "next_ar[0]"<< next_ar[0] << endl;
                //cout << "next_ar[1]"<< next_ar[1] << endl;
                readLineFromFile(inputFile, next_ar);
                if(next_ar[1] < 0){
                    no_next_arrival = true;
                }else{
                    no_next_arrival = false;
                }
            }
        }
        else if(next_ar[0] >= next_dep_time || no_next_arrival == true){ //DEPARTURE EVENT -- If both are equal, departure happens first, then the new arrival is added to the buffer.

            //cout << "DEPARTUREEVENT" << endl;

            buffer.dequeue();
            double arrival_time = timerBuffer.getFront();
            double delay = next_dep_time - arrival_time - (next_dep_time - last_dep_time);
            //cout << "delay added: "<< delay << endl;
            //cout << "arrival_time: "<< arrival_time << endl;

            timerBuffer.dequeue();
            totalDelay += delay;

            
            //cout << "next_dep_time: "<< next_dep_time << endl;

            buffer_count--;

            if(!buffer.isEmpty()){
                last_dep_time = next_dep_time;
                next_dep_time += buffer.getFront() / bandwidth;
                
                no_next_dep = false;
            }else{
                last_dep_time = next_dep_time;
                no_next_dep = true;
            }
        }



    }
    //Calculating required values
    double totalPackets = packetsArrived + packetsDropped;
    double averageDelay = totalDelay / packetsArrived;
    double packetLossPercentage = static_cast<double>(packetsDropped) / (packetsArrived + packetsDropped);

    //Printing them out
    cout << endl << "Router Buffer Size: " << BUFFER_SIZE << endl;
    cout << "Bandwith(Mbps): " << BANDWIDTH << endl;
    cout << "------------------------ " << endl;
    cout << "Total packets:  " << totalPackets << endl;
    cout << "Packets arrived: " << packetsArrived << endl;
    cout << "Packets dropped: "<< packetsDropped << endl;
    cout << "Total queueing delay: " << totalDelay<< " seconds" <<endl;
    cout << "------------------------ " << endl;
    cout << "Average queueing delay: "<< averageDelay << " seconds" << endl;
    cout << "Packet loss percentage: %" << (packetLossPercentage * 100) <<endl;





    //Close file reader
    inputFile.close();
    //Exit
    return 0;

}