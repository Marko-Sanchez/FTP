#ifndef LOADBALANCER_HPP
#define LOADBALANCER_HPP

#include <mutex>
#include <queue>
#include <vector>
#include <thread>

#include "Worker.hpp"

class LoadBalancer
{
    private:
        std::mutex               m_qmutex;
        std::shared_ptr<std::atomic_bool> m_stop;

        std::thread              m_balancer;
        std::vector<Worker>      m_workers;
        std::queue<int>          m_jobs;

        void Balance();

    public:
        LoadBalancer(int numThreads = std::thread::hardware_concurrency());
        ~LoadBalancer();

        void AddJob(int clientsocket);
};

#endif // LOADBALANCER_HPP
