#include "main.h"

bool paused = false;
int score = 0;

std::random_device rd;
std::mt19937 gen(rd());

std::uniform_int_distribution<int> distrib(0, 6);

bool piecePlayed = false;
bool softDrop = false;
bool canHold = true;
int currentPiece = distrib(gen) % 7;
int nextPiece = distrib(gen) % 7;
int storedPiece = -1;

Block blocks[20][10];

void draw() {
    std::string frameBuffer = "";

    frameBuffer += "\x1B[" + std::to_string(DEFAULT) + "m\n";
    frameBuffer += "\x1B[H\n";
    frameBuffer += "                       ┌─Next───┐\n";
    Piece next = pieces[nextPiece];
    for (int row = 0; row < 3; row ++) {
        frameBuffer += "                       │";
        for (int col = 0; col < 4; col ++) {
            if (next.blockMask[row][col] == 1) {
                frameBuffer += "\x1B[38;5;" + std::to_string(next.block.color) + "m██\x1B[0m";
            }
            else {
                frameBuffer += "  ";
            }
        }
        frameBuffer += "│\n";
    }
    frameBuffer += "                       └────────┘\n";

    frameBuffer += "                       ┌─Store──┐\n";
    Piece stored = pieces[storedPiece == -1 ? 0 : storedPiece];
    for (int row = 0; row < 3; row ++) {
        frameBuffer += "                       │";
        for (int col = 0; col < 4; col ++) {
            if (stored.blockMask[row][col] == 1 && storedPiece != -1) {
                frameBuffer += "\x1B[38;5;" + std::to_string(stored.block.color) + "m██\x1B[0m";
            }
            else {
                frameBuffer += "  ";
            }
        }
        frameBuffer += "│\n";
    }
    frameBuffer += "                       └────────┘\n";

    frameBuffer += "                       ┌─Score──┐\n";
    frameBuffer += "                       │        │";
    frameBuffer += "\x1B[" + std::to_string(countDigits(score) + 1) + "D" + std::to_string(score) + "\n";
    frameBuffer += "                       └────────┘\n";



    frameBuffer += "\x1B[H\n";
    frameBuffer += "┌─Tetris─────────────┐\n";
    for (int row = 0; row < 20; row ++) {
        frameBuffer += "│";
        for (int col = 0; col < 10; col ++) {
            Block block = blocks[row][col];

            if (block.cellOccupied) {
                frameBuffer += "\x1B[38;5;" + std::to_string(block.color) + "m██\x1B[0m";
            }
            else {
                frameBuffer += ". ";
            }
        }
        frameBuffer += "│\n";
    }
    frameBuffer += "└────────────────────┘\n";

    frameBuffer += "\x1B[" + std::to_string(BLACK) + "m\n";

    std::cout << frameBuffer << std::flush;
}

void rotatePiece(int rotate) {
    if (rotate != 0) {
        bool isLeft = (rotate == -1);
        bool canRotate = true;

        std::vector<std::pair<int, int>> activeCells;
        int sumRow = 0, sumCol = 0;

        for (int row = 0; row < 20; row++) {
            for (int col = 0; col < 10; col++) {
                if (blocks[row][col].cellOccupied && !blocks[row][col].isStationary) {
                    activeCells.push_back({row, col});
                    sumRow += row;
                    sumCol += col;
                }
            }
        }

        if (!activeCells.empty()) {
            int pivotRow = sumRow / (int)activeCells.size();
            int pivotCol = sumCol / (int)activeCells.size();

            std::vector<std::pair<int, int>> targetPositions;

            for (const auto& cell : activeCells) {
                int r = cell.first;
                int c = cell.second;
                int newRow, newCol;

                if (isLeft) {
                    newRow = pivotRow - (c - pivotCol);
                    newCol = pivotCol + (r - pivotRow);
                } else {
                    newRow = pivotRow + (c - pivotCol);
                    newCol = pivotCol - (r - pivotRow);
                }

                if (newRow < 0 || newRow >= 20 || newCol < 0 || newCol >= 10 || blocks[newRow][newCol].isStationary) {
                    canRotate = false;
                    break;
                }
                targetPositions.push_back({newRow, newCol});
            }

            if (canRotate) {
                Block activeBlock = blocks[activeCells[0].first][activeCells[0].second];

                for (const auto& cell : activeCells) {
                    blocks[cell.first][cell.second] = {};
                }
                for (const auto& target : targetPositions) {
                    blocks[target.first][target.second] = activeBlock;
                }
            }
        }

        rotate = 0;
    }
}

