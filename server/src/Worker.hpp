#ifndef WORKER_HPP
#define WORKER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


#include <mutex>
#include <queue>
#include <thread>
#include <memory>
#include <atomic>
#include <cstring>

class Worker
{
    private:
        std::mutex queuemutex;
        ssize_t threadID;
        ssize_t jobsCompleted;
        std::atomic_bool workerStop;

        std::shared_ptr<std::atomic_bool> lb_stop;
        std::thread workerThread;
        std::queue<int> jobQueue;

        void worker();

    public:
        Worker(ssize_t id, std::shared_ptr<std::atomic_bool> loadbalancer_stop);
        ~Worker();
        Worker(const Worker &other);
        Worker(Worker &&other);
        Worker& operator=(Worker &other);

        void AddJob(int clientsocket);
        int GetJob();
        void IncrementJobsCompleted();
        int GetJobsInQueue() const;
        ssize_t GetID() const;
        ssize_t GetJobsCompleted() const;

};

#endif // WORKER_HPP
