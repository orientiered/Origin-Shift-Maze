#pragma once
#include <random>
//��������� �����-������ ��� �������� ������������� ��������� ����� �����
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

    //������ �������� ������������ ����� [a,b]
    void set(std::pair<int, int> params)
    {
        dist.param(std::uniform_int_distribution<int>::param_type(params.first, params.second));
    }
    //�������� ��������� ����� � �������� ���������
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

    //������ �������� ������������ ����� [a,b]
    void set(std::pair<float, float> params)
    {
        dist.param(std::uniform_real_distribution<float>::param_type(params.first, params.second));
    }
    //�������� ��������� ����� � �������� ���������
    float get()
    {
        return dist(gen);
    }
};