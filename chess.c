#include "chess.h"
#include <string.h>

bool get_color(char piece_raw) {
  return piece_raw > W_KING;
}

void cast_rays(char board[8][8], char deltas[4][2], SelectedPiece piece, bool piece_color, char out_board[8][8]) {
  for (int i = 0; i < 4; i++) {
    int j = 1;
    while (true) {
      int new_x = deltas[i][0] * j + piece.x;
      int new_y = deltas[i][1] * j + piece.y;

      if (new_x < 0 || new_x > 7) break;
      if (new_y < 0 || new_y > 7) break;

      if (board[new_x][new_y] != NONE) {
        if (get_color(board[new_x][new_y]) == piece_color) {
          break;
        }
        else {
          out_board[new_x][new_y] = REGULAR;
          break;
        }
      }
      out_board[new_x][new_y] = REGULAR;

      j++;
    }
  }
}

bool is_in_check(char board[8][8], SelectedPiece piece) {
  char piece_raw = board[piece.x][piece.y];
  bool piece_color = get_color(piece_raw);

  char mod = (piece_color) ? -1 : 1;
  char opposite_piece_color_mod = (piece_color) ? 0 : 6;
  int pawn_x1 = piece.x + 1;
  int pawn_x2 = piece.x - 1;
  int pawn_y = piece.y + mod;
  if (pawn_y >= 0 && pawn_y <= 7) {
    if (pawn_x1 <= 7 && board[pawn_x1][pawn_y] == W_PAWN + opposite_piece_color_mod) return true;
    if (pawn_x2 >= 0 && board[pawn_x2][pawn_y] == W_PAWN + opposite_piece_color_mod) return true;
  }

  char deltas[8][2] = {
    {-1, 2}, {1, 2},
    {2, -1}, {2, 1},
    {-1, -2}, {1, -2},
    {-2, -1}, {-2, 1}
  };
  for (int i = 0; i < 8; i++) {
    int new_x = piece.x + deltas[i][0];
    int new_y = piece.y + deltas[i][1];

    if (new_x < 0 || new_x > 7) continue;
    if (new_y < 0 || new_y > 7) continue;

    if (board[new_x][new_y] == W_KNIGHT + opposite_piece_color_mod) {
      return true;
    }
  }

  char queen_deltas[8][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {0, 1}, {0, -1}, {-1, 0}, {1, 0}};

  for (int i = 0; i < 8; i++) {
    char j = 1;
    while (true) {
      int new_x = queen_deltas[i][0] * j + piece.x;
      int new_y = queen_deltas[i][1] * j + piece.y;
      
      if (new_x < 0 || new_x > 7) break;
      if (new_y < 0 || new_y > 7) break;
      if (board[new_x][new_y] != NONE) {
        if (board[new_x][new_y] == W_QUEEN + opposite_piece_color_mod) {
          return true;
        }
        if (i < 4) {
          if (board[new_x][new_y] == W_BISHOP + opposite_piece_color_mod) {
            return true;
          }
        } else {
          if (board[new_x][new_y] == W_ROOK + opposite_piece_color_mod) {
              return true;
          }
        }
        break;
      }

      j++;
    }
  }
  for (int i = -1; i < 2; i++) {
    for (int j = -1; j < 2; j++) {
      int new_x = piece.x + i;
      int new_y = piece.y + j;
      if (new_x < 0 || new_x > 7) continue;
      if (new_y < 0 || new_y > 7) continue;
      if (board[new_x][new_y] == W_KING + opposite_piece_color_mod) {
        return true;
      }
    }
  }

  return false;
}

