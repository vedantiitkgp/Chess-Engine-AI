#include <bits/stdc++.h>

#define MAX_MOVES 256
#define INFINITY 1000000
#define MAX_DEPTH 5

using namespace std;

const int PEICE_VALUES[7] = {
    0, // Empty
    100, //Pawn
    200, // Knight
    300, // Bishop
    500, // Rook
    900, // Queen
    2000 // King
};

enum Peice {
    EMPTY = 0,
    W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
};

// Board State representation
typedef struct {
    int squares[64]; // Array representing value of squares
    int to_move; // 1 fo white and -1 for black
    int castling_rights; // Bitmask for Castling rights [0, 1 bit for white, 2, 3 bit for black]
    int en_passant; // En passant target square (0-63) or -1
    int halfmove_clock;
    int fullmove_number;
} Board;

// Move Representation
typdef struct {
    int from; // Start square
    int to; // End Square
    int peice; // Peice Number assigned by enu,
    int captured; // Peice Captured in This move
    int promotion; // Peice Promoted
    int is_castling;
    int is_en_passant;
    int prev_en_passant;
    int prev_castling_rights;
    int prev_halfmove_clock;
} Move;

// ------- Function Prototypes ---------- //

int generate_moves(Board *board, Move *moves);
void apply_move(Board *board, Move *move);
void undo_move(Board *board, Move *move);
int evaluate_board(Board *board);
int minimax(Board *board, int depth, int alpha, int beta, int maxmizingPlayer, clock_t end_time);
int move_heuristic(Board *board, Move *move);
void sort_moves(Board *board, Move *moves, int num_moves);
const char* choose_best_move(Board *board, double per_move_time, double total_time);

// --------------------- //

// Global variables
char notation_buffer[6]; // Buffer to hold the notation of the best move

