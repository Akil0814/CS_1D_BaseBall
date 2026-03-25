#ifndef SOUVENIR_REPOSITORY_H
#define SOUVENIR_REPOSITORY_H

class Application;

class SouvenirRepository
{
    friend class Application;

public:


private:
    SouvenirRepository();
    SouvenirRepository(const SouvenirRepository& copy) = delete;
    SouvenirRepository& operator=(const SouvenirRepository& copy) = delete;
    SouvenirRepository(SouvenirRepository&& move) = delete;
    SouvenirRepository& operator=(SouvenirRepository&& move) = delete;
};

#endif // SOUVENIR_REPOSITORY_H
