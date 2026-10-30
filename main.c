#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chess.h"

#define SCREEN_WIDTH  600
#define SCREEN_HEIGHT 600

#define WHITE 0
#define BLACK 1

char game_board[8][8] = {
  {W_ROOK, W_PAWN, NONE, NONE, NONE, NONE, B_PAWN, B_ROOK}, // a-file
  {W_KNIGHT, W_PAWN, NONE, NONE, NONE, NONE, B_PAWN, B_KNIGHT}, // b-file
  {W_BISHOP, W_PAWN, NONE, NONE, NONE, NONE, B_PAWN, B_BISHOP}, // c-file
  {W_QUEEN, W_PAWN, NONE, NONE, NONE, NONE, B_PAWN, B_QUEEN}, // d-file
  {W_KING, W_PAWN, NONE, NONE, NONE, NONE, B_PAWN, B_KING}, // e-file
  {W_BISHOP, W_PAWN, NONE, NONE, NONE, NONE, B_PAWN, B_BISHOP}, // f-file
  {W_KNIGHT, W_PAWN, NONE, NONE, NONE, NONE, B_PAWN, B_KNIGHT}, // g-file
  {W_ROOK, W_PAWN, NONE, NONE, NONE, NONE, B_PAWN, B_ROOK} // h-file
};

char available_board[8][8];

// Piece choices shown in the promotion strip, in display order.
static const char promotion_choices[4] = {W_QUEEN, W_ROOK, W_BISHOP, W_KNIGHT};

// Called after every completed move: reports check / checkmate / stalemate
// in the window title. Returns true once the game is over.
static bool update_status(SDL_Window* window, bool color_turn, char board[8][8], char last_move[2][2], bool can_castle[2][3]) {
  SelectedPiece king = find_king(board, color_turn);
  bool checked = (king.x != -1) && is_in_check(board, king);
  char title[64];

  if (!side_has_moves(board, color_turn, last_move, can_castle)) {
    if (checked) {
      snprintf(title, sizeof(title), "Chess - Checkmate: %s wins",
               color_turn == WHITE ? "Black" : "White");
    } else {
      snprintf(title, sizeof(title), "Chess - Stalemate");
    }
    SDL_SetWindowTitle(window, title);
    return true;
  }

  snprintf(title, sizeof(title), "Chess - %s to move%s",
           color_turn == WHITE ? "White" : "Black",
           checked ? " (check!)" : "");
  SDL_SetWindowTitle(window, title);
  return false;
}

