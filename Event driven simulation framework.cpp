#include <iostream>
#include <queue>
#include <random>
#include <thread>
#include <chrono>
#include <iomanip>
#include <string>

using namespace std;

// Simulated "seconds"
constexpr int SIM_SPEED = 1;  // real seconds per simulated second

struct CPUProcess {
    string taskName;
    int id;
    int arrivalTime;
    int serviceTime;

    bool operator>(const CPUProcess& other) const {
        return arrivalTime > other.arrivalTime;
    }
};

void waitSeconds(int simSeconds) {
    this_thread::sleep_for(chrono::seconds(simSeconds * SIM_SPEED));
}

int main() {
    const int numProcesses = 5;

    // Setup RNG
    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<> arrivalGapDist(1, 3);  // time between tasks
    uniform_int_distribution<> serviceTimeDist(3, 6); // service time

    // Simulating real-world CPU processes
    vector<string> processNames = {
        "I/O Operation",
        "Disk Read/Write",
        "Network Request",
        "Memory Allocation",
        "Data Compression"
    };

    // Generate process events
    priority_queue<CPUProcess, vector<CPUProcess>, greater<CPUProcess>> eventQueue;
    int currentTime = 0;
    int nextArrivalTime = 0;

    for (int i = 0; i < numProcesses; ++i) {
        int gap = arrivalGapDist(rng);
        nextArrivalTime += gap;
        int serviceTime = serviceTimeDist(rng);
        eventQueue.push({ processNames[i], i + 1, nextArrivalTime, serviceTime });

        // Log the operation generated
        cout << "[" << nextArrivalTime << "] " << processNames[i] << ": Generated.\n";
    }

    queue<CPUProcess> waitingQueue;
    bool cpuCoreBusy = false;
    int cpuCoreFreeAt = 0;
    int simulationTime = 0;
    int processesCompleted = 0;

    vector<int> waitTimes;
    vector<int> serviceTimes;

    cout << "-------------------------------------------\n";
    cout << "      Starting Real-Time CPU Simulation   \n";
    cout << "-------------------------------------------\n\n";

    // Main simulation loop
    while (processesCompleted < numProcesses || !waitingQueue.empty()) {
        // Handle arrivals
        while (!eventQueue.empty() && eventQueue.top().arrivalTime == simulationTime) {
            CPUProcess arriving = eventQueue.top();
            eventQueue.pop();

            cout << "[" << simulationTime << "] " << arriving.taskName
                << ": Submitted to Priority Queue with Priority " << arriving.arrivalTime << ".\n";

            if (waitingQueue.empty()) {
                cout << "[" << simulationTime << "] " << arriving.taskName
                    << ": Waiting in Priority Queue. Current queue size: " << waitingQueue.size() << ".\n";
            }

            if (!cpuCoreBusy) {
                cpuCoreBusy = true;
                cpuCoreFreeAt = simulationTime + arriving.serviceTime;
                waitTimes.push_back(0);
                serviceTimes.push_back(arriving.serviceTime);

                cout << "[" << simulationTime << "] " << arriving.taskName
                    << ": Execution started on Core 1.\n";
            }
            else {
                waitingQueue.push(arriving);
                cout << "[" << simulationTime << "] " << arriving.taskName
                    << ": Waiting in Priority Queue. Current queue size: " << waitingQueue.size() << ".\n";
            }
        }

        // Check if current service is done
        if (cpuCoreBusy && simulationTime == cpuCoreFreeAt) {
            cout << "[" << simulationTime << "] " << processNames[processesCompleted]
                << ": Execution completed on Core 1.\n";
            processesCompleted++;
            cpuCoreBusy = false;

            if (!waitingQueue.empty()) {
                CPUProcess next = waitingQueue.front();
                waitingQueue.pop();

                int waitTime = simulationTime - next.arrivalTime;
                waitTimes.push_back(waitTime);
                serviceTimes.push_back(next.serviceTime);

                cpuCoreBusy = true;
                cpuCoreFreeAt = simulationTime + next.serviceTime;

                cout << "[" << simulationTime << "] " << next.taskName
                    << ": Arrived at Core 1 from Queue. \n";

                cout << "[" << simulationTime << "] " << next.taskName
                    << ": Execution started on Core 1.\n";
            }
        }

        // Advance simulation time
        waitSeconds(1);
        simulationTime++;
    }

    // Final statistics
    int totalWait = 0, totalService = 0;
    for (int w : waitTimes) totalWait += w;
    for (int s : serviceTimes) totalService += s;

    cout << "\n-------------------------------------------\n";
    cout << "            Simulation Complete           \n";
    cout << "-------------------------------------------\n";
    cout << "Total processes: " << numProcesses << "\n";
    cout << "-------------------------------------------\n";
    cout << "Average wait time: " << fixed << setprecision(2)
        << (float)totalWait / waitTimes.size() << " seconds\n";
    cout << "Average service time: " << fixed << setprecision(2)
        << (float)totalService / serviceTimes.size() << " seconds\n";
    cout << "-------------------------------------------\n";
}
