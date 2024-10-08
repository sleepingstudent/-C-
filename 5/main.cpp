#include "anealing.h"
#include <cstring>

static size_t NUM_THREAD = 1;

int main(int argc, char **argv)
{
    srand(time(0));

    if (!strcmp(argv[1], "-g") || !strcmp(argv[1], "--generate"))
    {
        // (-g/--generate), output file, cpu number, task number, minimal task duration, maximal task duration
        anealing::test_generator(argv[2], std::stoi(argv[3]), std::stoi(argv[4]), std::stoi(argv[5]), std::stoi(argv[6]));
    }
    else if (!strcmp(argv[1], "-c") || !strcmp(argv[1], "--compute"))
    {
        // (-c/--generate), input file, number of threads
        auto d = anealing::read_data(argv[2]);
        NUM_THREAD = std::stoi(argv[3]);

        anealing::First_schedule<anealing::Schedule> sch_gen;
        anealing::Schedule start_schedule = sch_gen.generate_schedule(d.num_cpu, d.num_task, d.exec_time);

        std::cout << "Start time: " << start_schedule.get_time() << "\n";

        int starts_without_change{};
        anealing::Schedule best_schedule = start_schedule;
        std::mutex mtx;

        auto begin = std::chrono::high_resolution_clock::now();

        while (starts_without_change < 5)
        {
            // args: start_temp, temp_law, it_without_change, temp_it, start_schedule
            std::vector<anealing::Anealing<anealing::Schedule>> main_proc_v;

            for (int i = 0; i < NUM_THREAD; ++i)
            {
                main_proc_v.push_back(anealing::Anealing<anealing::Schedule>(100, 1, 100, 5, start_schedule));
            }

            std::vector<std::thread> threads;

            for (int i = 0; i < NUM_THREAD; ++i)
            {
                threads.push_back(std::thread{[&best_schedule, &main_proc_v, &mtx, i]
                                              { main_proc_v[i].mainloop(best_schedule, mtx); }});
            }

            for (int i = 0; i < NUM_THREAD; ++i)
            {
                threads[i].join();
            }

            if (start_schedule == best_schedule)
            {
                starts_without_change++;
            }
            else
            {
                starts_without_change = 0;
                start_schedule = best_schedule;
            }

            std::cout << "Current target function: " << best_schedule.get_time() << "\n";
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();

        std::cout << "Result!\n";

        // std::cout << "Time per cpu: ";
        // for (auto it : best_schedule.time_per_cpu())
        // {
        //     std::cout << it << " ";
        // }
        // std::cout << "\n";

        std::cout << "Result time: " << best_schedule.get_time() << "\nComputation time: " << duration / 1e9 << "\n";
    }

    // for (int n = 1; n <= 6; ++n)
    // {
    //     long double duration{};
    //     anealing::Schedule best_schedule;

    //     for (int i = 0; i < 3; ++i)
    //     {
    //         auto d = anealing::read_data("test_data/parallel_test_big");
    //         anealing::First_schedule<anealing::Schedule> sch_gen;
    //         anealing::Schedule start_schedule = sch_gen.generate_schedule(d.num_cpu, d.num_task, d.exec_time);

    //         int starts_without_change{};
    //         best_schedule = start_schedule;
    //         std::mutex mtx;

    //         auto begin = std::chrono::high_resolution_clock::now();

    //         while (starts_without_change < 5)
    //         {
    //             // args: start_temp, temp_law, it_without_change, temp_it, start_schedule
    //             std::vector<anealing::Anealing<anealing::Schedule>> main_proc_v;

    //             for (int i = 0; i < NUM_THREAD; ++i)
    //             {
    //                 main_proc_v.push_back(anealing::Anealing<anealing::Schedule>(100, 1, 100, 5, start_schedule));
    //             }

    //             std::vector<std::thread> threads;

    //             for (int i = 0; i < NUM_THREAD; ++i)
    //             {
    //                 threads.push_back(std::thread{[&best_schedule, &main_proc_v, &mtx, i]
    //                                               { main_proc_v[i].mainloop(best_schedule, mtx); }});
    //             }

    //             for (int i = 0; i < NUM_THREAD; ++i)
    //             {
    //                 threads[i].join();
    //             }

    //             if (start_schedule == best_schedule)
    //             {
    //                 starts_without_change++;
    //             }
    //             else
    //             {
    //                 starts_without_change = 0;
    //                 start_schedule = best_schedule;
    //             }
    //         }

    //         auto end = std::chrono::high_resolution_clock::now();
    //         duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / 1e9;
    //     }

    //     std::cout << n << ' ' << duration / 3 << " " << best_schedule.get_time() << "\n";
    // }
}