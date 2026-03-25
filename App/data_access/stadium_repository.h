#ifndef STADIUM_REPOSITORY_H
#define STADIUM_REPOSITORY_H

class Application;

class StadiumRepository
{
    friend class Application;

public:


private:
    StadiumRepository(const StadiumRepository& copy) = delete;
    StadiumRepository& operator=(const StadiumRepository& copy) = delete;
    StadiumRepository(StadiumRepository&& move) = delete;
    StadiumRepository& operator=(StadiumRepository&& move) = delete;
    StadiumRepository();
};

#endif // STADIUM_REPOSITORY_H
