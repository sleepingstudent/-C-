#pragma once 

#include <vector>
#include <string>
#include <queue>
#include <set>
#include <map>

#include <thread>
#include <mutex>
#include <barrier>
#include <syncstream>

#include <filesystem>
#include <fstream>

#include <algorithm>
#include <random>
#include <concepts>
#include <ranges>

#include "new_ptr.hpp"


namespace fs = std::filesystem;
namespace view = std::ranges::views;

using namespace std::chrono_literals;

struct Message {
    int role;
    int msg;
    int kill = 0;
    std::set<int> killed = {};
};


enum Roles {
    MAFIA,
    DOCTOR,
    COMISSAR,
    MANIAC,
    CIVILLIAN,
    BULL,
    EXECUTIONER,
    NINJA,
    MASTER
};


auto TIME = 1ms;


std::map<int, std::string> role_names = {{0, "Mafia"}, 
    {1, "Doctor"}, 
    {2, "Comissar"}, 
    {3, "Maniac"}, 
    {4, "Civillian"}, 
    {5, "Bull"}, 
    {6, "Executioner"}, 
    {7, "Ninja"}};

std::queue<Message> com_q;
std::mutex com_mtx;


template <typename T>
concept PlayerConcept = requires(T* v, NewPtr<int> &num, NewPtr<std::set<int>> &names, int role, int &maf_num, std::queue<Message> &msg, std::mutex &mtx, NewPtr<std::barrier<void (*)()>> &bar, bool player, int my_role)
{
    {v->vote(maf_num, role, player, *names, msg, mtx, bar)} -> std::convertible_to<void>;
    {v->act(role, player, *names, msg, mtx)} -> std::convertible_to<void>;
    {v->play(num, names, role, maf_num, msg, mtx, bar, player, my_role)} -> std::convertible_to<void>;
};

template <typename T>
concept MstConcept = requires(T* v, NewPtr<int> &num, NewPtr<std::set<int>> &names, int role, int &maf_num, std::map<int, std::queue<Message>> &msg, std::map<int, std::mutex> &mtx, NewPtr<std::barrier<void (*)()>> &bar, bool player)
{
    {v->play(num, names, role, maf_num, msg, mtx, bar, player)} -> std::convertible_to<void>;
};


void on_completion() {}


