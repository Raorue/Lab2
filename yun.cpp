#include <vector>
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <cstdlib>
#include <ctime> 
#include <algorithm>

using namespace std;

const string RESET  = "\033[0m";
const string RED    = "\033[31m";
const string BLUE   = "\033[34m";
const string GREEN  = "\033[32m";
const string YELLOW = "\033[33m";

//기초 설정들 완

struct BoardCell {
    int id;             
    int owner;          
    int pieceCount;     

    int next;           
    int altNext;        
    int prev;
};

vector<BoardCell> board(30);

void initBoard() {
    for (int i = 0; i < 30; i++) {
        board[i].id = i;
        board[i].owner = 0;
        board[i].pieceCount = 0;

        board[i].next = -1;      
        board[i].altNext = -1; 
        board[i].prev = -1;
    }

    // 바깥 테두리
    board[0].next = 1;

    board[1].next = 2;
    board[2].next = 3;
    board[3].next = 4;
    board[4].next = 5;
    board[5].next = 6;

    board[6].next = 7;
    board[7].next = 8;
    board[8].next = 9;
    board[9].next = 10;
    board[10].next  = 11;
    
    board[11].next = 12;
    board[12].next = 13;
    board[13].next = 14;
    board[14].next = 15;
    board[15].next = 16;
    
    board[16].next = 17;
    board[17].next = 18;
    board[18].next = 19;
    board[19].next = 29;

    //모서리 처리용
    board[5].altNext = 20;
    board[10].altNext = 25;
    board[22].altNext = 27;

    //내부 처리용
    board[20].next = 21;
    board[21].next = 22;
    
    board[23].next = 24;
    board[24].next = 15;

    board[25].next = 26;
    board[26].next = 22;

    board[27].next = 28;
    board[28].next = 29;

    board[22].next = 27;

    board[29].next = -1; 
    
    //백도 처리용
    board[1].prev = 29;
    board[2].prev = 1;
    board[3].prev = 2;
    board[4].prev = 3;
    board[5].prev = 4;

    board[6].prev = 5;
    board[7].prev = 6;
    board[8].prev = 7;
    board[9].prev = 8;
    board[10].prev = 9;

    board[11].prev = 10;
    board[12].prev = 11;
    board[13].prev = 12;
    board[14].prev = 13;
    board[15].prev = 14;

    board[16].prev = 15;
    board[17].prev = 16;
    board[18].prev = 17;
    board[19].prev = 18;

    board[20].prev = 5;
    board[21].prev = 20;
    board[22].prev = 21;
    board[23].prev = 22;
    board[24].prev = 23;
    board[25].prev = 10;
    board[26].prev = 25;
    board[27].prev = 22;
    board[28].prev = 27;

    board[29].prev = 19;
}

//보드 설정 완

struct Piece {
    int position;   
    int groupId; 
};

struct Player { 
    int id;                    
    vector<Piece> pieces;       
};

vector<Player> players;

void initPlayers (int playerCount, int pieceCount) {
    players.clear();                 
    players.reserve(playerCount);    

    for (int i = 1; i <= playerCount; i++) {
        Player p;
        p.id = i;
        for (int j = 0; j < pieceCount; j++) {
            Piece piece;
            piece.position = 0;   
            piece.groupId = j;    
            p.pieces.push_back(piece);
        }
        players.push_back(p);
    }
}

enum YutResult {
    DO,     // 1칸
    GAE,    // 2칸
    GEOL,   // 3칸
    YUT,    // 4칸
    MO,     // 5칸
    BACKDO  // -1칸
};

YutResult throwYut() {
    int r = rand() % 16;   // 0 ~ 15

    if (r < 3) return DO;             
    else if (r < 9) return GAE;       
    else if (r < 13) return GEOL;     
    else if (r < 14) return YUT;      
    else if (r < 15) return MO;       
    else return BACKDO;            

}

int yutToMove (YutResult r) {
    switch (r) {
        case DO:     return 1;
        case GAE:    return 2;
        case GEOL:   return 3;
        case YUT:    return 4;
        case MO:     return 5;
        case BACKDO: return -1;
    }
    return 0;
}

struct MoveResult {
    bool captured = false;
    bool stacked = false;
    bool finished = false;
};


