#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>   // rand, srand
#include <ctime>     // time

#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#endif

using namespace std;

//----------------------------------
// Cấu trúc tọa độ
//----------------------------------
struct Point {
    int x, y;
};

//----------------------------------
// Hướng di chuyển
//----------------------------------
enum Direction { UP, DOWN, LEFT, RIGHT };

//----------------------------------
// Hỗ trợ nhập bàn phím (Windows/Linux)
//----------------------------------
#if !(defined(_WIN32) || defined(_WIN64))
int kbhit() {
    struct timeval tv = { 0, 0 };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);
}

int getch_noblock() {
    char c = 0;
    if (kbhit()) {
        if (read(STDIN_FILENO, &c, 1) == 1) return (int)c;
    }
    return 0;
}

struct TermGuard {
    termios oldt;
    TermGuard() {
        tcgetattr(STDIN_FILENO, &oldt);
        termios newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    ~TermGuard() {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
};
#endif

//----------------------------------
// Hàm tạo thức ăn ngẫu nhiên
//----------------------------------
Point randomFood(int width, int height, const vector<Point>& snake) {
    Point food;
    bool valid = false;
    while (!valid) {
        food.x = rand() % (width - 2) + 1;
        food.y = rand() % (height - 2) + 1;
        valid = true;
        // tránh spawn lên thân rắn
        for (auto& s : snake)
            if (s.x == food.x && s.y == food.y)
                valid = false;
    }
    return food;
}

//----------------------------------
// Hàm vẽ khung + rắn + thức ăn
//----------------------------------
void draw(const vector<Point>& snake, Point food, int width, int height, int score) {
    vector<string> screen(height, string(width, ' '));

    for (int x = 0; x < width; ++x) {
        screen[0][x] = '#';
        screen[height - 1][x] = '#';
    }
    for (int y = 0; y < height; ++y) {
        screen[y][0] = '#';
        screen[y][width - 1] = '#';
    }

    // rắn
    bool head = true;
    for (auto& p : snake) {
        if (p.x > 0 && p.x < width - 1 && p.y > 0 && p.y < height - 1)
            screen[p.y][p.x] = head ? 'O' : 'o', head = false;
    }

    // thức ăn
    screen[food.y][food.x] = '*';

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    for (auto& row : screen) cout << row << '\n';
    cout << "Score: " << score << " | ↑/↓/←/→ or W/A/S/D to Control | Q to Exit\n";
}

//----------------------------------
// Hàm di chuyển rắn
//----------------------------------
void moveSnake(vector<Point>& snake, Direction dir, int width, int height, bool grow) {
    Point head = snake.front();
    Point newHead = head;

    switch (dir) {
    case UP:    newHead.y--; break;
    case DOWN:  newHead.y++; break;
    case LEFT:  newHead.x--; break;
    case RIGHT: newHead.x++; break;
    }

    // vòng lại
    if (newHead.x <= 0) newHead.x = width - 2;
    if (newHead.x >= width - 1) newHead.x = 1;
    if (newHead.y <= 0) newHead.y = height - 2;
    if (newHead.y >= height - 1) newHead.y = 1;

    snake.insert(snake.begin(), newHead);
    if (!grow)
        snake.pop_back(); // nếu không ăn thì bỏ đuôi
}

//----------------------------------
// Chương trình chính
//----------------------------------
int main() {
    srand(time(nullptr));

    int width = 40, height = 15;
    vector<Point> snake;
    int startX = width / 2;
    int startY = height / 2;

    for (int i = 0; i < 5; ++i)
        snake.push_back({ startX - i, startY });

    Direction dir = RIGHT;
    bool running = true;
    int score = 0;

#if !(defined(_WIN32) || defined(_WIN64))
    TermGuard tg;
#endif

    // tạo mục tiêu đầu tiên
    Point food = randomFood(width, height, snake);

    while (running) {
        // xử lý phím
#if defined(_WIN32) || defined(_WIN64)
        if (_kbhit()) {
            int c = _getch();
            if (c == 0 || c == 224) {
                int ext = _getch();
                if (ext == 72) dir = UP;
                else if (ext == 80) dir = DOWN;
                else if (ext == 75) dir = LEFT;
                else if (ext == 77) dir = RIGHT;
            }
            else {
                char ch = (char)c;
                if (ch == 'w' || ch == 'W') dir = UP;
                else if (ch == 's' || ch == 'S') dir = DOWN;
                else if (ch == 'a' || ch == 'A') dir = LEFT;
                else if (ch == 'd' || ch == 'D') dir = RIGHT;
                else if (ch == 'q' || ch == 'Q') running = false;
            }
        }
#else
        int c = getch_noblock();
        if (c != 0) {
            if (c == 27) { // arrow keys
                if (kbhit()) {
                    int c2 = getchar();
                    if (c2 == '[' && kbhit()) {
                        int c3 = getchar();
                        if (c3 == 'A') dir = UP;
                        else if (c3 == 'B') dir = DOWN;
                        else if (c3 == 'C') dir = RIGHT;
                        else if (c3 == 'D') dir = LEFT;
                    }
                }
            }
            else {
                char ch = (char)c;
                if (ch == 'w' || ch == 'W') dir = UP;
                else if (ch == 's' || ch == 'S') dir = DOWN;
                else if (ch == 'a' || ch == 'A') dir = LEFT;
                else if (ch == 'd' || ch == 'D') dir = RIGHT;
                else if (ch == 'q' || ch == 'Q') running = false;
            }
        }
#endif

        // kiểm tra ăn thức ăn
        bool grow = false;
        Point head = snake.front();
        if (head.x == food.x && head.y == food.y) {
            score++;
            grow = true;
            food = randomFood(width, height, snake);
        }

        moveSnake(snake, dir, width, height, grow);
        draw(snake, food, width, height, score);

        this_thread::sleep_for(chrono::milliseconds(150));
    }

    cout << "Game Over! Your score: " << score << endl;
    return 0;
}