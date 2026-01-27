#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <string>
#include <algorithm>

const int EXPLOSION_LIMIT = 10000;
const int INITIAL_DISH = 3000;
const int NUM_FAT_MEN = 3;
const int SIMULATION_TIME_SEC = 5;

std::mutex mtx;

struct Result {
    std::string outcome;
    std::vector<int> eaten;
    std::vector<int> dishes;
};

Result run_simulation(int gluttony, int efficiency) {
    std::vector<int> dishes(NUM_FAT_MEN, INITIAL_DISH);
    std::vector<int> eaten(NUM_FAT_MEN, 0);
    bool running = true;
    std::string outcome = "Не все съели";

    auto cook = std::thread([&]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::lock_guard<std::mutex> lock(mtx);
            if (!running) break;
            for (int i = 0; i < NUM_FAT_MEN; ++i) {
                dishes[i] += efficiency;
            }
        }
        });
    
    std::vector<std::thread> eaters;
    for (int id = 0; id < NUM_FAT_MEN; ++id) {
        eaters.emplace_back([&, id]() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                std::lock_guard<std::mutex> lock(mtx);
                if (!running) break;

                if (eaten[id] >= EXPLOSION_LIMIT) {
                    continue;
                }

                int room = EXPLOSION_LIMIT - eaten[id];
                int available = dishes[id];
                int take = std::min({ gluttony, room, available });

                if (take > 0) {
                    dishes[id] -= take;
                    eaten[id] += take;

                    if (dishes[id] <= 0) {
                        outcome = "Еда закончилась";
                        running = false;
                        break;
                    }
                }

                bool all_exploded = true;
                for (int i = 0; i < NUM_FAT_MEN; ++i) {
                    if (eaten[i] < EXPLOSION_LIMIT) {
                        all_exploded = false;
                        break;
                    }
                }
                if (all_exploded) {
                    outcome = "Все съели и взорвались";
                    running = false;
                    break;
                }
            }
            });
    }

    std::this_thread::sleep_for(std::chrono::seconds(SIMULATION_TIME_SEC));

    {
        std::lock_guard<std::mutex> lock(mtx);
        running = false;
    }

    cook.join();
    for (auto& t : eaters) {
        t.join();
    }

    return { outcome, eaten, dishes };
}

int main() {
#ifdef _WIN32
    system("chcp 1251 > nul");
#endif

    struct Test {
        int gluttony;
        int efficiency;
    };

    std::vector<Test> tests = {
        {80, 5},      // Медленный повар, умеренная жадность
        {100, 1000},  // Быстрый повар, высокая жадность
        {1, 2}        // Очень медленный едок, медленный повар
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "\nТест " << (i + 1) << ":\n";
        std::cout << "GLUTTONY = " << tests[i].gluttony
            << ", EFFICIENCY = " << tests[i].efficiency << "\n";

        Result res = run_simulation(tests[i].gluttony, tests[i].efficiency);

        std::cout << "Исход: " << res.outcome << "\n";
        std::cout << "Результаты:\n";

        int total_eaten = 0;
        int total_left = 0;

        for (int j = 0; j < NUM_FAT_MEN; ++j) {
            std::cout << "Толстяк #" << (j + 1)
                << ": съел " << res.eaten[j]
                << ", в тарелке " << res.dishes[j] << "\n";
            total_eaten += res.eaten[j];
            total_left += res.dishes[j];
        }

        std::cout << "Всего съедено: " << total_eaten << "\n";
        std::cout << "Всего осталось: " << total_left << "\n";
        std::cout << "Изначальная еда: " << (INITIAL_DISH * NUM_FAT_MEN) << "\n";

       
        if (res.outcome == "Все съели и взорвались") {
            bool all_at_limit = true;
            for (int eaten : res.eaten) {
                if (eaten < EXPLOSION_LIMIT) {
                    all_at_limit = false;
                    break;
                }
            }
            if (!all_at_limit) {
                std::cout << "ВНИМАНИЕ: результат 'Все взорвались', но не все достигли лимита!\n";
            }
        }
    }

    return 0;
}