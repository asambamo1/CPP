//Hex Board Game
 
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <vector>
#include <time.h>
#include <ctime>
#include <fstream>
#include <string>
#include <bitset>
#include <queue>
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
 
 
using namespace std;
 
static std::random_device rd;
static unsigned long seed = rd();
static std::mt19937 Mersenne(seed);
 
 
class hexboard { //create hexboard
public:
    hexboard(int dimension=11): _dimension(dimension) {
        _initial_board();
        _initial_adjacency();
    }
    int get_dimension(){return _dimension;}
   
    void set_cell(int, int, int); //X and O on board
    void display();
 
protected:
    int _dimension;
   
    vector<vector<int> > _adjacent_list; //adjacents
 
private:
    string _head; //a-k on 11x11 board
   
    vector<string> _board;
   
    void _initial_board(); //initial string diagram
    void _initial_adjacency(); //calculate initial adjacency
 
};
 
typedef bitset<128> Taken; //11*11 = 121
 
class hexgame: public hexboard {
 public:
    hexgame(int dimension): hexboard(dimension), _end(false) {_initial_borders();}
   
    bool space_taken(int, int);
    bool comp_win(const Taken&);
    bool get_end() {return _end;}
    bool winner(int); //return true if winner
   
    int  get_turn() {return _move;}
   
    void free_cells(vector<int>&);
    void player_move(); //get move from person
    void make_move(int, int);
    void set_end() {_end = true;}
    void current_position();
 
 protected:
   
    int _BestSearch(const Taken&, int); //search for a good move to make
    int _move;
   
    Taken _player_position[2]; //player positions
    Taken _border_game[4]; //bits masks for the borders
 
    void make_move(int);
    void undo_move(int);
 
 private:
    bool _end;
 
    void _initial_borders();
 };
 
 
template <typename T>
inline string _to_str (T a) { //convert char or int to a string
    string str; ostringstream temp;
    temp << a;
    return temp.str();
}
 
void hexboard::_initial_board() { // <LETTER><letter> (Example: Aa)
    string s;
    for(int i=0; i < _dimension-1; i++)
        s += (_to_str<char>('a' + i) + "   ");
    s += (_to_str<char>('a' + _dimension-1) + '\n');
    _head = s;
    for(int i=0; i < _dimension; i++) {
        string s2 = _to_str<char>('A' + i) + "  ";
        string row = ".   ";
        for(int j=0; j < _dimension-1; j++)
            s2 += row;
        s2 += (".  " + _to_str<char>('A' + i) + "\n");
        _board.push_back(s2);
    }
}
 
 
 