int main() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL Initialization Failed: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Window* window = SDL_CreateWindow("Chess - White to move", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_Log("Window Creation Failed: %s\n", SDL_GetError());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
  if (!renderer) {
    SDL_Log("Renderer Creation Failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_Texture* svgTextures[13] = {0};
  for (int i = 0; i < 12; i++) {
    char name[16];
    snprintf(name, sizeof(name), "assets/%d.svg", i);
    svgTextures[i] = IMG_LoadTexture(renderer, name);
    if (!svgTextures[i]) {
      SDL_Log("Failed to load %s: %s\n", name, SDL_GetError());
      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      SDL_Quit();
      return EXIT_FAILURE;
    }
  }

  SelectedPiece selected_piece = {-1, -1};

  char last_move[2][2] = {{-1, -1}, {-1, -1}};
  bool color_turn = WHITE;

  bool can_castle[2][3] = {{true, true, true}, {true, true, true}};

  // Promotion: once a pawn lands on the last rank the game waits for the
  // player to pick a piece before continuing.
  bool promoting = false;
  SelectedPiece promote_square = {0, 0};

  bool game_over = false;

  bool is_running = true;
  SDL_Event event;

  while (is_running) {
    // Recompute the layout every frame so resizing the window keeps working.
    int win_w, win_h;
    SDL_GetWindowSize(window, &win_w, &win_h);
    float board_size = (win_w < win_h) ? (float)win_w : (float)win_h;
    float square = board_size / 8;
    float origin_x = ((float)win_w - board_size) / 2;
    float origin_y = ((float)win_h - board_size) / 2;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          is_running = false;
          break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
          if (event.button.button == SDL_BUTTON_LEFT && !game_over) {
            // Map the click to a board square, ignoring clicks outside the board.
            float mx = event.button.x - origin_x;
            float my = event.button.y - origin_y;
            if (mx < 0 || mx >= board_size || my < 0 || my >= board_size) break;

            int square_x = (int)(mx / square);
            int square_y = 7 - (int)(my / square);
            if (square_x < 0) square_x = 0;
            if (square_x > 7) square_x = 7;
            if (square_y < 0) square_y = 0;
            if (square_y > 7) square_y = 7;

            if (promoting) {
              // The promotion strip: four squares centered on the board.
              float strip_w = square * 4;
              float strip_x = origin_x + (board_size - strip_w) / 2;
              float strip_y = origin_y + (board_size - square) / 2;
              if (event.button.x >= strip_x && event.button.x < strip_x + strip_w &&
                  event.button.y >= strip_y && event.button.y < strip_y + square) {
                int choice = (int)((event.button.x - strip_x) / square);
                if (choice < 0) choice = 0;
                if (choice > 3) choice = 3;
                game_board[promote_square.x][promote_square.y] =
                    promotion_choices[choice] + (color_turn == BLACK ? 6 : 0);
                promoting = false;
                color_turn = !color_turn;
                game_over = update_status(window, color_turn, game_board, last_move, can_castle);
              }
              break;
            }

            if (selected_piece.x == -1 && selected_piece.y == -1) {
              if (game_board[square_x][square_y] != NONE && get_color(game_board[square_x][square_y]) == color_turn) {
                selected_piece.x = square_x;
                selected_piece.y = square_y;
                valid_move_generate(game_board, selected_piece, last_move, can_castle, available_board);
              }
            } else {
              MoveTypes move = available_board[square_x][square_y];
              if (move != ILLEGAL) {
                char moved_piece = game_board[selected_piece.x][selected_piece.y];
                char captured_piece = game_board[square_x][square_y];
                int home_rank = (color_turn == WHITE) ? 0 : 7;

                game_board[square_x][square_y] = moved_piece;
                game_board[selected_piece.x][selected_piece.y] = NONE;

                if (move == EN_PASSANT) {
                  game_board[square_x][square_y - ((color_turn) ? -1 : 1)] = NONE;
                }
                else if (move == QUEENSIDE_CASTLE) {
                  game_board[0][square_y] = NONE;
                  game_board[3][square_y] = W_ROOK + color_turn * 6;
                } else if (move == KINGSIDE_CASTLE) {
                  game_board[7][square_y] = NONE;
                  game_board[5][square_y] = W_ROOK + color_turn * 6;
                }

                // Castling rights die with the king, with the rook moving off
                // its home square, or with that rook being captured.
                if (moved_piece == W_KING || moved_piece == B_KING) {
                  can_castle[color_turn][1] = false;
                }
                if (moved_piece == W_ROOK || moved_piece == B_ROOK) {
                  if (selected_piece.x == 0 && selected_piece.y == home_rank) can_castle[color_turn][0] = false;
                  if (selected_piece.x == 7 && selected_piece.y == home_rank) can_castle[color_turn][2] = false;
                }
                if (captured_piece == W_ROOK || captured_piece == B_ROOK) {
                  bool captured_color = get_color(captured_piece);
                  int captured_home = (captured_color == WHITE) ? 0 : 7;
                  if (square_x == 0 && square_y == captured_home) can_castle[captured_color][0] = false;
                  if (square_x == 7 && square_y == captured_home) can_castle[captured_color][2] = false;
                }

                last_move[0][0] = selected_piece.x;
                last_move[0][1] = selected_piece.y;
                last_move[1][0] = square_x;
                last_move[1][1] = square_y;

                selected_piece.x = -1;
                selected_piece.y = -1;
                memset(available_board, 0, sizeof(available_board));

                // A pawn reaching the last rank waits for the promotion choice
                // before the turn passes over.
                if ((moved_piece == W_PAWN || moved_piece == B_PAWN) &&
                    (square_y == 0 || square_y == 7)) {
                  promoting = true;
                  promote_square.x = square_x;
                  promote_square.y = square_y;
                } else {
                  color_turn = !color_turn;
                  game_over = update_status(window, color_turn, game_board, last_move, can_castle);
                }
              } else {
                selected_piece.x = -1;
                selected_piece.y = -1;
                memset(available_board, 0, sizeof(available_board));
              }
            }
          }
          break;
      }
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        SDL_FRect rect = { origin_x + i * square,
            origin_y + j * square,
            square,
            square};
        if ((i + j) % 2 == 1) {
          SDL_SetRenderDrawColor(renderer, 105, 146, 62, 255);
          SDL_RenderFillRect(renderer, &rect);
        }
        if (available_board[i][7 - j] != ILLEGAL) {
          SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
          SDL_RenderFillRect(renderer, &rect);
        } 
        if (game_board[i][7 - j] == NONE) continue;
        SDL_RenderTexture(renderer, svgTextures[(unsigned char)game_board[i][7 - j]], NULL, &rect);
      }
    }

    // Promotion strip: four candidate pieces centered on the board.
    if (promoting) {
      float strip_w = square * 4;
      SDL_FRect strip_rect = { origin_x + (board_size - strip_w) / 2,
          origin_y + (board_size - square) / 2,
          strip_w,
          square};
      SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
      SDL_RenderFillRect(renderer, &strip_rect);
      for (int k = 0; k < 4; k++) {
        SDL_FRect piece_rect = { strip_rect.x + k * square,
            strip_rect.y,
            square,
            square};
        SDL_RenderTexture(renderer,
            svgTextures[promotion_choices[k] + (color_turn == BLACK ? 6 : 0)],
            NULL, &piece_rect);
      }
    }

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}