void pawn_moves_generate(char board[8][8], bool piece_color, SelectedPiece piece, char last_move[2][2], char out_board[8][8]) {
  char mod = (piece_color) ? -1 : 1;
  int forward_y = piece.y + mod;
  if (forward_y < 0 || forward_y > 7) return; // no promotion support: pawn stuck on last rank

  for (int i = -1; i < 3; i += 2) {
    int capture_x = piece.x + i;
    if (capture_x < 0 || capture_x > 7) continue;
    if (board[capture_x][forward_y] != NONE && get_color(board[capture_x][forward_y]) != piece_color) {
      out_board[capture_x][forward_y] = REGULAR;
    }
  }
  if (board[piece.x][forward_y] == NONE) {
    out_board[piece.x][forward_y] = REGULAR;
    if ((piece_color == BLACK && piece.y == 6) || (piece_color == WHITE && piece.y == 1)) {
      if (board[piece.x][piece.y + mod * 2] == NONE) {
        out_board[piece.x][piece.y + mod * 2] = REGULAR;
      }
    }
  }
  for (int i = -1; i < 3; i += 2) {
    int capture_x = piece.x + i;
    if (capture_x < 0 || capture_x > 7) continue;
    if (board[capture_x][piece.y] == B_PAWN - piece_color * 6) {
      if ((last_move[1][0] == capture_x && last_move[1][1] == piece.y) && (last_move[0][1] == 1 || last_move[0][1] == 6)) {
        // The en passant target square must be empty, otherwise the capture
        // would silently swallow whatever piece stands there.
        if (board[capture_x][forward_y] == NONE) {
          out_board[capture_x][forward_y] = EN_PASSANT;
        }
      }
    }
  }
}

void knight_moves_generate(char board[8][8], SelectedPiece piece, bool piece_color, char out_board[8][8]) {
  char deltas[8][2] = {
    {-1, 2}, {1, 2},
    {2, -1}, {2, 1},
    {-1, -2}, {1, -2},
    {-2, -1}, {-2, 1}
  };
  for (int i = 0; i < 8; i++) {
    int new_x = piece.x + deltas[i][0];
    int new_y = piece.y + deltas[i][1];

    if (new_x < 0 || new_x > 7) continue;
    if (new_y < 0 || new_y > 7) continue;

    out_board[new_x][new_y] = (get_color(board[new_x][new_y]) != piece_color || board[new_x][new_y] == NONE);
  }
}

void ray_moves_generate(char board[8][8], SelectedPiece piece, char piece_type, bool piece_color, char out_board[8][8]) {
  char deltas[4][2];
  if (piece_type == W_BISHOP) {
    char bishop_deltas[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    memcpy(deltas, bishop_deltas, sizeof(deltas));
  } else {
    char rook_deltas[4][2] = {{0, 1}, {0, -1}, {-1, 0}, {1, 0}};
    memcpy(deltas, rook_deltas, sizeof(deltas));
  }

  cast_rays(board, deltas, piece, piece_color, out_board);
}

// Reports whether the king, temporarily placed on (to_x, to_y), would be
// attacked. Used to reject castling through or into check.
bool king_path_attacked(char board[8][8], SelectedPiece from, int to_x, int to_y) {
  char king = board[from.x][from.y];
  char vacated = board[to_x][to_y];

  board[from.x][from.y] = NONE;
  board[to_x][to_y] = king;
  bool attacked = is_in_check(board, (SelectedPiece){to_x, to_y});
  board[to_x][to_y] = vacated;
  board[from.x][from.y] = king;

  return attacked;
}

void king_moves_generate(char board[8][8], SelectedPiece piece, bool piece_color, bool can_castle[2][3], char out_board[8][8]) {
  for (int i = -1; i < 2; i++) {
    for (int j = -1; j < 2; j++) {
      int new_x = i + piece.x;
      int new_y = j + piece.y;

      if (new_x < 0 || new_x > 7) continue;
      if (new_y < 0 || new_y > 7) continue;

      if (get_color(board[new_x][new_y]) == piece_color && board[new_x][new_y] != NONE) continue;
      out_board[new_x][new_y] = KING_MOVE;
    }
  }
  // Castling also requires the king to be on its home square.
  if (!can_castle[piece_color][1] || piece.x != 4 || (piece.y != 0 && piece.y != 7)) return;
  if (is_in_check(board, piece)) return;

  // Queenside Castling: b/c/d-file must be empty, c- and d-file not attacked.
  if (can_castle[piece_color][0] &&
      board[3][piece.y] == NONE && board[2][piece.y] == NONE && board[1][piece.y] == NONE &&
      !king_path_attacked(board, piece, 3, piece.y) &&
      !king_path_attacked(board, piece, 2, piece.y)) {
    out_board[2][piece.y] = QUEENSIDE_CASTLE;
  }
  // Kingside Castling: f/g-file must be empty, f- and g-file not attacked.
  if (can_castle[piece_color][2] &&
      board[5][piece.y] == NONE && board[6][piece.y] == NONE &&
      !king_path_attacked(board, piece, 5, piece.y) &&
      !king_path_attacked(board, piece, 6, piece.y)) {
    out_board[6][piece.y] = KINGSIDE_CASTLE;
  }
}

// Locates the king of the given color, or {-1, -1} when there is none.
SelectedPiece find_king(char board[8][8], bool color) {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (board[i][j] == W_KING + color * 6) return (SelectedPiece){i, j};
    }
  }
  return (SelectedPiece){-1, -1};
}

