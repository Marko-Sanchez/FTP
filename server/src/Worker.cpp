#include "Worker.hpp"
#include <sys/socket.h>

Worker::Worker(ssize_t id, std::shared_ptr<std::atomic_bool> stop):
    threadID(id),
    jobsCompleted(0),
    workerStop(false),
    lb_stop(stop)
{
    workerThread = std::thread(&Worker::worker, this);
}

Worker::~Worker()
{
    printf("Worker %ld: Thread stopped\n", GetID());

    workerStop.store(true);
    if (workerThread.joinable())
    {
        workerThread.join();
    }
}

Worker::Worker(const Worker &other):
    threadID(other.threadID),
    jobsCompleted(other.jobsCompleted),
    lb_stop(other.lb_stop),
    jobQueue(other.jobQueue)
{}

Worker::Worker(Worker &&other):
    threadID(other.threadID),
    jobsCompleted(other.jobsCompleted),
    lb_stop(other.lb_stop),
    jobQueue(std::move(other.jobQueue))
{
    if (other.workerThread.joinable())
    {
        other.workerThread.join();
    }
}

Worker& Worker::operator=(Worker &other)
{
    if (this != &other)
    {
        threadID = other.threadID;
        jobsCompleted = other.jobsCompleted;
        lb_stop = other.lb_stop;
        jobQueue = other.jobQueue;


        if (other.workerThread.joinable())
        {
            other.workerThread.join();
        }
    }
    return *this;
}

void Worker::worker()
{
    const size_t BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE] = {0};

    printf("Worker %ld: Thread started\n", GetID());
    while (!lb_stop->load() and !workerStop.load())
    {
        if (GetJobsInQueue() > 0)
        {
            int client_socket = GetJob();
            if (client_socket == -1)
            {
                printf("Worker %ld: No job\n", GetID());
                continue;
            }
            printf("Worker %ld: Job received\n", GetID());

            ssize_t bytesRead{0};
            while ((bytesRead = recv(client_socket, &buffer, BUFFER_SIZE, MSG_DONTWAIT)) > 0)
            {
                printf("Worker %ld: %s\n", GetID(), buffer);
                memset(buffer, 0, BUFFER_SIZE);
            }

            char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 4\r\n\r\nOK\r\n\r\n";
            send(client_socket, response, sizeof(response), 0);
            close(client_socket);

            IncrementJobsCompleted();
            memset(buffer, 0, BUFFER_SIZE);
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    printf("Worker %ld: Thread stopped\n", GetID());
}

void Worker::AddJob(int clientsocket)
{
    std::lock_guard<std::mutex> lock(queuemutex);
    jobQueue.push(clientsocket);
}

int Worker::GetJob()
{
    std::lock_guard<std::mutex> lock(queuemutex);

    if (jobQueue.empty()) return -1;

    int clientsocket = jobQueue.front();
    jobQueue.pop();

    return clientsocket;
}

void Worker::IncrementJobsCompleted()
{
    ++jobsCompleted;
}

int Worker::GetJobsInQueue() const
{
    return jobQueue.size();
}

ssize_t Worker::GetID() const
{
    return threadID;
}

ssize_t Worker::GetJobsCompleted() const
{
    return jobsCompleted;
}