class Player {
public:
    virtual ~Player() {}
    virtual void hello(int role, int id) {
        switch(role) {
            case MAFIA:
                std::osyncstream(std::cout) << "Your id= " << id << ". Ты владыка ночи, ибо ты МАФИЯ, уничтожь всех мирных!\n\n";
                break;
            case BULL:
                std::osyncstream(std::cout) << "Your id= " << id << ". Ты БЫК, уничтожь всех мирных! Даже меньяк тебе не страшен\n\n";
                break;
            case DOCTOR:
                std::osyncstream(std::cout) << "Your id= " << id << ". Ты ДОКТОР, спаситель тех, кто на грани из-за преступности! Лечи мирных\n\n";
                break;
            case COMISSAR:
                std::osyncstream(std::cout) << "Your id= " << id << ". Ты КОМИСАР, определяй и уничтожай злодеев!\n\n";
                break;
            case MANIAC:
                std::osyncstream(std::cout) << "Your id= " << id << ". Ты МАНЬЯК уничтожь всех, осталься один!\n\n";
                break; 
            case CIVILLIAN: 
                std::osyncstream(std::cout) << "Your id= " << id << ". Ты МИРНЫЙ голосуй против зла!\n\n" << std::endl;
                break;
            case EXECUTIONER: 
                std::osyncstream(std::cout) << "Your id= " << id << ". Ты ПАЛАЧ, прими верное решение и приговори зло к смерти!\n\n" << std::endl;
                break;
        }
    }
    virtual bool update(int &role, std::set<int> &names, std::queue<Message> &q, std::mutex &mtx) {
        while (true) {
            mtx.lock();
            while (q.empty()) {
                mtx.unlock();
                std::this_thread::sleep_for(TIME);
                mtx.lock();
            }

            if (q.front().role == MASTER)
                break;

            mtx.unlock();
            std::this_thread::sleep_for(TIME);
        }
        Message killed = q.front();
        q.pop();
        mtx.unlock();
        bool ans = false;
        for (auto &i : killed.killed) {
            if (i == role)
                ans = true;

            names.erase(i);
        }

        return ans;
    }
    virtual void vote(int &kicked, int role, bool player, std::set<int> &names, std::queue<Message> &msg, std::mutex &mtx, NewPtr<std::barrier<void (*)()>> &bar) {
        int choice = -1;

        if (player) {
            std::string pls = "Возможные игроки для голосования: [";

            for (auto &i : names)
                pls += std::to_string(i) + ", ";
            pls[pls.length() - 2] = ']', pls[pls.length() - 1] = '\n';
            do {
                std::osyncstream(std::cout) << pls << "Выбери из списка своего кандидата для голосования: ";
                std::cin >> choice;
            } while (!names.contains(choice));
        } else {
            int n = std::rand() % names.size();
            auto it = std::begin(names);
            std::advance(it,n);
            choice = *it;
        }

        mtx.lock();
        msg.push(Message{role, choice});
        mtx.unlock();

        if (role == EXECUTIONER) {
            std::this_thread::sleep_for(TIME);
            while (true) {
                mtx.lock();
                while (msg.empty()) {
                    mtx.unlock();
                    std::this_thread::sleep_for(TIME);
                    mtx.lock();
                }
                if (msg.front().role == MASTER)
                    break;

                mtx.unlock();
                std::this_thread::sleep_for(TIME);
            }
            std::set<int> to_kick = msg.front().killed;
            msg.pop();
            mtx.unlock();

            if (to_kick.size() > 1) {                
                if (player) {
                    to_kick.insert(-1);
                    std::string pls = "Игроки набрали равное количество голосов, выбери кто подозрительнее(-1 оставить всех): [";

                    for (auto &i : to_kick)
                        pls += std::to_string(i) + ", ";
                    pls[pls.length() - 2] = ']', pls[pls.length() - 1] = '\n';
                    do {
                        std::osyncstream(std::cout) << pls << "Выбери игрока, кто твой кандидат (-1 сохранить всех): ";
                        std::cin >> choice;
                    } while (!to_kick.contains(choice));
                } else {
                    int n = std::rand() % to_kick.size();
                    auto it = std::begin(to_kick);
                    std::advance(it,n);
                    choice = *it;
                }
            } else
                choice = *(to_kick.begin());

            mtx.lock();
            msg.push(Message{role, choice});
            mtx.unlock();

            std::this_thread::sleep_for(TIME);
        }

        std::this_thread::sleep_for(TIME);
        while (true) {
            mtx.lock();
            while (msg.empty()) {
                mtx.unlock();
                std::this_thread::sleep_for(TIME);
                mtx.lock();
            }
            if (msg.front().role == MASTER)
                    break;

            mtx.unlock();
            std::this_thread::sleep_for(TIME);
        }
        kicked = msg.front().msg;
        msg.pop();
        mtx.unlock();

        (*bar).arrive_and_wait();
    }
    virtual void act(int role, bool player, std::set<int> &names, std::queue<Message> &msg, std::mutex &mtx) {
        int choice = 0;

        if (player && role != CIVILLIAN && role != EXECUTIONER) {
            std::string pls = "Возможные игроки для выбора: [";

            for (auto &i : names)
                pls += std::to_string(i) + ", ";
            pls[pls.length() - 2] = ']', pls[pls.length() - 1] = '\n';
            std::osyncstream(std::cout) << pls << "Выбери игрока из списка: ";
            std::cin >> choice;
        } else {
            int n = std::rand() % names.size();
            auto it = std::begin(names);
            std::advance(it,n);
            choice = *it;
        }

        mtx.lock();
        msg.push(Message{role, choice});
        mtx.unlock();
    }
    virtual void play(NewPtr<int> &num,
                      NewPtr<std::set<int>> &names,
                      int role,
                      int &maf_num,
                      std::queue<Message> &msg,
                      std::mutex &mtx,
                      NewPtr<std::barrier<void (*)()>> &bar,
                      bool player,
                      int my_role) = 0;
    virtual bool is_end(std::queue<Message> &msg, std::mutex &mtx) {
        while (true) {
            mtx.lock();
            while (msg.empty()) {
                mtx.unlock();
                std::this_thread::sleep_for(TIME);
                mtx.lock();
            }
            if (msg.front().role == MASTER)
                    break;

            mtx.unlock();
            std::this_thread::sleep_for(TIME);
        }
        Message m = msg.front();
        msg.pop();
        mtx.unlock();

        return m.msg > 0;
    }
};

 
class Master {
    int man_id;
    int doctor_act = -1;
    int man_act = -1;
    int com_act = -1;
    int maf_act = -1;
    int exec_id = -1;

    std::set<int> maf_votes;

    std::map<int, int> players;
    std::map<int, int> day_votes;

    bool is_end(size_t num, int &maf_num);

    void kill(NewPtr<std::set<int>> &names, int &maf_num, std::map<int, std::queue<Message>> &q, std::map<int, std::mutex> &mtx, std::ofstream &ofs);
    void kick(std::set<int> &names, int &maf_num, std::map<int, std::queue<Message>> &q, std::map<int, std::mutex> &mtx, std::ofstream &ofs);
    void vote(std::set<int> &names, std::map<int, std::queue<Message>> &q, std::map<int, std::mutex> &mtx, std::ofstream &ofs);
    void act(std::set<int> &names, std::map<int, std::queue<Message>> &q, std::map<int, std::mutex> &mtx, std::ofstream &ofs);
    bool finish(NewPtr<int> &num, int &maf_num, std::set<int> &names, std::map<int, std::queue<Message>> &q, std::map<int, std::mutex> &mtx, bool end, std::ofstream &ofs);
public:
    void play(NewPtr<int> &num,
              NewPtr<std::set<int>> &names,
              int role,
              int &maf_num,
              std::map<int, std::queue<Message>> &msg,
              std::map<int, std::mutex> &mtx,
              NewPtr<std::barrier<void (*)()>> &bar,
              bool player);
};


class Mafia : public Player {
protected:
    int maf_n;
    int maf_num;

    std::set<int> victims;
    std::map<int, int> mafs;

    NewPtr<std::map<int, std::queue<Message>>> mafia_chat;
    NewPtr<std::map<int, std::mutex>> mafia_mtx;