// Generate all legal moves for current board position
int generate_moves(Board *board, Move *moves){
    int num_moves = 0;
    int direction, start, end;
    int side = board->to_move;
    int opponent = -board->to_move;

    for(int i=0; i<64; i++){
        int peice = board->squares[i];
        if(peice == EMPTY) continue;
        if((side == 1 && peice >= W_PAWN && peice <= W_KING)||(side == -1 && peice >= B_PAWN && peice <= B_KING)){
            switch(peice){
                case W_PAWN:
                case B_PAWN:
                    // Pawn Moves
                    {
                        // Move Type-1 (Forward Moves)
                        direction = (side == 1) ? 8 : -8;
                        start = i;
                        end = start + direction;
                        if(end >=0 && end<64 && board->squares[end] == EMPTY){
                            //Registering Normal Move
                            moves[num_moves] = (Move){start, end, peice, EMPTY, EMPTY, 0, 0, board->en_passant, board->castling_rights, board->halfmove_clock};
                            num_moves++;

                            // Checking double moves from starting position
                            if((side == 1 && i/8 == 1) || (side == -1 && i/8 == 6)){
                                end += direction;
                                if(board->squares[end] == EMPTY){
                                    moves[num_moves] = (Move){start, end, peice, EMPTY, EMPTY, 0, 0, board->en_passant, board->castling_rights, board->halfmove_clock};
                                    num_moves++;
                                }
                            }

                        }

                        // Move Type-2 (Captures + En Passant)
                        int capture_dirs = {-1, 1};
                        for(auto capture_dir : capture_dirs){
                            end = start + direction + capture_dir;
                            if(end>=0 && end<64 && (end%8 != 7 || capture_dir != -side) &&(end %8 == 0 || capture_dir != side)){
                                int target = board->squares[end];
                                if((target != EMPTY) && (side == 1 && target >= B_PAWN) &&(side == -1 && target >= W_PAWN && target <= W_KING)){
                                    moves[num_moves] = (Move){start, end, peice, target, EMPTY, 0, board->en_passant, board->castling_rights, board->halfmove_clock};
                                    num_moves++;
                                }

                                // En Passant
                                if(end == board->en_passant){
                                    moves[num_moves] = (Move){start, end, peice, (side==1)? B_PAWN : W_PAWN, EMPTY, 0, 1, board->en_passant, board->castling_rights, board->halfmove_clock};
                                    num_moves++;
                                }
                            }
                        }
                    }
                    break;

                case W_KNIGHT:
                case B_KNIGHT:
                    // Knight Moves
                    {
                        int knight_moves[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
                        start = i;
                        for(auto knight_move : knight_moves){
                            end = start + knight_move;
                            if(end >= 0 && end < 64 && abs((end % 8) - (start % 8)) <= 2){
                                int target = board->squares[end];
                                if((target == EMPTY) || (side == 1 && target >=B_PAWN) ||(side == -1 && target >= W_PAWN && target <= W_KING)){
                                    moves[num_moves] = (Move){start, end, peice, target, EMPTY, 0, 0, board->en_passant, board->castling_rights, board->halfmove_clock};
                                    num_moves++;
                                }
                            }
                        }
                    }
                    break;
                
                case W_BISHOP:
                case W_ROOK:
                case W_QUEEN:
                case B_BISHOP:
                case B_ROOK:
                case B_QUEEN:
                    // Sliding Peices
                    {
                        int directions[8];
                        int num_directions = 0;
                        if(peice == W_BISHOP || peice == B_BISHOP){
                            int dirs = {-9, -7, 7, 9};
                            memcpy(directions, dirs, sizeof(dirs));
                            num_directions = 4;
                        }
                        else if(peice == W_ROOK || peice == B_ROOK){
                            int dirs = {-8, -1, 1, 8};
                            memcpy(directions, dirs, sizeof(dirs));
                            num_directions = 4;
                        }
                        else if(peice == W_QUEEN || peice == B_QUEEN){
                            int dirs = {-9, -8, -1, 1, 8, 9};
                            memcpy(directions, dirs, sizeof(dirs));
                            num_directions = 8;
                        }

                        for(auto direction : directions){
                            start = i;
                            end = i;
                            while(1){
                                int rank = end / 8;
                                int file = end%8;
                                end += direction; 
                                if(end<0 || end >= 64) break;
                                int end_rank = end / 8;
                                int end_file = end % 8;

                                // Prevent Warping
                                if(abs(end_file - file) > 1 || abs(end_rank - rank) > 1) break;
                                int target = board->squares[end];

                                if((target == EMPTY) || (side == 1 && target >= B_PAWN) ||(side == -1 && target >= W_PAWN && target <= W_KING)){
                                    moves[num_moves] = (Move) {start, end, peice, target, EMPTY, 0, 0, board->en_passant, board->castling_rights, board->halfmove_clock};
                                    num_moves++;
                                    if(target != EMPTY) break;
                                }
                            }
                        }
                    }
                    break;
                case W_KING:
                case B_KING:
                    // King Moves
                    {
                        int directions[8] = {-9, -8, -7, -1, 1, 7 ,8, 9};
                        start = i;
                        for(auto direction : directions){
                            end = start + dir;
                            if(end >= 0 && end < 64 && abs((start%8) - (end % 8)) <= 1){
                                target = board->squares[end];
                                if((target == EMPTY) || (side == 1 && target >= B_PAWN) || (side == -1 && target >= W_PAWN && target <= W_KING)){
                                    moves[num_moves] = (Move){start, end, peice, target, EMPTY, 0, 0, board->en_passant, board->castling_rights, board->halfmove_clock};
                                    num_moves++;
                                }
                            }
                        }

                        // Castling(Simplified doesnt check that if squares under attack)
                        if((side == 1 && peice == W_KING) && (side == -1 && peice == B_KING)){
                            // Kingside Castling
                            if (board->castling_rights & (side == 1 ? 0x1 : 0x4)) {
                                if((board->squares[start + 1] == EMPTY) && (board->squares[start + 2] == EMPTY) && (board->squares[start+3] == ((side == 1) ? W_ROOK : B_ROOK))){
                                    moves[num_moves] = (Move){start, start + 2, peice, EMPTY, EMPTY, 1, 0, board->en_passant, board->castling_rights, board->halfmove_clock };
                                    num_moves++;
                                }
                            }

                            // Queenside Castling
                            if(board->castling_rights & (side == 1 ? 0x2 : 0x8)){
                                if((board->squares[start - 1] == EMPTY) && (board->squares[start - 2] == EMPTY) && (board->squares[start - 3] == EMPTY) && (board->squares[start-4] == ((side == 1) ? W_ROOK : B_ROOK))){
                                    moves[num_moves] = (Move){start, start - 2, peice, EMPTY, EMPTY, 1, 0, board->en_passant, board->castling_rights, board->halfmove_clock };
                                    num_moves++;
                                }
                            }
                        }
                    }
                    break;
            }
        }
    }
    return num_moves;
}

void apply_move(Board *board, Move *move){
    //Save Previous State
    move->prev_en_passant = board->en_passant;
    move->prev_castling_rights = board->castling_rights;
    move->prev_halfmove_clock = board->halfmove_clock;

    board->en_passant = -1;

    //Move the Peice
    board->squares[move->to] = move->peice;
    board->squares[move->from] = EMPTY;

    //Handle Captures
    if(move->captures != EMPTY){
        board->halfmove_clock = 0;
    }

    //Handle double pawn move (set en passant target)
    if(((move->peice == W_PAWN) || (move->peice == B_PAWN)) && abs(move->from - move->to) == 16 ){
        board->en_passant = (move->from + move-> to)/2;
    }

    //Handling en_passant pawn capture
    if(move->is_en_passant){
        int captured_pawn_pos = move_to + (board->to_move) ? 8 : - 8;
        board->squares[captured_pawn_pos] = EMPTY;
    }

    //Handling Pawn promotions
    if(move->promotion != EMPTY){
        board->squares[move->to] = move->promotion;
    }

    // Handle Castling_rights
    if (move->piece == W_KING) {
        board->castling_rights &= ~0x3; // Clear white castling rights
    } else if (move->piece == B_KING) {
        board->castling_rights &= ~0xC; // Clear black castling rights
    } else if (move->piece == W_ROOK) {
        if (move->from == 7) board->castling_rights &= ~0x1; // H1 rook moved
        else if (move->from == 0) board->castling_rights &= ~0x2; // A1 rook moved
    } else if (move->piece == B_ROOK) {
        if (move->from == 63) board->castling_rights &= ~0x4; // H8 rook moved
        else if (move->from == 56) board->castling_rights &= ~0x8; // A8 rook moved
    }

    //Handle Castling Move
    if(move->is_castling){
        if(move->to == move->from + 2){
            //Kingside castling
            board->squares[move->from + 1] = board->squares[move->from + 3];
            board->squares[move->from + 3] = EMPTY;
        }
        else if(move->to == move->from - 2){
            board->squares[move->from -1] = board->squares[move->from-4];
            board->squares[move->from-4] = EMPTY;
        }
    }

    //Update Turn
    board->to_move = - board->to_move;

    //Update Halfmove Clock
    if(move->piece == W_PAWN || move->piece == B_PAWN || move->captured != EMPTY){
        board->halfmove_clock = 0;
    }
    else{
        board->halfmove_clock++;
    }

    if(board->to_move == 1){
        board->fullmove_number++;
    }
}




int main(){

    return 0;
}

