#include "LoadBalancer.hpp"

void LoadBalancer::Balance()
{
    while (!m_stop->load())
    {
        if (!m_jobs.empty())
        {
            std::lock_guard lock{m_qmutex};

            size_t smallestQueue{0};
            for (size_t worker{1}; worker < m_workers.size(); ++worker)
            {
                smallestQueue = (m_workers[worker].GetJobsInQueue() < m_workers[smallestQueue].GetJobsInQueue()) ? worker : smallestQueue;
            }
            m_workers[smallestQueue].AddJob(m_jobs.front());
            printf("Balancer: Job added to worker %ld\n", smallestQueue);
            m_jobs.pop();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }// end while
}

LoadBalancer::LoadBalancer(int numThreads)
{
    printf("Number of threads: %d\n", numThreads);
    m_stop = std::make_shared<std::atomic_bool>(false);
    m_workers.reserve(numThreads);

    for (int thread{0}; thread < numThreads; ++thread)
    {
        m_workers.emplace_back(thread, m_stop);
    }

    // Launch Balancer.
    m_balancer = std::thread{&LoadBalancer::Balance, this};
}

LoadBalancer::~LoadBalancer()
{
    m_stop->store(true);
    m_balancer.join();
}

void LoadBalancer::AddJob(int clientsocket)
{
    std::lock_guard lock{m_qmutex};
    m_jobs.push(clientsocket);
}
