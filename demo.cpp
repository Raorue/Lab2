// yut_ui.cpp
#include <bits/stdc++.h>
using namespace std;

static const int START = -1;
static const int FIN = 29;

// ANSI color codes
const string COL_RESET = "\033[0m";
const string COL_RED   = "\033[31m";
const string COL_BLUE  = "\033[94m";
const string COL_GREEN = "\033[92m";
const string COL_PURP  = "\033[35m";

// board topology
vector<int> nextNode;            // nextNode[n] -> single-step forward
vector<vector<int>> prevNodes;   // prevNodes[n] -> predecessors

void buildBoard() {
    nextNode.assign(FIN, FIN);        // indices 0..28; FIN is 29 reserved
    prevNodes.assign(FIN, vector<int>());

    // main route 0..20
    for (int i = 0; i <= 19; ++i) nextNode[i] = i + 1;
    nextNode[20] = FIN; // move past 20 => FIN

    // shortcuts
    // 5 -> 21 -> 22 -> 23 -> 24 -> 15
    nextNode[5]  = 21;
    nextNode[21] = 22;
    nextNode[22] = 23;
    nextNode[23] = 24;
    nextNode[24] = 15;

    // 10 -> 25 -> 26 -> 27 -> 28 -> 15
    nextNode[10] = 25;
    nextNode[25] = 26;
    nextNode[26] = 27;
    nextNode[27] = 28;
    nextNode[28] = 15;

    // fill prevNodes
    for (int i = 0; i <= 28; ++i) {
        int nx = nextNode[i];
        if (nx <= 28) prevNodes[nx].push_back(i);
        // if nx == FIN we won't add
    }
}

// single-step next: START -> 0, FIN->FIN
int singleNext(int pos) {
    if (pos == START) return 0;
    if (pos == FIN) return FIN;
    if (pos >= 0 && pos <= 28) return nextNode[pos];
    return FIN;
}

// choose predecessor for back-step when history missing
int choosePred(int pos) {
    if (pos <= 0) return 0;
    if (pos > 28) return 28;
    if (!prevNodes[pos].empty()) {
        // prefer pos-1 if present
        for (int c : prevNodes[pos]) if (c == pos - 1) return c;
        // else return minimal
        return *min_element(prevNodes[pos].begin(), prevNodes[pos].end());
    }
    if (pos > 0 && pos <= 20) return pos - 1;
    return 0;
}

// trace forward/back steps and return final pos + path
pair<int, vector<int>> moveTrace(int start, int steps) {
    vector<int> path;
    int cur = start;
    if (cur == FIN) { path.push_back(FIN); return {FIN, path}; }
    if (steps == 0) return {cur, path};

    if (steps > 0) {
        for (int i = 0; i < steps; ++i) {
            cur = singleNext(cur);
            path.push_back(cur);
            if (cur == FIN) break;
        }
        return {cur, path};
    } else {
        for (int i = 0; i < -steps; ++i) {
            if (cur == START) { path.push_back(START); break; }
            int pred = choosePred(cur);
            cur = pred;
            path.push_back(cur);
        }
        return {cur, path};
    }
}

// Piece & Player
struct Piece {
    int pos;
    vector<int> history;
    Piece(): pos(START) {}
};

struct Player {
    string name;
    vector<Piece> pieces;
    string colorCode;
    Player(int m=1, const string &n="P", const string &c=""): name(n), pieces(m), colorCode(c) {}
};

// toss probabilities
string tossYut() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<> d(0.0,1.0);
    double r = d(gen);
    if (r < 0.03) return "back-do";
    if (r < 0.40) return "do";
    if (r < 0.73) return "gae";
    if (r < 0.90) return "geol";
    if (r < 0.97) return "yut";
    return "mo";
}
int yutSteps(const string &s) {
    if (s == "back-do") return -1;
    if (s == "do") return 1;
    if (s == "gae") return 2;
    if (s == "geol") return 3;
    if (s == "yut") return 4;
    if (s == "mo") return 5;
    return 0;
}

