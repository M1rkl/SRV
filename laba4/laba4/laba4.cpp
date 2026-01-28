#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>
#include <algorithm>

const int EXPLOSION_LIMIT = 10000;

const int GLUTTONY = 10;
const int EFFICIENCY_FACTOR = 11;

std::vector<int> dishes = { 3000, 3000, 3000 };
std::vector<int> eaten_food = { 0, 0, 0 };
std::mutex shared_mutex;
std::atomic<bool> simulation_running(true);
std::string outcome = "Кук уволился сам! (Прошло 5 дней)";

void cook_thread() {
    while (simulation_running) {
        shared_mutex.lock();
        if (simulation_running) {
            for (int i = 0; i < 3; ++i) {
                dishes[i] += EFFICIENCY_FACTOR;
            }
        }
        shared_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void fat_man_thread(int id) {
    while (simulation_running) {
        shared_mutex.lock();
        if (!simulation_running) {
            shared_mutex.unlock();
            break;
        }

        if (eaten_food[id] >= EXPLOSION_LIMIT) {
            shared_mutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        int room_in_stomach = EXPLOSION_LIMIT - eaten_food[id];
        int food_to_eat = std::min({ dishes[id], GLUTTONY, room_in_stomach });

        if (food_to_eat > 0) {
            dishes[id] -= food_to_eat;
            eaten_food[id] += food_to_eat;

            if (dishes[id] <= 0) {
                outcome = "Кука уволили! (Тарелка опустела)";
                simulation_running = false;
            }
        }

        if (eaten_food[0] >= EXPLOSION_LIMIT &&
            eaten_food[1] >= EXPLOSION_LIMIT &&
            eaten_food[2] >= EXPLOSION_LIMIT) {
            outcome = "Кук не получил зарплату! (Все толстяки лопнули)";
            simulation_running = false;
        }

        shared_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

struct Test {
    int gluttony;
    int efficiency_factor;
};

int main() {
    setlocale(LC_ALL, "Russian");

    std::vector<Test> tests = {
        {80, 5},
        {100, 1000},
        {1, 2}
    };

    std::cout << "Симуляция запущена с параметрами:" << std::endl;
    std::cout << "Обжорство (Толстяки едят): " << GLUTTONY << " еды/раз" << std::endl;
    std::cout << "Эффективность (Повар готовит): " << EFFICIENCY_FACTOR << " еды/партия на тарелку" << std::endl;

    std::thread cook(cook_thread);
    std::vector<std::thread> fat_men;
    for (int i = 0; i < 3; ++i) {
        fat_men.emplace_back(fat_man_thread, i);
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
    if (simulation_running) {
        simulation_running = false;
    }

    cook.join();
    for (auto& t : fat_men) {
        t.join();
    }

    std::cout << "Симуляция завершена!" << std::endl;
    std::cout << "ИТОГ: " << outcome << std::endl;
    std::cout << "Финальное состояние:" << std::endl;
    for (int i = 0; i < 3; ++i) {
        std::cout << "Толстяк #" << i + 1 << " съел: " << eaten_food[i]
            << " еды. | На тарелке #" << i + 1 << " осталось: " << dishes[i]
            << " еды." << std::endl;
    }

    return 0;
}
