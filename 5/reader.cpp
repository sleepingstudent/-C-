#include "anealing.h"

namespace anealing{
    data read_data(std::string &&input_file){
        std::ifstream file;

        file.open(input_file);
        
        data d;

        if (file.is_open()){
            char tmp;

            file >> d.num_cpu >> tmp >> d.num_task;
            
            for (int i = 0; i < d.num_task; ++i){
                size_t cur_val;

                file >> cur_val;

                if (i != d.num_task - 1){
                    file >> tmp;
                }

                d.exec_time[i] = cur_val;
            }

            file.close();
        }
        
        return d;
    }
} 
/*
int main(){
    auto tmp = anealing::read_data("test_input");

    std::cout << tmp.num_cpu << ' ' << tmp.num_task << '\n';

    for (int i = 0; i < tmp.num_task; ++i){
        std::cout << i << ' ' << tmp.exec_time[i] << '\n';
    }
}
*/