    void meet(int role, int &maf_num, bool player);
    bool update(int &role, std::set<int> &names, std::queue<Message> &q, std::mutex &mtx) {
        while (true) {
            mtx.lock();
            while (q.empty()) {
                mtx.unlock();
                std::this_thread::sleep_for(TIME);
                mtx.lock();
            }
            if (q.front().role == MASTER)
                    break;

            mtx.unlock();
            std::this_thread::sleep_for(TIME);
        }
        Message killed = q.front();
        q.pop();
        mtx.unlock();
        bool ans = false;
        for (auto &i : killed.killed) {
            if (mafs.erase(i))
                --this->maf_num;

            if (i == role)
                ans = true;

            names.erase(i);
        }

        return ans;
    }
    void discuss(int role, NewPtr<int> &num, int &choice);
    void vote(int &kicked, int role, bool player, std::set<int> names, std::queue<Message> &msg, std::mutex &mtx, NewPtr<std::barrier<void (*)()>> &bar);
    void act(int role, int &maf_num, bool player, NewPtr<int> &num, std::queue<Message> &msg, std::mutex &mtx);
public:
    Mafia(int n,
          NewPtr<std::map<int, std::queue<Message>>> &mafia_chat,
          NewPtr<std::map<int, std::mutex>> &mafia_mtx) : maf_n(n), mafia_chat(mafia_chat), mafia_mtx(mafia_mtx) {}
    virtual ~Mafia() {}

    void play(NewPtr<int> &num,
              NewPtr<std::set<int>> &names,
              int role,
              int &maf_num,
              std::queue<Message> &msg,
              std::mutex &mtx,
              NewPtr<std::barrier<void (*)()>> &bar,
              bool player,
              int my_role = MAFIA);
};


class Bull : public Mafia {
public:
    Bull(int n,
          NewPtr<std::map<int, std::queue<Message>>> &mafia_chat,
          NewPtr<std::map<int, std::mutex>> &mafia_mtx) : Mafia (n, mafia_chat, mafia_mtx) {}
    virtual ~Bull() {}
    void play(NewPtr<int> &num,
              NewPtr<std::set<int>> &names,
              int role,
              int &maf_num,
              std::queue<Message> &msg,
              std::mutex &mtx,
              NewPtr<std::barrier<void (*)()>> &bar,
              bool player) { Mafia::play(num, names, role, maf_num, msg, mtx, bar, player, BULL); }
};


class Doctor : public Player {
    int chosen = -1;

    void act(int role, bool player, std::set<int> &names, std::queue<Message> &msg, std::mutex &mtx);
public:
    void play(NewPtr<int> &num,
              NewPtr<std::set<int>> &names,
              int role,
              int &maf_num,
              std::queue<Message> &msg,
              std::mutex &mtx,
              NewPtr<std::barrier<void (*)()>> &bar,
              bool player,
              int my_role = DOCTOR);
};


class Comissar : public Player { 
    std::set<int> unchecked;
    std::set<int> mafs;

    void act(int role, bool player, std::set<int> &names, std::queue<Message> &msg, std::mutex &mtx);
    bool update(int &role, std::set<int> &names, std::queue<Message> &q, std::mutex &mtx) {
        while (true) {
            mtx.lock();
            while (q.empty()) {
                mtx.unlock();
                std::this_thread::sleep_for(TIME);
                mtx.lock();
            }
            if (q.front().role == MASTER)
                break;

            mtx.unlock();
            std::this_thread::sleep_for(TIME);
        }
        Message killed = q.front();
        q.pop();
        mtx.unlock();

        bool ans = false;
        for (auto &i : killed.killed) {
            mafs.erase(i);

            if (i == role)
                ans = true;

            names.erase(i);
        }

        return ans;
    }
public:
    void play(NewPtr<int> &num,
              NewPtr<std::set<int>> &names,
              int role,
              int &maf_num,
              std::queue<Message> &msg,
              std::mutex &mtx,
              NewPtr<std::barrier<void (*)()>> &bar,
              bool player,
              int my_role = COMISSAR);
};


class Maniac : public Player { 
    void act(int role, bool player, std::set<int> &names, std::queue<Message> &msg, std::mutex &mtx) {}
public:
    void play(NewPtr<int> &num,
              NewPtr<std::set<int>> &names,
              int role,
              int &maf_num,
              std::queue<Message> &msg,
              std::mutex &mtx,
              NewPtr<std::barrier<void (*)()>> &bar,
              bool player,
              int my_role = MANIAC);
};


class Civillian : public Player { 
    void act(int role, bool player, std::set<int> &names, std::queue<Message> &msg, std::mutex &mtx) {}
public:
    void play(NewPtr<int> &num,
              NewPtr<std::set<int>> &names,
              int role,
              int &maf_num,
              std::queue<Message> &msg,
              std::mutex &mtx,
              NewPtr<std::barrier<void (*)()>> &bar,
              bool player,
              int my_role = CIVILLIAN);
};


bool Master::is_end(size_t num, int &maf_num) {
    return (maf_num == 0 && man_id < 0) || (maf_num + maf_num) > int(num) || (maf_num == (int(num) - maf_num) && man_id < 0) || (maf_num == 0 && num == 2 && man_id >= 0);
}

