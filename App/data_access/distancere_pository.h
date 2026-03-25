#ifndef DISTANCERE_POSITORY_H
#define DISTANCERE_POSITORY_H

class Application;

class DistanceRepository
{
    friend class Application;

public:


private:
    DistanceRepository();
    DistanceRepository(const DistanceRepository& copy) = delete;
    DistanceRepository& operator=(const DistanceRepository& copy) = delete;
    DistanceRepository(DistanceRepository&& move) = delete;
    DistanceRepository& operator=(DistanceRepository&& move) = delete;
};

#endif // DISTANCERE_POSITORY_H
