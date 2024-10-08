#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <type_traits>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <future>

namespace anealing
{
    class Schedule_abstr
    {
    protected:
        size_t cpu_cnt_;
        std::vector<std::vector<size_t>> schedule_;
        std::unordered_map<size_t, size_t> task_time_;

    public:
        virtual size_t get_time() = 0;
        virtual size_t get_proc_num() const = 0;
        virtual std::vector<size_t> get_proc_tasks(size_t proc) const = 0;
        virtual std::vector<std::vector<size_t>> get_schedule() const = 0;
        virtual std::vector<size_t> task_per_cpu() const = 0;
        virtual std::vector<size_t> time_per_cpu() = 0;
        virtual void set_schedule(std::vector<std::vector<size_t>> &schedule) = 0;
    };

    class Schedule : public Schedule_abstr
    {
    public:
        Schedule(size_t cpu_cnt, const std::unordered_map<size_t, size_t> &task_time);
        Schedule();
        size_t get_time();
        size_t get_proc_num() const;
        std::vector<size_t> get_proc_tasks(size_t proc) const;
        std::vector<std::vector<size_t>> get_schedule() const;
        std::vector<size_t> task_per_cpu() const;
        std::vector<size_t> time_per_cpu();
        void set_schedule(std::vector<std::vector<size_t>> &schedule);

        bool operator==(const Schedule& second) const;
    };

    template <class Schedule_>
    class Mutation_abstr
    {
    public:
        virtual Schedule_ transform(Schedule_ cur_schedule) = 0;
    };

    template <class T>
    class Mutation : public Mutation_abstr<T>
    {
    public:
        T transform(T cur_schedule)
        {
            // mutation operation - move task from one cpu to another
            if (cur_schedule.get_proc_num() < 2)
            {
                return cur_schedule;
            }

            // T next_schedule;

            for (int i = 0; i < 5; ++i)
            {

                size_t proc = rand() % cur_schedule.get_proc_num();
                auto cur_cpu = cur_schedule.get_proc_tasks(proc);

                while (!cur_cpu.size())
                {
                    proc = rand() % cur_schedule.get_proc_num();
                    cur_cpu = cur_schedule.get_proc_tasks(proc);
                }

                auto new_schedule = cur_schedule.get_schedule();
                size_t pos_to_move = rand() % cur_cpu.size();
                size_t task = new_schedule[proc][pos_to_move];

                new_schedule[proc].erase(new_schedule[proc].begin() + pos_to_move);

                size_t new_proc = rand() % cur_schedule.get_proc_num();

                while (new_proc == proc)
                {
                    new_proc = rand() % cur_schedule.get_proc_num();
                }

                new_schedule[new_proc].push_back(task);

                // next_schedule = cur_schedule;

                // next_schedule.set_schedule(new_schedule);
                cur_schedule.set_schedule(new_schedule);
            }

            return cur_schedule;
        }
    };

    class Temp_change_abstr
    {
    protected:
        double temperature_;
        double start_temperature_;

    public:
        virtual double temprature_step(size_t iteration) = 0;
        double get_temprature() { return temperature_; }
    };

    class Boltzman : public Temp_change_abstr
    {
    public:
        Boltzman(double start_temperature = 0);
        double temprature_step(size_t iteration);
    };

    class Cauchy : public Temp_change_abstr
    {
    public:
        Cauchy(double start_temperature = 0);
        double temprature_step(size_t iteration);
    };

    class LogDiv : public Temp_change_abstr
    {
    public:
        LogDiv(double start_temperature = 0);
        double temprature_step(size_t iteration);
    };

    class Temperature
    {
        size_t temp_law_;
        Boltzman boltzman_law;
        Cauchy cauchy_law;
        LogDiv logdiv_law;

    public:
        Temperature(size_t temp_law, double start_temp);
        double temprature_step(size_t iteration);
    };

    template <class T>
    class Anealing
    {
        double start_temp_;
        size_t temp_law_;
        size_t it_without_change_;
        size_t temp_it_;
        T start_schedule_;

        size_t best_time_;
        T best_schedule_;

    public:
        Anealing(double start_temp, size_t temp_law, size_t it_without_change, size_t temp_it, T &start_schedule) : start_temp_(start_temp), temp_law_(temp_law), it_without_change_(it_without_change), temp_it_(temp_it), start_schedule_(start_schedule) {}

        void mainloop(T& global_best_schedule, std::mutex& mtx)
        {
            if (start_schedule_.get_proc_num() < 2)
            {
                return;
            }

            int cur_it_without_change{0};
            int iteration{0};

            Temperature temp_change(temp_law_, start_temp_);
            T cur_schedule = start_schedule_;
            size_t cur_time = cur_schedule.get_time();
            Mutation<T> mutator;

            best_schedule_ = start_schedule_;
            best_time_ = start_schedule_.get_time();

            while (cur_it_without_change < it_without_change_)
            {
                ++iteration;

                double cur_temp = temp_change.temprature_step(iteration);

                for (int it = 0; it < temp_it_; ++it)
                {
                    T new_schedule = mutator.transform(cur_schedule);

                    size_t new_time = new_schedule.get_time();

                    if (new_time < cur_time)
                    {
                        // best_time_ = new_time;
                        // best_schedule_ = new_schedule;
                        // cur_schedule = new_schedule;
                        // cur_time = new_time;
                        // cur_it_without_change = 0;
                        if (new_time < best_time_)
                        {
                            cur_it_without_change = 0;
                            best_time_ = new_time;
                            best_schedule_ = new_schedule;
                        }

                        cur_time = new_time;
                        cur_schedule = new_schedule;
                    }
                    else
                    {
                        cur_it_without_change++;

                        if (cur_time == new_time){
                            continue;
                        }

                        double prob = (double)rand() / RAND_MAX;

                        double d = (double)new_time - cur_time;

                        if (prob < exp(-d / cur_temp))
                        {
                            cur_time = new_time;
                            cur_schedule = new_schedule;
                        }
                    }
                }
            }


            mtx.lock();
            if (global_best_schedule.get_time() > best_schedule_.get_time()){
                global_best_schedule = best_schedule_;
            }
            mtx.unlock();

            //return best_schedule_;
        }
    };

    template <class T>
    class First_schedule
    {
    public:
        First_schedule() {}

        T generate_schedule(int num_cpu, int num_task, std::unordered_map<size_t, size_t> &exec_time)
        {
            std::vector<std::vector<size_t>> task_on_cpu(num_cpu);

            for (int i = 0; i < num_task; ++i)
            {
                task_on_cpu[rand() % num_cpu].push_back(i);
            }

            T start_schedule(num_cpu, exec_time);
            start_schedule.set_schedule(task_on_cpu);

            return start_schedule;
        }

        T generate_dummy_schedule(int num_cpu, int num_task, std::unordered_map<size_t, size_t> &exec_time)
        {
            std::vector<std::vector<size_t>> task_on_cpu(num_cpu);

            for (int i = 0; i < num_task; ++i)
            {
                task_on_cpu[0].push_back(i);
            }

            T start_schedule(num_cpu, exec_time);
            start_schedule.set_schedule(task_on_cpu);

            return start_schedule;
        }
    };

    struct data
    {
        size_t num_cpu;
        size_t num_task;
        std::unordered_map<size_t, size_t> exec_time;

        data() {}
    };

    data read_data(std::string &&input_file);

    void test_generator(std::string &&out_file, size_t num_cpu, size_t num_task, size_t min_duration, size_t max_duration);

}