void Master::kill(NewPtr<std::set<int>> &names, int &maf_num, std::map<int, std::queue<Message>> &q, std::map<int, std::mutex> &mtx, std::ofstream &ofs) {
    for (auto &i : *names) {
        mtx[i].lock();

        q[i].push(Message{MASTER, 0, 0, maf_votes});
        mtx[i].unlock();
    }

    for (auto &i : maf_votes) {
        (*names).erase(i);

        if (players[i] == MANIAC)
            man_id = -1;
        else if (players[i] == MAFIA || players[i] == BULL || players[i] == NINJA)
            --maf_num;
        else if (players[i] == EXECUTIONER)
            exec_id = -1;
    }

    std::string got_killed;
    if (maf_votes.size() == 0)
        got_killed = "Этой ночью никто не был убит! Продолжаем! ";
    else if (maf_votes.size() == 1) 
        got_killed = "Этой ночью был убит: ";
    else 
        got_killed = "Этой ночью были убиты: ";

    for (auto &i : maf_votes)
        got_killed += std::to_string(i) + " ";
    got_killed[got_killed.length() - 1] = '\n';

    std::cout << got_killed;
    if (maf_act >= 0)
        got_killed += "Мафия убила: " + std::to_string(maf_act) + "; ";
    if (man_act >= 0)
        got_killed += "Маньяк убил: " + std::to_string(man_act) + "; ";
    if (com_act >= 0)
        got_killed += "Комиссар убил: " + std::to_string(com_act) + "; ";

    got_killed[got_killed.length() - 1] = '\n';
    ofs << got_killed;

    maf_votes.clear();
}

void Master::kick(std::set<int> &names, int &maf_num, std::map<int, std::queue<Message>> &q, std::map<int, std::mutex> &mtx, std::ofstream &ofs) {
    int kill = -1;
    int imx = *(std::ranges::max_element(day_votes | view::values));

    std::set<int> to_kick;
    std::ranges::copy_if(
        day_votes | view::filter([votes = imx](const auto& item) {
            return item.second == votes;
        }) | view::keys,
        std::inserter(to_kick, to_kick.end()),
        [](const auto& item) {
            return true; // Только ID игрока
        }
    );


    if (exec_id >= 0) {
        mtx[exec_id].lock();
        q[exec_id].push(Message{MASTER, 0, 0, to_kick});
        mtx[exec_id].unlock();

        std::this_thread::sleep_for(2 * TIME);

        while (true) {
        mtx[exec_id].lock();
            while (q[exec_id].empty()) {
                mtx[exec_id].unlock();
                std::this_thread::sleep_for(TIME);
                mtx[exec_id].lock();
            }
            if (q[exec_id].front().role != MASTER)
                break;

            mtx[exec_id].unlock();
            std::this_thread::sleep_for(TIME);
        }

        imx = q[exec_id].front().msg;
        q[exec_id].pop();
        mtx[exec_id].unlock();
    } else {
        int n = std::rand() % to_kick.size();
        auto it = std::begin(to_kick);
        std::advance(it,n);
        imx = *it;
    }

    if (imx >= 0)
        kill = players[imx];

    for (auto &i : names) {
        mtx[i].lock();
        q[i].push(Message{MASTER, imx, kill});
        mtx[i].unlock();
    }

    if (imx >= 0) {
        std::osyncstream(std::cout) << "Этим днём были кикнуты:  " << imx << std::endl;
        ofs << "Этим днём были кикнуты:  " << imx << std::endl;
        names.erase(imx);
    } else {
        std::osyncstream(std::cout) << "Никого не кикнули!\n";
        ofs << "Никого не кикнули!\n";
    }

    if (imx == -1)
        return;

    if (players[imx] == MANIAC)
        man_id = -1;
    else if (players[imx] == MAFIA || players[imx] == BULL)
        --maf_num;
    else if (players[imx] == EXECUTIONER)
        exec_id = -1;
}

void Master::vote(std::set<int> &names, std::map<int, std::queue<Message>> &q, std::map<int, std::mutex> &mtx, std::ofstream &ofs) {
    int n = names.size(), count = n;
    std::set<int> unlocked;

    while (count > 0)
        for (auto &i : names) {
            if (unlocked.contains(i))
                continue;

            mtx[i].lock();
            if (!q[i].empty()) {
                Message msg = q[i].front();
                q[i].pop();

                unlocked.insert(i);
                --count;

                ++day_votes[msg.msg];
            }
            mtx[i].unlock();
        }
}

void Master::act(std::set<int> &names, std::map<int, std::queue<Message>> &q, std::map<int, std::mutex> &mtx, std::ofstream &ofs) {
    int count = names.size();
    std::set<int> unlocked;

    bool com_check_fl = false;
    int com_check_id = 0, man_kill = -1;

    while (count > 0)
        for (auto &i : names) {
            if (unlocked.contains(i))
                continue;

            mtx[i].lock();
            if (!q[i].empty()) {
                Message msg = q[i].front();
                q[i].pop();

                --count;
                unlocked.insert(i);
                players[i] = msg.role;

                switch (msg.role) {
                    case NINJA:
                    case BULL:
                    case MAFIA:
                        maf_act = msg.msg;
                        maf_votes.insert(msg.msg);
                        break;
                    case DOCTOR:
                        doctor_act = msg.msg;
                        break;
                    case COMISSAR:
                        if (msg.kill == 1) {
                            com_act = msg.msg;
                            maf_votes.insert(msg.msg);
                        }
                        else {
                            com_check_fl = true;
                            com_check_id = msg.msg;
                        }
                        break;
                    case MANIAC:
                        man_act = msg.msg;
                        man_kill = msg.msg;
                        maf_votes.insert(msg.msg);
                        break;
                    case EXECUTIONER:
                        exec_id = i;
                        break;
                    default:
                        break;
                }
            }
            mtx[i].unlock();
        }

    if (com_check_fl) {
        com_mtx.lock();
        // std::osyncstream(std::cout) << "Comissar checked: " << com_check_id << " " << players[com_check_id] << std::endl;
        com_q.push(Message{MASTER, players[com_check_id]});
        com_mtx.unlock();
    }

    std::set<int> to_erase;
    for (auto &i : maf_votes) {
        if (i == doctor_act) {
            to_erase.insert(i);

            if (maf_act == i)
                maf_act = -1;
            else if (man_act == i)
                man_act = -1;
            else if (com_act == i)
                com_act = -1;

            // std::osyncstream(std::cout) << "This night Doctor hilled: " << i << std::endl;
            ofs << "Этой ночью доктор вылечил: " << i << std::endl;
        } else if (man_kill == i && players[i] == BULL) {
            man_act = -1;
            to_erase.insert(i);
        }
    }

    for (auto &i : to_erase)
        maf_votes.erase(i);

    doctor_act = -1;
}