MoveResult movePiece(Piece& piece, int move, int playerId) {
    MoveResult result;

    int oldPos = piece.position;
    int cur;

    if (piece.position == 30) {
    return result; 
    }

    if (piece.position == -1) {
        if (move < 0) return result;   
        cur = 0;                      
    }

    else {
        cur = piece.position;
    }

    bool firstStep = true;

    while (move != 0) {
        // (백도)
        if (move < 0) {
            cur = board[cur].prev;
            move++;
            continue;
        }

        // 앞으로 이동
        if (cur == 29) {
            piece.position = 30;
            result.finished = true;
            return result;
        }

        if (firstStep && board[cur].altNext != -1) {
            cur = board[cur].altNext;
        } 
        else {
            cur = board[cur].next;
        }
        firstStep = false;
        move--;
    }

    // 이전 칸 board 정리
    if (oldPos >= 0 && oldPos <= 29) {
        board[oldPos].pieceCount--;
        if (board[oldPos].pieceCount == 0) {
            board[oldPos].owner = 0;
        }
    }

    // 도착 처리
    // 완주
    if (cur == -1) {
        piece.position = 30;
        result.finished = true;
        return result;
    }

    // 잡기
    if (board[cur].owner != 0 && board[cur].owner != playerId) {
        // 상대 말 전부 제거 
        int opponent = board[cur].owner;

        for (auto& pl : players) {
            if (pl.id == opponent) {
                for (auto& p : pl.pieces) {
                    if (p.position == cur) {
                        p.position = -1;
                    }
                }
            }
        }

        board[cur].pieceCount = 0;
        board[cur].owner = 0;

        result.captured = true;
    }

    // 합치기
    if (board[cur].owner == playerId) {
        result.stacked = true;
    }

    // 5️board & piece 최종 갱신
    piece.position = cur;
    board[cur].owner = playerId;
    board[cur].pieceCount++;

    return result;
}

int selectPiece(const Player& player) {
    int idx;

    while (true) {
        cout << "Choose piece index (0 ~ "
             << player.pieces.size() - 1 << "): ";
        cin >> idx;

        if (idx >= 0 && idx < player.pieces.size()) {
            return idx;
        }

        if (player.pieces[idx].position == 30) {
            cout << "This piece already finished. Choose another.\n";
            continue;
        }

        cout << "Invalid piece.\n";
    }
}

string ownerColor(int owner) {
    switch (owner) {
        case 1: return RED;
        case 2: return BLUE;
        case 3: return GREEN;
        case 4: return YELLOW;
        default: return RESET;
    }
}

string cellStr(int idx) {
    if (idx < 0) return "   ";

    string color = ownerColor(board[idx].owner);

    // 빈 칸
    if (board[idx].pieceCount == 0) {
        return "[ ]";
    }

    return "[" + color + to_string(board[idx].pieceCount) + RESET + "]";
}


void printBoardUI() {

    cout << cellStr(10) << " - " << cellStr(9) << " - " << cellStr(8)
         << " - " << cellStr(7) << " - " << cellStr(6) << " - " << cellStr(5) << endl;

    cout << " | " << cellStr(25) << "                     " << cellStr(20) << " | " << endl;

    cout << cellStr(11) << "    .                 .    " << cellStr(4) << endl;

    cout << " |       " << cellStr(26) << "         " << cellStr(21) << "       | " << endl;

    cout << cellStr(12) << "          .     .          " << cellStr(3) << endl;

    cout << " |             " << cellStr(22) << "             | " << endl;

    cout << cellStr(13) << "          .     .          " << cellStr(2) << endl;

    cout << " |       " << cellStr(23) << "         " << cellStr(27) << "       | " << endl;

    cout << cellStr(14) << "    .                 .    " << cellStr(1) << endl;

    cout << " | " << cellStr(24) << "                     " << cellStr(28) << " | " << endl;

    cout << cellStr(15) << " - " << cellStr(16) << " - " << cellStr(17)
         << " - " << cellStr(18) << " - " << cellStr(19) << " - " << cellStr(29) << "^Start" << endl;
    

    cout << "-----------------------------------------\n";
}

int yutOrder(YutResult y) {
    switch (y) {
        case BACKDO: return 0;
        case DO:     return 1;
        case GAE:    return 2;
        case GEOL:   return 3;
        case YUT:    return 4;
        case MO:     return 5;
    }
    return 0;
}

vector<YutResult> throwAndStoreYut() {
    vector<YutResult> hand;

    while (true) {
        YutResult y = throwYut();
        hand.push_back(y);

        if (y != YUT && y != MO)
            break;  // 윷/모 아니면 종료
    }

    return hand;
}

