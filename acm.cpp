
#include <bits/stdc++.h>
#include "parser.h"

#define all(v) v.begin(), v.end()

using namespace std;

/** Global Constants **/
const string TRACE = "trace";
const string SHOW_STATISTICS = "stats";
const string ALGORITHMS[9] = {"", "FCFS", "RR-", "SPN", "SRT", "HRRN", "FB-1", "FB-2i", "AGING"};

bool sortByServiceTime(const tuple<string, int, int> &a, const tuple<string, int, int> &b) {
    return get<2>(a) < get<2>(b);
}

bool sortByArrivalTime(const tuple<string, int, int> &a, const tuple<string, int, int> &b) {
    return get<1>(a) < get<1>(b);
}

bool descendingly_by_response_ratio(tuple<string, double, int> a, tuple<string, double, int> b) {
    return get<1>(a) > get<1>(b);
}

bool byPriorityLevel(const tuple<int, int, int> &a, const tuple<int, int, int> &b) {
    if (get<0>(a) == get<0>(b))
        return get<2>(a) > get<2>(b);
    return get<0>(a) > get<0>(b);
}

void clear_timeline() {
    for (int i = 0; i < last_instant; i++)
        for (int j = 0; j < process_count; j++)
            timeline[i][j] = ' ';
}

string getProcessName(const tuple<string, int, int> &a) {
    return get<0>(a);
}

int getArrivalTime(const tuple<string, int, int> &a) {
    return get<1>(a);
}

int getServiceTime(const tuple<string, int, int> &a) {
    return get<2>(a);
}

int getPriorityLevel(const tuple<string, int, int> &a) {
    return get<2>(a);
}

double calculate_response_ratio(int wait_time, int service_time) {
    return (wait_time + service_time) * 1.0 / service_time;
}

void fillInWaitTime() {
    for (int i = 0; i < process_count; i++) {
        int arrivalTime = getArrivalTime(processes[i]);
        for (int k = arrivalTime; k < finishTime[i]; k++) {
            if (timeline[k][i] != '*')
                timeline[k][i] = '.';
        }
    }
}

void firstComeFirstServe() {
    int time = getArrivalTime(processes[0]);
    for (int i = 0; i < process_count; i++) {
        int processIndex = i;
        int arrivalTime = getArrivalTime(processes[i]);
        int serviceTime = getServiceTime(processes[i]);

        finishTime[processIndex] = time + serviceTime;
        turnAroundTime[processIndex] = finishTime[processIndex] - arrivalTime;
        normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / serviceTime;

        for (int j = time; j < finishTime[processIndex]; j++)
            timeline[j][processIndex] = '*';
        for (int j = arrivalTime; j < time; j++)
            timeline[j][processIndex] = '.';
        time += serviceTime;
    }
}

void roundRobin(int originalQuantum) {
    queue<pair<int, int>> q;
    int j = 0;
    if (getArrivalTime(processes[j]) == 0) {
        q.push(make_pair(j, getServiceTime(processes[j])));
        j++;
    }
    int currentQuantum = originalQuantum;
    for (int time = 0; time < last_instant; time++) {
        if (!q.empty()) {
            int processIndex = q.front().first;
            q.front().second--;
            int remainingServiceTime = q.front().second;
            int arrivalTime = getArrivalTime(processes[processIndex]);
            int serviceTime = getServiceTime(processes[processIndex]);
            currentQuantum--;
            timeline[time][processIndex] = '*';

            while (j < process_count && getArrivalTime(processes[j]) == time + 1) {
                q.push(make_pair(j, getServiceTime(processes[j])));
                j++;
            }

            if (currentQuantum == 0 && remainingServiceTime == 0) {
                finishTime[processIndex] = time + 1;
                turnAroundTime[processIndex] = finishTime[processIndex] - arrivalTime;
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / serviceTime;
                currentQuantum = originalQuantum;
                q.pop();
            } else if (currentQuantum == 0 && remainingServiceTime != 0) {
                q.pop();
                q.push(make_pair(processIndex, remainingServiceTime));
                currentQuantum = originalQuantum;
            } else if (currentQuantum != 0 && remainingServiceTime == 0) {
                finishTime[processIndex] = time + 1;
                turnAroundTime[processIndex] = finishTime[processIndex] - arrivalTime;
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / serviceTime;
                q.pop();
                currentQuantum = originalQuantum;
            }
        }

        while (j < process_count && getArrivalTime(processes[j]) == time + 1) {
            q.push(make_pair(j, getServiceTime(processes[j])));
            j++;
        }
    }
    fillInWaitTime();
}

void shortestProcessNext() {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    int j = 0;
    for (int i = 0; i < last_instant; i++) {
        while (j < process_count && getArrivalTime(processes[j]) <= i) {
            pq.push(make_pair(getServiceTime(processes[j]), j));
            j++;
        }
        if (!pq.empty()) {
            int processIndex = pq.top().second;
            int arrivalTime = getArrivalTime(processes[processIndex]);
            int serviceTime = getServiceTime(processes[processIndex]);
            pq.pop();

            int temp = arrivalTime;
            for (; temp < i; temp++)
                timeline[temp][processIndex] = '.';

            temp = i;
            for (; temp < i + serviceTime; temp++)
                timeline[temp][processIndex] = '*';

            finishTime[processIndex] = i + serviceTime;
            turnAroundTime[processIndex] = finishTime[processIndex] - arrivalTime;
            normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / serviceTime;
            i = temp - 1;
        }
    }
}

void shortestRemainingTime() {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    int j = 0;
    for (int i = 0; i < last_instant; i++) {
        while (j < process_count && getArrivalTime(processes[j]) == i) {
            pq.push(make_pair(getServiceTime(processes[j]), j));
            j++;
        }
        if (!pq.empty()) {
            int processIndex = pq.top().second;
            int remainingTime = pq.top().first;
            pq.pop();
            int serviceTime = getServiceTime(processes[processIndex]);
            int arrivalTime = getArrivalTime(processes[processIndex]);
            timeline[i][processIndex] = '*';

            if (remainingTime == 1) {
                finishTime[processIndex] = i + 1;
                turnAroundTime[processIndex] = finishTime[processIndex] - arrivalTime;
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / serviceTime;
            } else {
                pq.push(make_pair(remainingTime - 1, processIndex));
            }
        }
    }
    fillInWaitTime();
}

void highestResponseRatioNext() {
    vector<tuple<string, double, int>> present_processes;
    int j = 0;
    for (int current_instant = 0; current_instant < last_instant; current_instant++) {
        while (j < process_count && getArrivalTime(processes[j]) <= current_instant) {
            present_processes.push_back(make_tuple(getProcessName(processes[j]), 1.0, 0));
            j++;
        }

        for (auto &proc : present_processes) {
            string process_name = get<0>(proc);
            int process_index = processToIndex[process_name];
            int wait_time = current_instant - getArrivalTime(processes[process_index]);
            int service_time = getServiceTime(processes[process_index]);
            get<1>(proc) = calculate_response_ratio(wait_time, service_time);
        }

        sort(all(present_processes), descendingly_by_response_ratio);

        if (!present_processes.empty()) {
            int process_index = processToIndex[get<0>(present_processes[0])];
            while (current_instant < last_instant && get<2>(present_processes[0]) != getServiceTime(processes[process_index])) {
                timeline[current_instant][process_index] = '*';
                current_instant++;
                get<2>(present_processes[
