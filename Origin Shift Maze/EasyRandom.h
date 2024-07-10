#pragma once
#include <random>
//ћаленький класс-обЄртка дл€ простого генерировани€ случайных целых чисел
class EasyRandom
{
    
private:
    std::random_device rnd;
    std::mt19937 gen;
    std::uniform_int_distribution<int> dist;
public:
    EasyRandom()
    {
        gen.seed(rnd());
    }
    EasyRandom(int a, int b)
    {
        gen.seed(rnd());
        dist.param(std::uniform_int_distribution<int>::param_type(a, b));
    }

    //—тавит интервал генерируемых чисел [a,b]
    void set(std::pair<int, int> params)
    {
        dist.param(std::uniform_int_distribution<int>::param_type(params.first, params.second));
    }
    //ѕолучить случайное число в заданном интервале
    int get()
    {
        return dist(gen);
    }
};

class EasyRandomf
{

private:
    std::random_device rnd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist;
public:
    EasyRandomf()
    {
        gen.seed(rnd());
    }
    EasyRandomf(float a, float b)
    {
        gen.seed(rnd());
        dist.param(std::uniform_real_distribution<float>::param_type(a, b));
    }

    //—тавит интервал генерируемых чисел [a,b]
    void set(std::pair<float, float> params)
    {
        dist.param(std::uniform_real_distribution<float>::param_type(params.first, params.second));
    }
    //ѕолучить случайное число в заданном интервале
    float get()
    {
        return dist(gen);
    }
};