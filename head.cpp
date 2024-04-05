#include <queue>
#include <iostream>



int main(){
    std::priority_queue<std::pair<int, std::pair<int, int>>, std::vector<std::pair<int, std::pair<int, int>>>, std::greater<std::pair<int, std::pair<int, int>>> > pq;
    pq.push({30, {20, 10}});
    pq.push({30, {10, 10}});
    pq.push({30, {40, 10}});
    pq.push({30, {10, 5}});
    std::pair<int, std::pair<int, int>> p;
    p = pq.top();
    std::cout<< p.first<< " "<< p.second.first<< " " << p.second.second;
    return 0;
}