void Master::play(NewPtr<int> &num,
                  NewPtr<std::set<int>> &names,
                  int role,
                  int &maf_num,
                  std::map<int, std::queue<Message>> &msg,
                  std::map<int, std::mutex> &mtx,
                  NewPtr<std::barrier<void (*)()>> &bar,
                  bool player) {
    int day = 0;

    if (player)
        std::osyncstream(std::cout) << "Ты наблюдатель, наслаждайся игрой!\n\n";
    std::osyncstream(std::cout) << "День: " << ++day << "\n-----------------------------\n";
    (*bar).arrive_and_wait();

    while (true) {
        fs::path path{"Log"};   
        path /= "log" + std::to_string(day) + ".txt";
        fs::create_directories(path.parent_path());
        std::ofstream ofs(path);
        ofs << "День: " << day << "\n-----------------------------\n";

        act(*names, msg, mtx, ofs);
        (*bar).arrive_and_wait();
        kill(names, maf_num, msg, mtx, ofs);
        (*bar).arrive_and_wait();

        day_votes.clear();
        for (auto &i : *names)
            day_votes[i] = 0;

        if (finish(num, maf_num, *names, msg, mtx, is_end((*names).size(), maf_num), ofs)) {
            std::string active = "Оставшиеся игроки' id: [";
            for (auto &i : *names)
                active += std::to_string(i) + ", ";
            active[active.length() - 2] = ']', active[active.length() - 1] = '\n';

            std::cout << active;
            ofs << active;

            (*bar).arrive_and_wait();
            (*bar).arrive_and_wait();
            break;
        }
        (*bar).arrive_and_wait();

        vote(*names, msg, mtx, ofs);
        kick(*names, maf_num, msg, mtx, ofs);
        (*bar).arrive_and_wait();

        std::string active = "Оставшиеся игроки' id: [";
        for (auto &i : *names)
            active += std::to_string(i) + ", ";
        active[active.length() - 2] = ']', active[active.length() - 1] = '\n';

        std::cout << active;
        ofs << active;

        bar.reset(new std::barrier(std::ssize(*names) + 1, on_completion));
        if (finish(num, maf_num, *names, msg, mtx, is_end((*names).size(), maf_num), ofs))  {
            ofs.close();
            break;
        }
        std::osyncstream(std::cout) << "\nДень: " << ++day << "\n-----------------------------\n";
        ofs.close();

        (*bar).arrive_and_wait();
    }

    fs::path path{"Log"};   
    path /= "log" + std::to_string(day + 1) + ".txt";
    std::filesystem::create_directories(path.parent_path());
    std::ofstream ofs(path);

    std::osyncstream(std::cout) << "\n\nИ роли игроков:\n";
    ofs << "INFO\n\nРоли игроков:\n";

    for (auto &[id, role] : players) {
        std::osyncstream(std::cout) << "Player" << id << "  --  " << role_names[role] << std::endl;
        ofs << "Player" << id << "  --  " << role_names[role] << std::endl;
    }
}

bool Master::finish(NewPtr<int> &num, int &maf_num, std::set<int> &names, std::map<int, std::queue<Message>> &q, std::map<int, std::mutex> &mtx, bool end, std::ofstream &ofs) {
    Message m{MASTER, end};
    int n = names.size();
    if ((maf_num + maf_num) > n || (maf_num  + maf_num == n && man_id < 0)) {
        m.kill = 1;
    }
    else if (maf_num == 0 && man_id < 0)
        m.kill = 0;
    else
        m.kill = 2;

    if (end) {
        switch (m.kill) {
            case 0:
                std::osyncstream(std::cout) << "Мирные выйграли!\n";
                ofs << "Мирные";
                break;
            case 1:
                std::osyncstream(std::cout) << "Мафия выйграла!\n";
                ofs << "Мафия";
                break;
            case 2:
                std::osyncstream(std::cout) << "Маньяк выйграл!\n";
                ofs << "Манияк";
        }

        ofs << " Победа!\n";
    }

    for (auto &i : names) {
        mtx[i].lock();
        q[i].push(m);
        mtx[i].unlock();
    }

    return end;
}

void Mafia::meet(int role, int &maf_num, bool player) {
    for (int i = 0; i < maf_num; i++) {
        if (i == maf_n)
            continue;

        (*mafia_mtx)[i].lock();
        (*mafia_chat)[i].push(Message{role, maf_n});
        (*mafia_mtx)[i].unlock();
    }
    std::string hi = "Your m8s are: ";
    int count = maf_num - 1;
    victims.erase(role);
    while (count > 0) {
        (*mafia_mtx)[maf_n].lock();
        while (!(*mafia_chat)[maf_n].empty()) {
            Message msg = (*mafia_chat)[maf_n].front();
            (*mafia_chat)[maf_n].pop();

            mafs[msg.role] = msg.msg;
            hi += std::to_string(msg.role) + " ";
            victims.erase(msg.role);
            --count;
        }

        (*mafia_mtx)[maf_n].unlock();
        std::this_thread::sleep_for(TIME);
    }

    hi[hi.length() - 1] = '\n';

    if (player) {
        if (maf_num > 1)
            std::cout << hi;
        else
            std::cout << "Ты единственная Мафия!\n";
    }
}

