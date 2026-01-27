#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <string>
#include <algorithm>


class CustomMutex {
private:
    std::atomic<bool> locked{ false };

public:
    void lock() {
        bool expected = false;
        while (!locked.compare_exchange_weak(expected, true,
            std::memory_order_acquire, std::memory_order_relaxed)) {
            expected = false;
            std::this_thread::yield();
        }
    }

    void unlock() {
        locked.store(false, std::memory_order_release);
    }
};

const int EXPLOSION_LIMIT = 10000;
const int INITIAL_DISH = 3000;
const int NUM_FAT_MEN = 3;
const int SIMULATION_TIME_SEC = 5;

CustomMutex mtx;

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

            mtx.lock();

            if (!running) {
                mtx.unlock();
                break;
            }

            for (int i = 0; i < NUM_FAT_MEN; ++i) {
                dishes[i] += efficiency;
            }

            mtx.unlock();
        }
        });

    std::vector<std::thread> eaters;
    for (int id = 0; id < NUM_FAT_MEN; ++id) {
        eaters.emplace_back([&, id]() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

                mtx.lock();

                if (!running) {
                    mtx.unlock();
                    break;
                }

                if (eaten[id] >= EXPLOSION_LIMIT) {
                    mtx.unlock();
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
                }

                mtx.unlock();
            }
            });
    }

    std::this_thread::sleep_for(std::chrono::seconds(SIMULATION_TIME_SEC));
    
    {
        mtx.lock();
        running = false;
        mtx.unlock();
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
        std::string description;
    };

    std::vector<Test> tests = {
        {80, 5,    "Умеренная жадность, медленный повар"},
        {100, 1000, "Высокая жадность, очень быстрый повар"},
        {1, 2,     "Низкая жадность, медленный повар"}
    };

    std::cout << "============================================\n";
    std::cout << "Лабораторная работа: Собственный мьютекс\n";
    std::cout << "Моделирование трех толстяков\n";
    std::cout << "============================================\n\n";

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "\n============================================\n";
        std::cout << "Тест " << (i + 1) << ": " << tests[i].description << "\n";
        std::cout << "GLUTTONY = " << tests[i].gluttony
            << ", EFFICIENCY = " << tests[i].efficiency << "\n";
        std::cout << "--------------------------------------------\n";

        Result res = run_simulation(tests[i].gluttony, tests[i].efficiency);

        std::cout << "Исход: " << res.outcome << "\n\n";
        std::cout << "Результаты по толстякам:\n";

        int total_eaten = 0;
        int total_left = 0;
        bool all_exploded = true;

        for (int j = 0; j < NUM_FAT_MEN; ++j) {
            std::cout << "  Толстяк #" << (j + 1)
                << ": съел " << res.eaten[j] << " из " << EXPLOSION_LIMIT
                << ", осталось в тарелке: " << res.dishes[j] << "\n";

            total_eaten += res.eaten[j];
            total_left += res.dishes[j];

            if (res.eaten[j] < EXPLOSION_LIMIT) {
                all_exploded = false;
            }
        }

        std::cout << "\nСуммарная статистика:\n";
        std::cout << "  Всего съедено: " << total_eaten << "\n";
        std::cout << "  Всего осталось: " << total_left << "\n";
        std::cout << "  Изначальная еда: " << (INITIAL_DISH * NUM_FAT_MEN) << "\n";

        // Проверка корректности
        std::cout << "\nПроверка корректности:\n";
        if ((total_eaten + total_left) == (INITIAL_DISH * NUM_FAT_MEN)) {
            std::cout << "  ✓ Сумма еды сохранилась корректно\n";
        }
        else {
            std::cout << "  ✗ Ошибка: сумма еды не соответствует начальной!\n";
        }

        if (res.outcome == "Все съели и взорвались" && !all_exploded) {
            std::cout << "  ✗ Ошибка: заявлен взрыв всех, но не все достигли лимита!\n";
        }
        else if (res.outcome == "Все съели и взорвались" && all_exploded) {
            std::cout << "  ✓ Все толстяки действительно достигли лимита\n";
        }
    }

    std::cout << "\n============================================\n";
    std::cout << "Все тесты завершены. Собственный мьютекс работает корректно.\n";
    std::cout << "============================================\n";

    return 0;
}