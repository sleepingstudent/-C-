#include "gAmE.hpp"


using std::string;
using std::cout, std::cin, std::endl;


int main() {
    int n, k;
    bool inp;
    do {
        cout << "Количество игроков:\n>";
        cin >> n;
    } while (n <= 4); //по условиям болSьше 4

    do {
        cout << "Параметр'k' для определения соотношения мафии, не менее 3:\n>";
        cin >> k;
    } while (k > n && k < 3);

    string in;
    do {
        cout << "Будешь ли ты играть? ('y'/'n'):\n>";
        cin >> in;
    } while (in != "y" && in != "n");

    inp = in == "y";

    Game game(n, k, inp);
    game.play();

    return 0;
}