void hexboard::_initial_adjacency() { //precalculate adjacency
     int i = _dimension*_dimension, x1, y1, x2, y2;
     _adjacent_list.resize(i, vector<int>());
     for (int j = 0; j < i; j++)
        for (int k = j+1; k < i; k++){ //right
            x1 = j % _dimension; y1 = j/_dimension;
            x2 = k % _dimension; y2 = k/_dimension;
            if(((x1 == x2) && ((y1-y2 == 1) || (y2-y1 == 1)))||((y1 == y2) && ((x1-x2 == 1) || (x2-x1 == 1))) ||((x2-x1 == 1) && (y2-y1 == -1)) || ((x2-x1 == -1) && (y2-y1 == 1))){
                _adjacent_list[j].push_back(k); //symmetry
                _adjacent_list[k].push_back(j); //symmetry
           
        }
    }
}
 
 
 
 void hexboard::set_cell(int row, int column, int X_O) { //X or O
      _board[row][3+4*column] = (X_O == 0? 'x': (X_O == 1? 'o':' '));
 
 }
 
 
 void hexboard::display() { //display board
    string s(2*_dimension-1, ' ');
    cout << s + "o\n";
    cout << ("  " + _head);
    string left = " ";
    for (int i = 0; i < _dimension; i++){
        if(i == _dimension/2)
            cout << (left.substr(0, left.size()-4) + "x   " + _board[i].substr(0, _board[i].size()-1) + "     x\n");
        else
            cout << (left + _board[i]);
        left += " ";
    }
    cout << (left + "  " + _head);
    cout << (left + s + "  o\n");
 }
 
 void hexgame::_initial_borders() { //initialize border bitmaps
    for(int i=0; i < _dimension; ++i) {
        _border_game[0].set(i*_dimension); //left
        _border_game[2].set((i+1)*_dimension-1); //right
        _border_game[1].set(i); //top
        _border_game[3].set((_dimension-1)*_dimension+i); //bottom
    }
    _move = 0; //x moves first
}
 
 const int MC_SIM = 1000; //Monte Carlo simulation number
 
 bool hexgame::space_taken(int row, int column){
    int bit = _dimension*row + column; //test if cell occupied
    return _player_position[0].test(bit) || _player_position[1].test(bit);
 }
 
 bool hexgame::comp_win(const Taken& position) {
    int scale = _move? _dimension : 1; //direction to move
    for(int j=0; j < _dimension; ++j) {
        int last = scale*j;
        if(position.test(last) && (_BestSearch(position, last) == _move)) return true;
    }
    return false;
}
 
 
 bool hexgame::winner(int last) {
    return _BestSearch(_player_position[_move], last) == _move;
 }
 
 int hexgame:: _BestSearch(const Taken& position, int cell) {
 Taken connectedcomp; //find connected components of node in graph
 queue<int> Q; //queue implementation to check when player wins
 Q.push(cell);
 connectedcomp.set(cell);
 while (! Q.empty()){ //when empty
    int i = Q.front();
    Q.pop();
    for (int j=0, n = _adjacent_list[i].size(); j<n; ++j){
        int k = _adjacent_list[i][j];
        if(((position.test(k)) && !connectedcomp.test(k))){
        connectedcomp.set(k);
        Q.push(k);
        }
    }
 }
 if(((_border_game[0] & connectedcomp).any()) && ((_border_game[2] & connectedcomp).any())) return 0;
 if(((_border_game[1] & connectedcomp).any()) && ((_border_game[3] & connectedcomp).any())) return 1;
 return -1; //no winner
 
 }
 
 
 void hexgame::make_move(int row, int column) {
    int cell = _dimension*row + column;
    _player_position[_move].set(cell); //set appropriate bit of occupied field
    set_cell(row, column, _move);
    if(winner(cell)) set_end(); // if player (person/PC) wins
    else _move = 1 - _move; //other player's move
 }
 
 void hexgame::make_move(int cell) { //set bit for occupied field
    _player_position[_move].set(cell);
    if(winner(cell))
        set_end();
 }
 
 void hexgame::free_cells(vector<int>& moves) {
    Taken occupied = _player_position[0] | _player_position[1];
    for(int i=0, n = _dimension*_dimension; i < n; ++i)
        if(! occupied.test(i))
            moves.push_back(i);
 }
 
 void hexgame::player_move() { //player making move
    string move;
    char row, column;
    do {
        cout << "Enter your move (Ex: Ak or Bd): "; //Ex: Ak
        cin >> move;
        row = move.at(0), column = move.at(1);
    } while((row < 'A') || (row > 'A'+_dimension-1) || (column < 'a') || (column > 'a'+_dimension-1) || space_taken(row-'A', column-'a'));
    make_move(row-'A', column-'a');
 }
 
 void hexgame::undo_move(int cell) {
    _player_position[_move].reset(cell);
 }
 
 void hexgame::current_position() { //finding curr position of move
    vector<int> moves;
    free_cells(moves); //possible legal moves
    int best = 0; //best response to current position
    int res  = moves[0];
    int rest = moves.size() - 1; //remaining moves
    Taken T;
    for(int current=0; current <= rest; ++current) {
        int move = moves[current];
        make_move(move);
        if(get_end()) {
            undo_move(move);
            res  = move;
            break;
        }
        vector<int> opponent;
        free_cells(opponent); //get all moves
        int n_wins = 0; //count number of winning positions by computer
        for(int t=0; t < MC_SIM; ++t) {
            shuffle(opponent.begin(), opponent.end(), Mersenne);
            T = _player_position[_move];
            for(int i=0; i < rest; i+=2) //Make computer make half of the moves available
                T.set(opponent[i]);
            if(comp_win(T))
                n_wins++;
        }
        if((n_wins > best) && (n_wins != MC_SIM)) { //score
            best = n_wins; //update
            res = move;
        }
        undo_move(move);
    }
 
    if(best==0) cout << "Player wins!\n";
    make_move(res / _dimension, res % _dimension); //response to be resulting move
}
 
int main(int argc, char** argv) {
    //calls the functions
    int dimension = 11; //11x11 creation.
    hexgame h(dimension);
    h.display(); //display
	
    cout << "Player 'X' moves first to connect Left and then Right\n";
    cout << "Player 'O' follows to move and tries to connect Top to Bottom\n";
    cout << "Two cells are connected only if they neighbor horizontally or diagonally\n";
	
    int you = 0;
    cout << "By default you are first to move ('X');\nIf you want to change this press 'C': ";
    char res; cin.get();
    if((res=cin.get()) == 'C') you = 1;
    cout << '\n';
    int to_move;
	
    do {
           to_move = h.get_turn();
           if(to_move == you) {
               h.display();
               h.player_move();
           } else {
               cout << "Making move...\n";
               h.current_position();
           }
    } while(h.get_end() == false);
    h.display();
    char xo = to_move? 'o':'x';
    cout << (to_move == you? "\nYou (":"\nComputer (") << xo << ") win!\n\n";
}