// get occupants at node: vector of (playerIndex, pieceIndex)
vector<pair<int,int>> occupantsAt(const vector<Player>& players, int node) {
    vector<pair<int,int>> out;
    for (int p = 0; p < players.size(); ++p) {
        for (int i = 0; i < players[p].pieces.size(); ++i) {
            if (players[p].pieces[i].pos == node) out.emplace_back(p,i);
        }
    }
    return out;
}

// move a bundle: moves all pieces of player pidx located at fromNode by steps
pair<int, vector<int>> moveBundle(vector<Player>& players, int pidx, int fromNode, int steps) {
    auto res = moveTrace(fromNode, steps);
    int newPos = res.first;
    for (int i = 0; i < players[pidx].pieces.size(); ++i) {
        if (players[pidx].pieces[i].pos == fromNode) {
            players[pidx].pieces[i].history.push_back(players[pidx].pieces[i].pos);
            players[pidx].pieces[i].pos = newPos;
        }
    }
    return res;
}

// bundle back one (history-aware)
int bundleBackOne(vector<Player>& players, int pidx, int fromNode) {
    int result = fromNode;
    for (int i = 0; i < players[pidx].pieces.size(); ++i) {
        if (players[pidx].pieces[i].pos == fromNode) {
            if (!players[pidx].pieces[i].history.empty()) {
                players[pidx].pieces[i].pos = players[pidx].pieces[i].history.back();
                players[pidx].pieces[i].history.pop_back();
            } else {
                players[pidx].pieces[i].pos = choosePred(players[pidx].pieces[i].pos);
            }
            result = players[pidx].pieces[i].pos;
        }
    }
    return result;
}

// capture opponents
bool captureOpponents(vector<Player>& players, int pidx, int node) {
    if (node == START || node == FIN) return false;
    bool any = false;
    for (int i = 0; i < players.size(); ++i) {
        if (i == pidx) continue;
        for (int j = 0; j < players[i].pieces.size(); ++j) {
            if (players[i].pieces[j].pos == node) {
                players[i].pieces[j].pos = START;
                players[i].pieces[j].history.clear();
                any = true;
            }
        }
    }
    return any;
}

int arrivedCount(const Player &pl) {
    int c=0;
    for (auto &pc : pl.pieces) if (pc.pos == FIN) ++c;
    return c;
}

// print UI box for a board cell (using color if present)
string cellBox(const vector<Player>& players, int node) {
    // want format like "[  ]" or "[\033[31m X\033[0m]" or multi "P0x2" etc.
    auto occ = occupantsAt(players, node);
    if (occ.empty()) return "[  ]";
    // if multiple occupants, show count per player
    // if single occupant, show player color and count 1
    // choose topmost display as first occupant's player color and index (simple)
    int p = occ[0].first;
    // count how many pieces of player p at this node
    int cnt = 0;
    for (auto &o : occ) if (o.first == p) ++cnt;
    string inside;
    if (cnt == 1) inside = to_string(p);
    else inside = to_string(p) + "x" + to_string(cnt);
    string color = players[p].colorCode;
    return "[" + color + inside + COL_RESET + "]";
}