void valid_move_generate(char board[8][8], SelectedPiece piece, char last_move[2][2], bool can_castle[2][3], char out_board[8][8]) {
  char piece_raw = board[piece.x][piece.y];
  char piece_typed = piece_raw % 6;
  bool piece_color = get_color(piece_raw);
  if (piece_typed == W_PAWN) {
    pawn_moves_generate(board, piece_color, piece, last_move, out_board);
  }
  else if (piece_typed == W_KNIGHT) {
    knight_moves_generate(board, piece, piece_color, out_board);
  }
  else if (piece_typed == W_BISHOP) {
    ray_moves_generate(board, piece, W_BISHOP, piece_color, out_board);
  }
  else if (piece_typed == W_ROOK) {
    ray_moves_generate(board, piece, W_ROOK, piece_color, out_board);
  }
  else if (piece_typed == W_QUEEN) {
    ray_moves_generate(board, piece, W_BISHOP, piece_color, out_board);
    ray_moves_generate(board, piece, W_ROOK, piece_color, out_board);
  }
  else if (piece_typed == W_KING) {
    king_moves_generate(board, piece, piece_color, can_castle, out_board);
  }

  SelectedPiece king = find_king(board, piece_color);
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (out_board[i][j]) {
        char prev_piece_on_new = board[i][j];
        board[i][j] = piece_raw;
        board[piece.x][piece.y] = NONE;

        // An en passant capture also removes a pawn from another square,
        // which matters when it was blocking a check.
        char prev_ep_victim = NONE;
        if (out_board[i][j] == EN_PASSANT) {
          prev_ep_victim = board[i][piece.y];
          board[i][piece.y] = NONE;
        }

        bool invalid;
        if (piece_typed == W_KING) {
          invalid = is_in_check(board, (SelectedPiece){i, j});
        }
        else {
          invalid = (king.x == -1) || is_in_check(board, king);
        }

        if (prev_ep_victim != NONE) board[i][piece.y] = prev_ep_victim;
        board[i][j] = prev_piece_on_new;
        board[piece.x][piece.y] = piece_raw;

        if (invalid) out_board[i][j] = ILLEGAL;
      }
    }
  }
}

// True when the given side has at least one legal move.
bool side_has_moves(char board[8][8], bool color, char last_move[2][2], bool can_castle[2][3]) {
  char moves[8][8];
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (board[i][j] == NONE || get_color(board[i][j]) != color) continue;
      memset(moves, 0, sizeof(moves));
      valid_move_generate(board, (SelectedPiece){i, j}, last_move, can_castle, moves);
      for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
          if (moves[x][y] != ILLEGAL) return true;
        }
      }
    }
  }
  return false;
}
