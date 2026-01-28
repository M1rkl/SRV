#include <coroutine>
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <cstdlib>

struct promise_type_1 {
    int value = 0;

    auto get_return_object() {
        return std::coroutine_handle<promise_type_1>::from_promise(*this);
    }
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() noexcept {}
    void unhandled_exception() {}

    std::suspend_always yield_value(int v) noexcept {
        value = v;
        return {};
    }
};

struct coroutine_1 {
    using promise_type = promise_type_1;

    std::coroutine_handle<promise_type_1> handle;

    coroutine_1(std::coroutine_handle<promise_type_1> h) : handle(h) {}
    ~coroutine_1() { if (handle) handle.destroy(); }

    void resume() { handle.resume(); }
    bool done() const { return handle.done(); }
    int get_value() const { return handle.promise().value; }
};

coroutine_1 example_coroutine() {
    co_yield 10;
    co_yield 25;
    co_yield 46;  
    co_yield 77;
}

struct promise_type_2 {
    int current_value = 0;

    auto get_return_object() {
        return std::coroutine_handle<promise_type_2>::from_promise(*this);
    }
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() noexcept {}
    void unhandled_exception() {}

    std::suspend_always yield_value(int value) noexcept {
        current_value = value;
        return {};
    }
};

struct task {
    using promise_type = promise_type_2;

    std::coroutine_handle<promise_type_2> handle;

    task(std::coroutine_handle<promise_type_2> h) : handle(h) {}
    ~task() { if (handle) handle.destroy(); }

    void resume() { handle.resume(); }
    bool done() const { return handle.done(); }
    int get_value() const { return handle.promise().current_value; }
};

task long_computation(int steps) {
    for (int i = 1; i <= steps; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        co_yield i;
    }
    co_return;
}

void print_progress(int current, int total) {
    float percent = static_cast<float>(current) / total * 100.f;
    int bar_width = 50;
    std::string bar;

    size_t pos = static_cast<size_t>(percent / 100.f * bar_width);
    std::string my_name = "Даниил";  

    for (size_t i = 0; i < bar_width; ++i) {
        if (i < pos) {
            bar += my_name[i % my_name.length()];
        }
        else {
            bar += ' ';
        }
    }

    std::cout << "\r[" << bar << "] " << static_cast<int>(percent) << "%";
    std::cout.flush();
}

int main() {
    system("chcp 1251>nul");

    std::cout << "Значения из корутины: ";
    coroutine_1 coro1 = example_coroutine();
    while (!coro1.done()) {
        coro1.resume();
        if (!coro1.done()) {
            std::cout << coro1.get_value() << " ";
        }
    }
    std::cout << "\n\n";

    const int TOTAL_STEPS = 100;
    task coro2 = long_computation(TOTAL_STEPS);
    while (!coro2.done()) {
        coro2.resume();
        if (coro2.done()) break;
        int progress = coro2.get_value();
        print_progress(progress, TOTAL_STEPS);
    }
    std::cout << "\n\nThe end." << std::endl;

    return 0;
}
