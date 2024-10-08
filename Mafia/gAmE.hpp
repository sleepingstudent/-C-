#pragma once 

#include <ctime>
#include <cstdlib>

#include "player.hpp"


template<typename T>
std::mutex& get_mutex(T &m, const int &id) {
    return m[id];
}


class Game {
    int n;
    int maf_n;

    bool master;

    std::vector<Player *> players;
    std::vector<int> players_ids;
    std::vector<int> players_roles;
    std::set<int> *names;

    std::map<int, std::queue<Message>> messages;
    std::map<int, std::mutex> mutexes;
public:
    Game(int n, int k, bool master);
    void play();
};

Game::Game(int n, int k, bool master) {
    this->n = n;
    this->maf_n = n / k;
    this->master = master;

    if (this->n >= 10000)
        TIME = 25ms;
    else if (this->n >= 1000)
        TIME = 15ms;
    else if (this->n >= 100)
        TIME = 5ms;

    names = new std::set<int>();

    players.push_back(static_cast<Player *>(new Doctor()));
    players.push_back(static_cast<Player *>(new Comissar()));
    players.push_back(static_cast<Player *>(new Maniac()));

    players_roles.push_back(DOCTOR);
    players_roles.push_back(COMISSAR);
    players_roles.push_back(MANIAC);

    NewPtr<std::map<int, std::queue<Message>>> mafia_chat(new std::map<int, std::queue<Message>>());
    NewPtr<std::map<int, std::mutex>> mafia_mtx(new std::map<int, std::mutex>);
    for (auto i : view::iota(0, std::min(maf_n, n - 3))) {
        (*mafia_chat)[i] = std::queue<Message>();
        players.push_back(static_cast<Player *>(new Mafia(std::ref(i), std::ref(mafia_chat), std::ref(mafia_mtx))));
        players_roles.push_back(MAFIA);
    }

    std::string choice = "n";
    int extra = this->n - maf_n - 3;
    if (extra > 0) {
        do {
            std::cout << "Игра с дополнительными ролями? 'y'/'n'\n>";
            std::cin >> choice;
        } while (choice != "y" && choice != "n");
    }
    if (choice == "y") {
        int ch = extra > 2 ? 3 : (extra == 2 ? 2 : 1);
        switch (ch) {
            case 3:
                (*mafia_chat)[this->maf_n] = std::queue<Message>();
                players.push_back(static_cast<Player *>(new Mafia(this->maf_n, std::ref(mafia_chat), std::ref(mafia_mtx))));
                players_roles.push_back(NINJA);
                this->maf_n++;
            case 2:
                (*mafia_chat)[this->maf_n] = std::queue<Message>();
                players.push_back(static_cast<Player *>(new Bull(this->maf_n, std::ref(mafia_chat), std::ref(mafia_mtx))));
                players_roles.push_back(BULL);
                this->maf_n++;
            case 1:
                players.push_back(static_cast<Player *>(new Civillian()));
                players_roles.push_back(EXECUTIONER);
        }
        extra -= ch;
    }

    for ([[maybe_unused]]auto _ : view::iota(0, extra)) {
        players.push_back(static_cast<Player *>(new Civillian()));
        players_roles.push_back(CIVILLIAN);
    }

    std::ranges::copy(std::views::iota(0, n), std::back_inserter(players_ids));

    std::random_device rd;
    std::mt19937 gen {rd()};

    std::ranges::shuffle(players_ids, gen);
}

void Game::play() {
    std::vector<std::thread> threads;

    int role = -1;
    if (this->master)
        role = std::rand() % this->n;

    for (auto i : view::iota(0, this->n))
        names->insert(i);

    NewPtr<int> players_num(new int(n));
    NewPtr<std::set<int>> players_names(names);

    NewPtr<std::barrier<void (*)()>> sync_point(new std::barrier(std::ssize(*names) + 1, on_completion));
    

    int i = 0;
    for (auto &player : players) {
        if (i != role) {
            int id = players_ids[i];
            messages[id] = std::queue<Message>();
            get_mutex<std::map<int, std::mutex>>(mutexes, id);
            threads.push_back(start_player(player,
                                          std::ref(players_num),
                                          std::ref(players_names),
                                          id,
                                          std::ref(maf_n),
                                          std::ref(messages[id]),
                                          std::ref(mutexes[id]),
                                          std::ref(sync_point),
                                          false,
                                          players_roles[i]));
        }
        ++i;
    }

    Master *mst = new Master();

    if (this->master) {
        threads.push_back(start_mst(mst,
                                  std::ref(players_num),
                                  std::ref(players_names),
                                  this->n,
                                  std::ref(maf_n),
                                  std::ref(messages),
                                  std::ref(mutexes),
                                  std::ref(sync_point),
                                  false));
        int id = players_ids[role];
        messages[id] = std::queue<Message>();
        get_mutex<std::map<int, std::mutex>>(mutexes, id);
        v_start_player(players[role],
                        std::ref(players_num),
                        std::ref(players_names),
                        id,
                        std::ref(maf_n),
                        std::ref(messages[id]),
                        std::ref(mutexes[id]),
                        std::ref(sync_point),
                        true,
                        players_roles[role]);
    } else {
        Master().play(std::ref(players_num),
                      std::ref(players_names),
                      std::ref(n),
                      std::ref(maf_n),
                      std::ref(messages),
                      std::ref(mutexes),
                      std::ref(sync_point),
                      true);
    }


    for (std::thread &thread : threads)
        thread.join();

    for (auto &player : players)
        delete player;

    delete mst;
}