// print the full board UI similar to example
void printUI(const vector<Player>& players, int numPlayers, int numPieces) {
    cout << "\n";
    cout << string(60, '*') << "\n";
    cout << string(60, '*') << "\n";
    cout << "**************** Let's start the Project 2 *****************\n";
    cout << string(60, '*') << "\n";
    cout << string(60, '*') << "\n";

    cout << "1. Yunnori Basic\n2. Yunnori Extension\n";
    cout << "Choose a game to play (1, 2) : 1\n";
    cout << "You will play a normal yunnori!\n";
    cout << string(60, '*') << "\n";
    cout << "*************************** Menu ***************************\n";
    cout << string(60, '*') << "\n";
    cout << "1. game start\n2. end program\n";
    cout << "Select the function you want : 1\n";
    cout << "number of players (2, 3, 4) : " << numPlayers << "\n";
    cout << "number of pieces per player (1, 2, 3, 4) : " << numPieces << "\n";

    // draw board layout using node positions approximated to the sample
    // We'll place nodes in this layout (matching earlier examples):
    // Top row: nodes 0..5 left->right? To match sample, adapt coordinates:
    // We'll use same layout as earlier but print colored boxes.

    // Row A: [0] - [1] - [2] - [3] - [4] - [5]
    cout << cellBox(players,0) << " - " << cellBox(players,1) << " - " << cellBox(players,2)
         << " - " << cellBox(players,3) << " - " << cellBox(players,4) << " - " << cellBox(players,5) << "\n";
    cout << " | " << "                    " << " | \n";
    cout << cellBox(players,6) << "    .                 .    " << cellBox(players,7) << "\n";
    cout << " |       " << cellBox(players,8) << "         " << cellBox(players,9) << "       | \n";
    cout << cellBox(players,10) << "          .     .          " << cellBox(players,11) << "\n";
    cout << " |             " << cellBox(players,12) << "             | \n";
    cout << cellBox(players,13) << "          .     .          " << cellBox(players,14) << "\n";
    cout << " |       " << cellBox(players,15) << "         " << cellBox(players,16) << "       | \n";
    cout << cellBox(players,17) << "    .                 .    " << cellBox(players,18) << "\n";
    cout << " | " << "                    " << " | \n";
    cout << cellBox(players,19) << " - " << cellBox(players,20) << " - " << cellBox(players,21)
         << " - " << cellBox(players,22) << " - " << cellBox(players,23) << " - " << cellBox(players,24)
         << " ^Start\n";

    cout << "-----------------------------------------\n";

    // Not started: pieces at START
    cout << "Not started : \n";
    for (int p = 0; p < players.size(); ++p) {
        int count = 0;
        for (int i = 0; i < players[p].pieces.size(); ++i) if (players[p].pieces[i].pos == START) ++count;
        // print colored boxes count times
        for (int k = 0; k < count; ++k) cout << players[p].colorCode << " □ " << COL_RESET;
    }
    cout << "\nArrived : ";
    // arrived pieces
    for (int p = 0; p < players.size(); ++p) {
        int count = 0;
        for (int i = 0; i < players[p].pieces.size(); ++i) if (players[p].pieces[i].pos == FIN) ++count;
        for (int k = 0; k < count; ++k) cout << players[p].colorCode << " ■ " << COL_RESET;
    }
    cout << "\n-----------------------------------------\n";
}

// helper to map display cells to actual node ids used earlier
// We used nodes 0..28. For UI above we printed certain nodes in an order — ensure mapping consistent:
// For clarity we'll use mapping matching earlier display: indexes shown were 0..24 in grid, but board logic nodes are 0..28.
// To keep consistent, we'll map UI calls to these node ids:
// Use simple mapping (this is arbitrary but consistent): 
// UI cell positions refer directly to nodes 0..24 printed; remaining nodes are accessible via movement functions.