void printYutHand(vector<YutResult>& hand) {
    sort(hand.begin(), hand.end(),[](YutResult a, YutResult b) {
             return yutOrder(a) < yutOrder(b);
         });

    cout << "Available Yut Results:\n";
    for (int i = 0; i < hand.size(); i++) {
        cout << "  [" << i << "] move " << yutToMove(hand[i]) << "\n";
    }
}

void printPieceStatus(const Player& p) {
    cout << "Not started : \n";
    for (const auto& pc : p.pieces) {
        if (pc.position == -1) {
            cout << RED << " □ " << RESET;
        }
    }
    cout << "\nArrived : \n";
    for (const auto& pc : p.pieces) {
        if (pc.position == 30) {
            cout << RED << " □ " << RESET;
        }
    }
    cout << "\n";
}

string yutToString(YutResult y) {
    switch (y) {
        case DO: return "do";
        case GAE: return "gae";
        case GEOL: return "geol";
        case YUT: return "yut";
        case MO: return "mo";
        case BACKDO: return "back-do";
    }
    return "";
}

bool stringToYut(const string& s, YutResult& out) {
    if (s == "do") out = DO;
    else if (s == "gae") out = GAE;
    else if (s == "geol") out = GEOL;
    else if (s == "yut") out = YUT;
    else if (s == "mo") out = MO;
    else if (s == "back-do") out = BACKDO;
    else return false;
    return true;
}


void printTurnUI(const Player& p, const vector<YutResult>& hand) {
    cout << "-----------------------------------------\n";
    cout << "Player " << RED << p.id - 1<< RESET << " turn\n";

    cout << "Piece : ";
    for (const auto& pc : p.pieces) {
        if (pc.position == -1) cout << "0 ";
        else if (pc.position == 30) cout << "";
        else cout << pc.position << " ";
    }
    cout << "\n";

    cout << "Yut : ";
    for (const auto& y : hand) {
        cout << yutToString(y) << " ";
    }
    cout << "\n";

    cout << "Write down the position of the player to move and yut\n";
    cout << "(back-do, do, gae, geol, yut, and mo)\n";
}

void gameLoop(int playerCount) {
    int currentPlayer = 0;
    bool gameOver = false;

    while (!gameOver) {
        Player& p = players[currentPlayer];
        bool extraTurn = true;

        while (extraTurn) {
            extraTurn = false;

            vector<YutResult> yutHand = throwAndStoreYut();

            while (!yutHand.empty()) {
                printBoardUI();
                printPieceStatus(p);
                printTurnUI(p, yutHand);
                
                cout << ">> position : ";
                int pieceIdx;
                cin >> pieceIdx;

                cout << ">> yut : ";
                string yutStr;
                cin >> yutStr;

                
                YutResult chosen;
                if (!stringToYut(yutStr, chosen)) {
                    cout << "Invalid yut input.\n";
                    continue;
                }

                // hand 안에 해당 윷이 있는지 탐색
                auto it = find(yutHand.begin(), yutHand.end(), chosen);
                if (it == yutHand.end()) {
                    cout << "You don't have that yut result.\n";
                    continue;
                }

                int move = yutToMove(chosen); 

                Piece& piece = p.pieces[pieceIdx];

                MoveResult r = movePiece(piece, move, p.id);

                yutHand.erase(it);

                if (r.captured) {
                    cout << "Captured opponent!\n";
                    extraTurn = true;
                }
                if (r.stacked)  cout << "Stacked with own piece!\n";
                if (r.finished) cout << "A piece has finished!\n";

                bool allFinished = true;
                for (auto& pc : p.pieces) {
                    if (pc.position != 30) {
                        allFinished = false;
                        break;
                    }
                }

                if (allFinished) {
                    cout << "\n🎉 Player " << p.id << " wins!\n";
                    gameOver = true;
                    return;
                }
            }
        }

        // 다음 차례
        currentPlayer = (currentPlayer + 1) % playerCount;
    }
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    int playerCount;
    int pieceCount;

    cout << "number of players (2, 3, 4) : ";
    cin >> playerCount;

    cout << "number of peices per player (1, 2, 3, 4) : ";
    cin >> pieceCount;

    // 초기화
    initBoard();
    initPlayers(playerCount, pieceCount);

    // 게임 시작
    gameLoop(playerCount);

    return 0;
}