void Mafia::discuss(int role, NewPtr<int> &num, int &choice) {
    for (auto &[role, i] : mafs) {
        if (i == maf_n)
            continue;

        (*mafia_mtx)[i].lock();
        (*mafia_chat)[i].push(Message{role, choice});
        (*mafia_mtx)[i].unlock();
    }

    int count = maf_num - 1, min_vict = choice, vict_id = -1;
    std::map<int, int> dec;
    dec[choice] = 1;

    while (count > 0) {
        (*mafia_mtx)[maf_n].lock();
        while (!(*mafia_chat)[maf_n].empty()) {
            Message msg = (*mafia_chat)[maf_n].front();
            (*mafia_chat)[maf_n].pop();

            dec[msg.msg]++;

            if (min_vict > msg.msg)
                min_vict = msg.msg;

            --count;
        }

        (*mafia_mtx)[maf_n].unlock();
        std::this_thread::sleep_for(TIME);
    }

    for (auto &[id, votes] : dec) {
        if (votes > count) {
            count = votes;
            vict_id = id;
        } else if (votes == count) {
            vict_id = -1;
        }
    }

    choice = vict_id >= 0 ? vict_id : min_vict;
}

void Mafia::vote(int &kicked, int role, bool player, std::set<int> names, std::queue<Message> &msg, std::mutex &mtx, NewPtr<std::barrier<void (*)()>> &bar) {
    int choice = 0;

    if (player) {
        std::string pls = "Список игроков для голосования: [";

        for (auto &i : victims)
            pls += std::to_string(i) + ", ";
        pls[pls.length() - 2] = ']', pls[pls.length() - 1] = '\n';

        std::osyncstream(std::cout) << pls << "Выбери из списка своего кандидата для голосования: ";
        std::cin >> choice;
    } else {
        int n = std::rand() % victims.size();
        auto it = std::begin(victims);
        std::advance(it,n);
        choice = *it;
    }

    mtx.lock();
    msg.push(Message{role, choice});
    mtx.unlock();

    std::this_thread::sleep_for(TIME);
    while (true) {
        mtx.lock();
        while (msg.empty()) {
            mtx.unlock();
            std::this_thread::sleep_for(TIME);
            mtx.lock();
        }
        if (msg.front().role == MASTER)
            break;

        mtx.unlock();
        std::this_thread::sleep_for(TIME);
    }
    Message m = msg.front();
    msg.pop();
    mtx.unlock();

    kicked = m.msg;
    if (m.kill == MAFIA || m.kill == BULL || m.kill == NINJA) {
        --maf_num;
        mafs.erase(kicked);
    }
    // msg.pop();
    // mtx.unlock();

    (*bar).arrive_and_wait();
}

void Mafia::act(int role, int &maf_num, bool player, NewPtr<int> &num, std::queue<Message> &msg, std::mutex &mtx) {
    int choice = -1;

    if (player) {
        std::string pls = "Список возможных жертв: [";
        for (auto &i : victims)
            pls += std::to_string(i) + ", ";
        pls[pls.length() - 2] = ']', pls[pls.length() - 1] = '\n';

        std::osyncstream(std::cout) << pls << "Выбери жертву: ";
        std::cin >> choice;
    } else {
        int n = std::rand() % victims.size();
        auto it = std::begin(victims);
        std::advance(it,n);
        choice = *it;
    }

    discuss(role, num, choice);

    mtx.lock();
    msg.push(Message{role, choice});
    mtx.unlock();
}

void Mafia::play(NewPtr<int> &num,
                 NewPtr<std::set<int>> &names,
                 int role,
                 int &maf_num,
                 std::queue<Message> &msg,
                 std::mutex &mtx,
                 NewPtr<std::barrier<void (*)()>> &bar,
                 bool player,
                 int my_role) {
    victims = *names;
    this->maf_num = maf_num;
    meet(role, maf_num, player);
    (*bar).arrive_and_wait();

    if (player)
        Player::hello(my_role, role);

    while (true) {
        act(my_role, maf_num, player, num, msg, mtx);
        (*bar).arrive_and_wait();

        bool killed = update(role, victims, msg, mtx);
        (*bar).arrive_and_wait();

        int kicked = -1;
        if (!killed){
            if (is_end(msg, mtx)) {
                (*bar).arrive_and_wait();
                (*bar).arrive_and_wait();
                break;
            }

            (*bar).arrive_and_wait();
            vote(kicked, my_role, player, victims, msg, mtx, bar);
        } else {
            (*bar).arrive_and_wait();
            (*bar).arrive_and_wait();
        }

        if (killed) {
            if (player) std::osyncstream(std::cout) << "Тебя убили! :(\n" << std::endl;
            break;
        } else if (kicked == role) {
            if (player) std::osyncstream(std::cout) << "Ты стал всеобщим кандидатом (кикнут)! :(\n" << std::endl;
            break;
        } else 
            victims.erase(kicked);

        if (is_end(msg, mtx)) {
            break;
        }

        (*bar).arrive_and_wait();
    }
}