void movePiece(int strafe) {
    if (strafe != 0) {
        int dir = (strafe > 0) ? 1 : -1; 
        bool canStrafe = true;

        for (int row = 0; row < 20; row++) {
            for (int col = 0; col < 10; col++) {
                if (blocks[row][col].cellOccupied && !blocks[row][col].isStationary) {
                    int newCol = col + dir;
                    if (newCol < 0 || newCol >= 10 || blocks[row][newCol].isStationary) {
                        canStrafe = false;
                        break;
                    }
                }
            }
            if (!canStrafe) break;
        }

        if (canStrafe) {
            int startCol = (dir > 0) ? 9 : 0;
            int endCol = (dir > 0) ? -1 : 10;
            int step = (dir > 0) ? -1 : 1;

            for (int row = 0; row < 20; row++) {
                for (int col = startCol; col != endCol; col += step) {
                    if (blocks[row][col].cellOccupied && !blocks[row][col].isStationary) {
                        blocks[row][col + dir] = blocks[row][col];
                        blocks[row][col] = {};
                    }
                }
            }
        }
        strafe = 0;
    }
}

int clearLines() {
    int linesCleared = 0;

    for (int row = 19; row >= 0; row--) {
        bool isFullLine = true;

        for (int col = 0; col < 10; col++) {
            if (!blocks[row][col].cellOccupied || !blocks[row][col].isStationary) {
                isFullLine = false;
                break;
            }
        }

        if (isFullLine) {
            linesCleared++;

            for (int r = row; r > 0; r--) {
                for (int col = 0; col < 10; col++) {
                    blocks[r][col] = blocks[r - 1][col];
                }
            }

            for (int col = 0; col < 10; col++) {
                blocks[0][col] = {};
            }

            row++;
        }
    }

    if (linesCleared == 1) score += 100;
    else if (linesCleared == 2) score += 300;
    else if (linesCleared == 3) score += 500;
    else if (linesCleared == 4) score += 800;

    return linesCleared;
}

int update(bool triggerGravity) {
    if (triggerGravity) {
        bool moveSuccess = true;
        for (int row = 19; row >= 0; row--) {
            for (int col = 0; col < 10; col++) {
                if (blocks[row][col].cellOccupied && !blocks[row][col].isStationary) {
                    if (row == 19 || blocks[row + 1][col].isStationary) {
                        moveSuccess = false;
                        break;
                    }
                }
            }
            if (!moveSuccess) break;
        }

        if (moveSuccess) {
            for (int row = 18; row >= 0; row--) {
                for (int col = 0; col < 10; col++) {
                    if (blocks[row][col].cellOccupied && !blocks[row][col].isStationary) {
                        blocks[row + 1][col] = blocks[row][col];
                        blocks[row][col] = {};
                    }
                }
            }
        } else {
            for (int row = 0; row < 20; row++) {
                for (int col = 0; col < 10; col++) {
                    if (blocks[row][col].cellOccupied && !blocks[row][col].isStationary) {
                        blocks[row][col].isStationary = true;
                    }
                }
            }
            piecePlayed = false;
        }
    }

    clearLines();

    if (!piecePlayed) {
        currentPiece = nextPiece;
        // nextPiece = rand() % 7;
        nextPiece = distrib(gen) % 7;
        Piece piece = pieces[currentPiece];
        
        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 4; col++) {
                if (piece.blockMask[row][col] == 1) {
                    blocks[row][3 + col].cellOccupied = piece.block.cellOccupied;
                    blocks[row][3 + col].color = piece.block.color;
                    blocks[row][3 + col].isStationary = false;
                }
            }
        }
        piecePlayed = true;
        canHold = true;
    }

    for (int col = 3; col < 7; col++) {
        if (blocks[0][col].cellOccupied && blocks[0][col].isStationary) {
            return 1;
        }
    }

    return 0;
}