int main() {
    // no ios sync changes to keep cout flush normal

    buildBoard();

    cout << "\n=== Yut Nori (ANSI UI) ===\n";

    int numPlayers = 0, numPieces = 0;
    while (numPlayers < 2 || numPlayers > 4) {
        cout << "Number of players (2..4): " << flush;
        if (!(cin >> numPlayers)) return 0;
    }
    while (numPieces < 1 || numPieces > 4) {
        cout << "Pieces per player (1..4): " << flush;
        if (!(cin >> numPieces)) return 0;
    }

    cout << "Auto toss? (1 = auto / 0 = manual) : " << flush;
    int autoMode = 1;
    if (!(cin >> autoMode)) return 0;

    // create players with color codes
    vector<Player> players;
    vector<string> colors = {COL_RED, COL_BLUE, COL_GREEN, COL_PURP};
    for (int i = 0; i < numPlayers; ++i) {
        Player pl(numPieces, "P"+to_string(i), colors[i]);
        players.push_back(pl);
    }

    // initialize positions at START (-1)
    for (auto &pl : players) for (auto &pc : pl.pieces) pc.pos = START;

    int turn = 0;
    while (true) {
        // render UI using a mapping to node ids
        // We'll present UI boxes but must map indices: to keep simple,
        // use node ids 0..24 for the displayed grid positions
        // (Note: nextNode mapping still uses nodes beyond 24 for shortcuts; movement unaffected)
        printUI(players, numPlayers, numPieces);

        int pidx = turn % numPlayers;
        cout << players[pidx].colorCode << "Player " << pidx << COL_RESET << " turn\n" << flush;

        // show piece positions in readable form
        cout << "Piece : ";
        for (int i = 0; i < players[pidx].pieces.size(); ++i) {
            int pos = players[pidx].pieces[i].pos;
            if (pos == START) cout << "S ";
            else if (pos == FIN) cout << "F ";
            else cout << pos << " ";
        }
        cout << "\n";

        // toss info
        string yres;
        if (autoMode) {
            yres = tossYut();
            cout << "Yut : " << yres << "\n";
        } else {
            cout << "Yut (back-do/do/gae/geol/yut/mo) : " << flush;
            cin >> yres;
            if (yres == "b") yres = "back-do";
            if (yres == "d") yres = "do";
            if (yres == "g") yres = "gae";
            if (yres == "ge") yres = "geol";
            if (yres == "y") yres = "yut";
            if (yres == "m") yres = "mo";
        }

        int steps = yutSteps(yres);
        bool extraTurn = false;

        // ask which piece to move
        cout << "Write down the position (index) of the piece to move: " << flush;
        int pick;
        if (!(cin >> pick)) return 0;
        if (pick < 0 || pick >= numPieces) {
            cout << "Invalid piece index. Turn skipped.\n";
            turn++;
            continue;
        }

        // if piece finished already, skip
        if (players[pidx].pieces[pick].pos == FIN) {
            cout << "Selected piece already arrived. Turn skipped.\n";
            turn++;
            continue;
        }

        int startPos = players[pidx].pieces[pick].pos;

        if (steps < 0) {
            // back-do: use history per piece (bundle)
            int newPos = bundleBackOne(players, pidx, startPos);
            cout << "Back-do moved bundle from " << startPos << " -> " << newPos << "\n";
            bool capped = captureOpponents(players, pidx, newPos);
            if (capped) { cout << "Captured on back-do! Extra toss.\n"; extraTurn = true; }
        } else {
            auto res = moveBundle(players, pidx, startPos, steps);
            int newPos = res.first;
            if (!res.second.empty()) {
                cout << "Path: ";
                for (int i = 0; i < res.second.size(); ++i) {
                    if (i) cout << "->";
                    if (res.second[i] == FIN) cout << "FIN"; else cout << res.second[i];
                }
                cout << "\n";
            } else {
                cout << "No move.\n";
            }
            bool capped = captureOpponents(players, pidx, newPos);
            if (capped) { cout << "Captured opponent piece(s)! Extra toss.\n"; extraTurn = true; }
            if (yres == "yut" || yres == "mo") { cout << "Yut/Mo => extra toss!\n"; extraTurn = true; }
        }

        // check win
        if (arrivedCount(players[pidx]) == numPieces) {
            cout << players[pidx].colorCode << "Player " << pidx << " WINS!" << COL_RESET << "\n";
            printUI(players, numPlayers, numPieces);
            break;
        }

        if (!extraTurn) turn++;
        else cout << "Player " << pidx << " gets an extra turn.\n";
    }

    return 0;
}