void Doctor::act(int role, bool player, std::set<int> &names, std::queue<Message> &msg, std::mutex &mtx) {
    int choice = chosen;

    if (player) {
        std::string pls = "Список игроков для исцеления: [";
        for (auto &i : names)
            pls += std::to_string(i) + ", ";
        pls[pls.length() - 2] = ']', pls[pls.length() - 1] = '\n';

        do {
            std::osyncstream(std::cout) << pls << "Выбери игрока для лечения: ";
            std::cin >> choice;
        } while (choice == chosen);
    } else {        
        do {
            choice = std::rand() % names.size();
        } while(chosen == choice);
    }

    chosen = choice;

    mtx.lock();
    msg.push(Message{role, choice});
    mtx.unlock();
}

void Doctor::play(NewPtr<int> &num,
                  NewPtr<std::set<int>> &names,
                  int role,
                  int &maf_num,
                  std::queue<Message> &msg,
                  std::mutex &mtx,
                  NewPtr<std::barrier<void (*)()>> &bar,
                  bool player,
                  int my_role) {
    std::set<int> hill{*names};

    (*bar).arrive_and_wait();

    if (player)
        Player::hello(my_role, role);

    while (true) {
        act(my_role, player, hill, msg, mtx);
        (*bar).arrive_and_wait();

        bool killed = Player::update(role, hill, msg, mtx);
        (*bar).arrive_and_wait();

        int kicked = -1;
        if (!killed){
            if (is_end(msg, mtx)) {
                (*bar).arrive_and_wait();
                (*bar).arrive_and_wait();
                break;
            }

            (*bar).arrive_and_wait();
            Player::vote(kicked, my_role, player, hill, msg, mtx, bar);
        } else {
            (*bar).arrive_and_wait();
            (*bar).arrive_and_wait();
        }

        if (killed) {
            if (player) std::osyncstream(std::cout) << "Ты убит! :(\n" << std::endl;
            break;
        } else if (kicked == role) {
            if (player) std::osyncstream(std::cout) << "Ты стал всеобщим кандидатом (кикнут)! :(\n" << std::endl;
            break;
        } else 
        hill.erase(kicked);

        if (is_end(msg, mtx))
            break;

        (*bar).arrive_and_wait();
    }
}

void Comissar::act(int role, bool player, std::set<int> &names, std::queue<Message> &msg, std::mutex &mtx) {
    int choice = -1, act = 0;

    if (player) {
        std::string pls = "Список игроков для проверки: [";
        for (auto &i : unchecked)
            pls += std::to_string(i) + ", ";
        pls[pls.length() - 2] = ']', pls[pls.length() - 1] = '\n';

        if (mafs.size() > 0) {
            pls += "Возможно убить следующую мафию: [";
            for (auto &i : mafs)
                pls += std::to_string(i) + ", ";
            pls[pls.length() - 2] = ']', pls[pls.length() - 1] = '\n';
        }

        std::osyncstream(std::cout) << pls << "Выбери игрока из списка и сделай действие[0 - проверка; 1 - выстрел]: ";
        std::cin >> act >> choice;
    } else if (mafs.size() > 0) {
        choice = *mafs.begin();
        act = 1;
    } else {
        int n = std::rand() % unchecked.size();
        auto it = std::begin(unchecked);
        std::advance(it,n);
        choice = *it;
    }

    Message out{role, choice, act};

    mtx.lock();
    msg.push(out);
    mtx.unlock();

    if (!act) {
        std::this_thread::sleep_for(TIME);
        while (true) {
            com_mtx.lock();
            while (com_q.empty()) {
                com_mtx.unlock();
                std::this_thread::sleep_for(TIME);
                com_mtx.lock();
            }
            if (com_q.front().role == MASTER)
                break;

            com_mtx.unlock();
            std::this_thread::sleep_for(TIME);
        }

        Message m = com_q.front();
        com_q.pop();
        com_mtx.unlock();

        unchecked.erase(choice);
        // std::osyncstream(std::cout) << "Player with id=" << choice << " role =" << m.msg << std::endl;
        if (m.msg != MAFIA && m.msg != BULL){
            if (player) std::osyncstream(std::cout) << "Игрок с id=" << choice << " мирный!\n";
        }
        else {
            if (player) std::osyncstream(std::cout) << "Игрок с id=" << choice << " мафия! Убей и голосуй за него!\n";
            mafs.insert(choice);
        }
    }
}

void Comissar::play(NewPtr<int> &num,
                  NewPtr<std::set<int>> &names,
                  int role,
                  int &maf_num,
                  std::queue<Message> &msg,
                  std::mutex &mtx,
                  NewPtr<std::barrier<void (*)()>> &bar,
                  bool player,
                  int my_role) {
    (*bar).arrive_and_wait();

    if (player)
        Player::hello(my_role, role);

    while (true) {
        if (unchecked.size() == 0) {
            unchecked = *names;
            unchecked.erase(role);
        }

        act(my_role, player, unchecked, msg, mtx);
        (*bar).arrive_and_wait();

        bool killed = update(role, unchecked, msg, mtx);
        (*bar).arrive_and_wait();

        int kicked = -1;
        if (!killed){
            if (is_end(msg, mtx)) {
                (*bar).arrive_and_wait();
                (*bar).arrive_and_wait();
                break;
            }

            (*bar).arrive_and_wait();
            Player::vote(kicked, my_role, player, unchecked, msg, mtx, bar);
        } else {
            (*bar).arrive_and_wait();
            (*bar).arrive_and_wait();
        }

        if (killed) {
            if (player) std::osyncstream(std::cout) << "Ты убит! :(\n" << std::endl;
            break;
        }
        else if (kicked == role) {
            if (player) std::osyncstream(std::cout) << "Ты стал всеобщим кандидатом (кикнут)! :(\n" << std::endl;
            break;
        }
        else {
            unchecked.erase(kicked);
            mafs.erase(kicked);
        }

        if (is_end(msg, mtx))
            break;

        (*bar).arrive_and_wait();
    }
}

