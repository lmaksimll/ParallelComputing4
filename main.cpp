#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <atomic>

using namespace std;

int n = 50000000;
vector <int> t(4*n);    //1 поток
vector <int> g(4*n);    //Много потоков

void array_initialization(vector<int>& a)
{
    random_device rd;                               // non-deterministic generator
    mt19937 gen(rd());                              // to seed mersenne twister.
    uniform_int_distribution<> dist(0,500000);      // distribute results between 1 and 6 inclusive.

    for (int i = 0; i < n; ++i) {
        a[i] = dist(gen);
    }
}

/*-------------------------------функции для 1 потока----------------------------------*/

void build (vector <int> &a, int v, int tl, int tr) {
    if (tl == tr)
        t[v] = a[tl];
    else {
        int tm = (tl + tr) / 2;
        build (a, v*2, tl, tm);
        build (a, v*2+1, tm+1, tr);
        t[v] = t[v*2] + t[v*2+1];
    }
}

int sum (int v, int tl, int tr, int l, int r) {
    if (l > r)
        return 0;
    if (l == tl && r == tr)
        return t[v];
    int tm = (tl + tr) / 2;
    return sum (v*2, tl, tm, l, min(r,tm))
        + sum (v*2+1, tm+1, tr, max(l,tm+1), r);
}

void one_stream(vector<int>& a)
{
    //Время начала работы
    clock_t tStart = clock();

    build(a,1,0,n-1);

    int res1 = sum(1,0,n-1,10,30);
    int res2 = 0;

    for (int j = 10; j <= 30; ++j) {
        res2 += a[j];
    }

    //Вывод результата
    cout << "---Result 1 stream---" << endl;
    cout << res1 << " - Sum" << endl;
    cout << res2 << " - Cycle" << endl;
    cout << "Time taken: " << (double)(clock() - tStart)/CLOCKS_PER_SEC << "s" << endl;
    cout << "------------" << endl;
}

/*---------------------------функции для множества потоков-----------------------------*/

void build_stream (const vector <int> &a, int v, int tl, int tr,int thread_count) {
    if (tl == tr)
        g[v] = a[tl];
    else {
        int tm = (tl + tr) / 2;

        if (thread_count <= 4)
        {
//            #pragma omp atomic
//            thread_count++;
//            cout<<"TC:" << thread_count << endl;
#pragma omp taskgroup
            {
                #pragma omp task default(shared)
                {
                    build_stream (a, v*2, tl, tm, thread_count+1);
                }

                #pragma omp task default(shared)
                {
                    build_stream (a, v*2+1, tm+1, tr, thread_count+1);
                }
//                #pragma omp taskwait
            }
        }
        else
        {
            build_stream (a, v*2, tl, tm, thread_count);
            build_stream (a, v*2+1, tm+1, tr, thread_count);
        }

        g[v] = g[v*2] + g[v*2+1];
    }
}

int sum_stream (int v, int tl, int tr, int l, int r) {
    if (l > r)
        return 0;
    if (l == tl && r == tr)
        return g[v];
    int tm = (tl + tr) / 2;
    return sum (v*2, tl, tm, l, min(r,tm))
        + sum (v*2+1, tm+1, tr, max(l,tm+1), r);
}

void more_stream(vector<int>& a)
{
    //Время начала работы
    clock_t tStart = clock();

    int thread_count = 0;

    #pragma omp parallel
    {
#pragma omp single
        build_stream(a,1,0,n-1,thread_count);
    }

    int res1 = sum_stream(1,0,n-1,10,30);
    int res2 = 0;

    for (int j = 10; j <= 30; ++j) {
        res2 += a[j];
    }

    //Вывод результата
    cout << "---Result more stream---" << endl;
    cout << res1 << " - Sum" << endl;
    cout << res2 << " - Cycle" << endl;
    cout << "Time taken: " << (double)(clock() - tStart)/CLOCKS_PER_SEC << "s" << endl;
    cout << "------------" << endl;
}

int main()
{
    vector <int> a(n);

    array_initialization(a);

    //a - отрезок на основе которого строим
    //v - текущая вершина
    //tl и tr - текущ. границы дерева

    //1 поток:
    one_stream(a);

    //Несколько потоков
    more_stream(a);
}