void hardDrop() {
    int dropDistance = 0;

    while (true) {
        bool canMoveDown = true;

        for (int row = 19; row >= 0; row--) {
            for (int col = 0; col < 10; col++) {
                if (blocks[row][col].cellOccupied && !blocks[row][col].isStationary) {
                    if (row == 19 || blocks[row + 1][col].isStationary) {
                        canMoveDown = false;
                        break;
                    }
                }
            }
            if (!canMoveDown) break;
        }

        if (canMoveDown) {
            for (int row = 18; row >= 0; row--) {
                for (int col = 0; col < 10; col++) {
                    if (blocks[row][col].cellOccupied && !blocks[row][col].isStationary) {
                        blocks[row + 1][col] = blocks[row][col];
                        blocks[row][col] = {};
                    }
                }
            }
            dropDistance++;
        } else {
            for (int row = 0; row < 20; row++) {
                for (int col = 0; col < 10; col++) {
                    if (blocks[row][col].cellOccupied && !blocks[row][col].isStationary) {
                        blocks[row][col].isStationary = true;
                    }
                }
            }
            piecePlayed = false;
            break;
        }
    }

    score += dropDistance * 2;
}

void storePiece() {
    if (!canHold) return;

    // 1. Clear active (non-stationary) blocks from grid before swapping
    for (int row = 0; row < 20; row++) {
        for (int col = 0; col < 10; col++) {
            if (blocks[row][col].cellOccupied && !blocks[row][col].isStationary) {
                blocks[row][col] = {};
            }
        }
    }

    // 2. Perform hold logic
    if (storedPiece == -1) {
        std::swap(currentPiece, storedPiece);
        // currentPiece = storedPiece;

        piecePlayed = false; // Triggers spawning nextPiece in update()
    } else {
        int tmp = currentPiece;
        currentPiece = storedPiece;
        storedPiece = tmp; // Assign storedPiece to the original piece saved in tmp

        // Spawn held piece at top center
        Piece piece = pieces[currentPiece];
        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 4; col++) {
                if (piece.blockMask[row][col] == 1) {
                    blocks[row][3 + col].cellOccupied = piece.block.cellOccupied;
                    blocks[row][3 + col].color = piece.block.color;
                    blocks[row][3 + col].isStationary = false;
                }
            }
        }
    }

    canHold = false; // Lock hold function until piece places
}

int game() {
    SetConsoleOutputCP(CP_UTF8);
    std::cout << "\x1B[?25l" << std::flush;

    for (int row = 0; row < 20; row++) {
        for (int col = 0; col < 10; col++) {
            blocks[row][col] = {};
        }
    }

    const auto frameDuration = std::chrono::milliseconds(16);
    const auto gravityDuration = std::chrono::milliseconds(500);
    auto lastGravityTime = std::chrono::steady_clock::now();

    while (true) {
        auto frameStart = std::chrono::steady_clock::now();
        
        softDrop = false;

        if (_kbhit()) {
            int ch = _getch();
            
            if (ch == 27) paused = !paused;

            if (!paused) {
                if (ch == ' ') hardDrop();
                if (ch == 'C' || ch == 'c') storePiece();
                if (ch == 'Z' || ch == 'z') rotatePiece(-1);
                if (ch == 'X' || ch == 'x') rotatePiece(1);
    
                if (ch == 224 || ch == 0) {
                    ch = _getch();
                    if (ch == 'K') movePiece(-1);
                    if (ch == 'M') movePiece(1);
                    if (ch == 'P') softDrop = true;
                }
            }
        }

        bool triggerGravity = false;
        if (frameStart - lastGravityTime >= gravityDuration) {
            triggerGravity = true;
            lastGravityTime = frameStart;
        }

        if (update((triggerGravity || softDrop) && !paused) == 1) break;

        if (!paused) {
            draw();
        }
        else {
            std::cout << "\x1B[" << RED + BACKGROUND << ";" << BRIGHT_WHITE << "m\x1B[12;8HPaused!";
        }

        auto frameTime = std::chrono::steady_clock::now() - frameStart;
        if (frameTime < frameDuration) {
            std::this_thread::sleep_for(frameDuration - frameTime);
        }
    }

    std::cout << "\x1B[?25h" << std::flush;

    std::cout << "\x1B[" << DEFAULT << "mYou Lost!\nFinal Score: " << score << std::endl;

    return 0;
}

void audio() {
    sound *s = new sound(2);

    Note notes[29] = { // 29 notes first bar
        {0, 659, 429}, {1, 493, 429},

        {0, 659, 429}
    };

    for (int i = 0; i < 17; i ++) {
        s->setChannel(0, notes[i].freq, notes[i].dur, notes[i].vol);
    
        s->flush();
    }
}

int main() {
    return game();

    // std::thread([]() {
    //     audio();
    // }).detach();

    // Beep(440, 1000);

    std::cin.get();

    return 0;
}