void Maniac::play(NewPtr<int> &num,
                  NewPtr<std::set<int>> &names,
                  int role,
                  int &maf_num,
                  std::queue<Message> &msg,
                  std::mutex &mtx,
                  NewPtr<std::barrier<void (*)()>> &bar,
                  bool player,
                  int my_role) {
    std::set<int> pls{*names};
    pls.erase(role);

    (*bar).arrive_and_wait();

    if (player)
        Player::hello(my_role, role);
   
    while (true) {
        Player::act(my_role, player, pls, msg, mtx);
        (*bar).arrive_and_wait();

        bool killed = Player::update(role, pls, msg, mtx);
        (*bar).arrive_and_wait();

        int kicked = -1;
        if (!killed){
            if (is_end(msg, mtx)) {
                (*bar).arrive_and_wait();
                (*bar).arrive_and_wait();
                break;
            }

            (*bar).arrive_and_wait();
            Player::vote(kicked, my_role, player, pls, msg, mtx, bar);
        } else {
            (*bar).arrive_and_wait();
            (*bar).arrive_and_wait();
        }

        if (killed) {
            if (player) std::osyncstream(std::cout) << "Ты убит! :(\n" << std::endl;
            break;
        }

        else if(kicked == role) {
            if (player) std::osyncstream(std::cout) << "Ты стал всеобщим кандидатом (кикнут)! :(\n" << std::endl;
            break;
        }
        else 
            pls.erase(kicked);

        if (is_end(msg, mtx))
            break;

        (*bar).arrive_and_wait();
    }
}

void Civillian::play(NewPtr<int> &num,
                     NewPtr<std::set<int>> &names,
                     int role,
                     int &maf_num,
                     std::queue<Message> &msg,
                     std::mutex &mtx,
                     NewPtr<std::barrier<void (*)()>> &bar,
                     bool player,
                     int my_role) {
    std::set<int> pls{*names};
    pls.erase(role);

    (*bar).arrive_and_wait();

    if (player)
        Player::hello(my_role, role);

    while (true) {
        Player::act(my_role, player, pls, msg, mtx);
        (*bar).arrive_and_wait();

        bool killed = Player::update(role, pls, msg, mtx);
        (*bar).arrive_and_wait();

        int kicked = -1;
        if (!killed){
            if (is_end(msg, mtx)) {
                (*bar).arrive_and_wait();
                (*bar).arrive_and_wait();
                break;
            }

            (*bar).arrive_and_wait();

            Player::vote(kicked, my_role, player, pls, msg, mtx, bar);
        } else {
            (*bar).arrive_and_wait();
            (*bar).arrive_and_wait();
        }

        if (killed) {
            if (player) std::osyncstream(std::cout) << "Ты убит! :(\n" << std::endl;
            break;
        }
        else if (kicked == role) {
            if (player) std::osyncstream(std::cout) << "Ты стал всеобщим кандидатом (кикнут)! :(\n" << std::endl;
            break;
        } else
            pls.erase(kicked);

        if (is_end(msg, mtx))
            break;

        (*bar).arrive_and_wait();
    }
}

template <PlayerConcept T>
std::thread start_player(T* player,
                  NewPtr<int> &num,
                  NewPtr<std::set<int>> &names,
                  int role,
                  int &maf_num,
                  std::queue<Message> &msg,
                  std::mutex &mtx,
                  NewPtr<std::barrier<void (*)()>> &bar,
                  bool is_player,
                  int my_role) {
    return std::thread([player, &num, &names, role, &maf_num, &msg, &mtx, &bar, is_player, my_role]() {
        player->play(num, names, role, maf_num, msg, mtx, bar, is_player, my_role);
    });
}

template <PlayerConcept T>
void v_start_player(T* player,
                  NewPtr<int> &num,
                  NewPtr<std::set<int>> &names,
                  int role,
                  int &maf_num,
                  std::queue<Message> &msg,
                  std::mutex &mtx,
                  NewPtr<std::barrier<void (*)()>> &bar,
                  bool is_player,
                  int my_role) {
    player->play(num, names, role, maf_num, msg, mtx, bar, is_player, my_role);
}

template <MstConcept T>
std::thread start_mst(T* player,
                  NewPtr<int> &num,
                  NewPtr<std::set<int>> &names,
                  int role,
                  int &maf_num,
                  std::map<int, std::queue<Message>> &msg,
                  std::map<int, std::mutex> &mtx,
                  NewPtr<std::barrier<void (*)()>> &bar,
                  bool is_player) {
    return std::thread([player, &num, &names, role, &maf_num, &msg, &mtx, &bar, is_player]() {
        player->play(num, names, role, maf_num, msg, mtx, bar, is_player);
    });
}

template <MstConcept T>
void v_start_player(T* player,
                  NewPtr<int> &num,
                  NewPtr<std::set<int>> &names,
                  int role,
                  int &maf_num,
                  std::map<int, std::queue<Message>> &msg,
                  std::map<int, std::mutex> &mtx,
                  NewPtr<std::barrier<void (*)()>> &bar,
                  bool is_player) {
    player->play(num, names, role, maf_num, msg, mtx, bar, is_player);
}