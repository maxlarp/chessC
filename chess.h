#ifndef CHESS_H
#define CHESS_H

#include <stdbool.h>

#define WHITE 0
#define BLACK 1

typedef enum {
  W_PAWN,
  W_KNIGHT,
  W_BISHOP,
  W_ROOK,
  W_QUEEN,
  W_KING,
  B_PAWN,
  B_KNIGHT,
  B_BISHOP,
  B_ROOK,
  B_QUEEN,
  B_KING,
  NONE
} ChessPieces;

// Values stored in available_board for each target square.
typedef enum {
  ILLEGAL,
  REGULAR,
  EN_PASSANT,
  QUEENSIDE_CASTLE, // value 3: king to c-file, rook a-file -> d-file
  KINGSIDE_CASTLE,  // value 4: king to g-file, rook h-file -> f-file
  KING_MOVE
} MoveTypes;

typedef struct {
  int x, y;
} SelectedPiece;

bool get_color(char piece_raw);
bool is_in_check(char board[8][8], SelectedPiece piece);
void valid_move_generate(char board[8][8], SelectedPiece piece, char last_move[2][2], bool can_castle[2][3], char out_board[8][8]);
bool side_has_moves(char board[8][8], bool color, char last_move[2][2], bool can_castle[2][3]);
SelectedPiece find_king(char board[8][8], bool color);

#endif
