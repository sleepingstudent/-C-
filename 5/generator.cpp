#include "anealing.h"

namespace anealing
{
    void test_generator(std::string &&out_file, size_t num_cpu, size_t num_task, size_t min_duration, size_t max_duration)
    {
        if (!num_task || !num_cpu)
        {
            return;
        }

        std::vector<size_t> times(num_task);

        for (int i = 0; i < num_task; ++i)
        {
            times[i] = rand() % (max_duration - min_duration + 1) + min_duration;
        }

        std::ofstream out_f;

        out_f.open(out_file);

        if (out_f.is_open())
        {

            // first line contain numcpu,num_task
            // second line contains durations of all tasks
            // csv separator - comma

            out_f << num_cpu << ',' << num_task << '\n';
            out_f << times[0];

            for (int i = 1; i < num_task; ++i)
            {
                out_f << "," << times[i];
            }

            out_f << "\n";

            out_f.close();
        }
    }
}

/*
int main()
{
    anealing::test_generator("test_input", 10, 100, 1, 1000);
    return 0;
}
*/