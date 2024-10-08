#include "anealing.h"

namespace anealing
{
    Schedule::Schedule(size_t cpu_cnt, const std::unordered_map<size_t, size_t> &task_time)
    {
        cpu_cnt_ = cpu_cnt;
        schedule_.resize(cpu_cnt_);
        task_time_ = task_time;
    }
    Schedule::Schedule() {}

    size_t Schedule::get_time()
    {
        size_t time = 0;
        size_t f_time = 0;
        for (auto &cpu : schedule_)
        {
            size_t cur_time = 0;
            

            for (auto task : cpu)
            {
                cur_time += task_time_[task];
            }

            // std::cout << cur_time << ' ';

            time = std::max(time, cur_time);
            
            if (!cpu.empty()) {
                size_t cur_f_time = task_time_[cpu.front()];
                if (f_time == 0) {
                    f_time = cur_f_time;
                } else {
                    f_time = std::min(f_time, cur_f_time);
                }
            }
        }

        // std::cout << '\n';

        return time - f_time;
    }

    size_t Schedule::get_proc_num() const
    {
        return cpu_cnt_;
    }

    std::vector<size_t> Schedule::task_per_cpu() const
    {
        std::vector<size_t> task_cpu;

        for (int i = 0; i < cpu_cnt_; ++i)
        {
            task_cpu.push_back(schedule_[i].size());
        }

        return task_cpu;
    }

    std::vector<size_t> Schedule::time_per_cpu()
    {
        std::vector<size_t> task_cpu;

        for (auto& cpu:  schedule_)
        {
            size_t cnt{0};

            for (auto task : cpu){
                cnt += task_time_[task];
            }

            task_cpu.push_back(cnt);
        }

        return task_cpu;
    }

    std::vector<size_t> Schedule::get_proc_tasks(size_t proc) const
    {
        return schedule_[proc];
    }

    std::vector<std::vector<size_t>> Schedule::get_schedule() const
    {
        return schedule_;
    }

    void Schedule::set_schedule(std::vector<std::vector<size_t>> &schedule)
    {
        schedule_ = schedule;
    }

    Boltzman::Boltzman(double start_temperature)
    {
        temperature_ = start_temperature_ = start_temperature;
    }

    double Boltzman::temprature_step(size_t iteration)
    {
        temperature_ = (double)start_temperature_ / std::log(1 + (iteration + 1));
        return temperature_;
    }

    Cauchy::Cauchy(double start_temperature)
    {
        temperature_ = start_temperature_ = start_temperature;
    }

    double Cauchy::temprature_step(size_t iteration)
    {
        temperature_ = (double)start_temperature_ / (1 + iteration);
        return temperature_;
    }

    LogDiv::LogDiv(double start_temperature)
    {
        temperature_ = start_temperature_ = start_temperature;
    }

    double LogDiv::temprature_step(size_t iteration)
    {
        temperature_ = (double)start_temperature_ * std::log(1 + (iteration + 1)) / (1 + (iteration + 1));
        return temperature_;
    }

    Temperature::Temperature(size_t temp_law, double start_temp) : temp_law_(temp_law)
    {
        if (temp_law_ == 1)
        {
            boltzman_law = Boltzman(start_temp);
        }
        else if (temp_law_ == 2)
        {
            cauchy_law = Cauchy(start_temp);
        }
        else
        {
            logdiv_law = LogDiv(start_temp);
        }
    }

    double Temperature::temprature_step(size_t iteration)
    {
        if (temp_law_ == 1)
        {
            return boltzman_law.temprature_step(iteration);
        }
        else if (temp_law_ == 2)
        {
            return cauchy_law.temprature_step(iteration);
        }
        else
        {
            return logdiv_law.temprature_step(iteration);
        }
    }

    bool Schedule::operator==(const Schedule& second) const{
        return std::tie(cpu_cnt_, schedule_, task_time_) == std::tie(second.cpu_cnt_, second.schedule_, second.task_time_);
    }
}