#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib> // rand, srand
#include <ctime>   // time

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
struct Point
{
    int x, y;
};

//----------------------------------
// Hướng di chuyển
//----------------------------------
enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

//----------------------------------
// Hỗ trợ nhập bàn phím (Windows/Linux)
//----------------------------------
#if !(defined(_WIN32) || defined(_WIN64))
int kbhit()
{
    struct timeval tv = { 0, 0 };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);
}

int getch_noblock()
{
    char c = 0;
    if (kbhit())
    {
        if (read(STDIN_FILENO, &c, 1) == 1)
            return (int)c;
    }
    return 0;
}

struct TermGuard
{
    termios oldt;
    TermGuard()
    {
        tcgetattr(STDIN_FILENO, &oldt);
        termios newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    ~TermGuard()
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
};
#endif

//----------------------------------
// Hàm vẽ rắn (đã bỏ khung và thức ăn)
//----------------------------------
void draw(const vector<Point>& snake, int width, int height, int score)
{
    vector<string> screen(height, string(width, ' '));

    // rắn
    bool head = true;
    for (auto& p : snake)
    {
        if (p.x >= 0 && p.x < width && p.y >= 0 && p.y < height) // Điều chỉnh để vẽ cả khi chạm biên
            screen[p.y][p.x] = head ? 'O' : 'o', head = false;
    }

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    for (auto& row : screen)
        cout << row << '\n';
    cout << "Score: " << score << " | ↑/↓/←/→ or W/A/S/D to Control | Q to Exit\n";
}

//----------------------------------
// Hàm di chuyển rắn
//----------------------------------
void moveSnake(vector<Point>& snake, Direction dir, int width, int height, bool grow)
{
    Point head = snake.front();
    Point newHead = head;

    switch (dir)
    {
    case UP:
        newHead.y--;
        break;
    case DOWN:
        newHead.y++;
        break;
    case LEFT:
        newHead.x--;
        break;
    case RIGHT:
        newHead.x++;
        break;
    }

    // vòng lại (thay vì 0 và width-1, ta dùng -1 và width/height)
    if (newHead.x < 0)
        newHead.x = width - 1;
    if (newHead.x >= width)
        newHead.x = 0;
    if (newHead.y < 0)
        newHead.y = height - 1;
    if (newHead.y >= height)
        newHead.y = 0;

    snake.insert(snake.begin(), newHead);
    if (!grow)
        snake.pop_back(); // nếu không ăn thì bỏ đuôi
}

//----------------------------------
// Chương trình chính
//----------------------------------
int main()
{
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

  
    while (running)
    {
        // xử lý phím
#if defined(_WIN32) || defined(_WIN64)
        if (_kbhit())
        {
            int c = _getch();
            if (c == 0 || c == 224)
            {
                int ext = _getch();
                if (ext == 72 && dir != DOWN)
                    dir = UP;
                else if (ext == 80 && dir != UP)
                    dir = DOWN;
                else if (ext == 75 && dir != RIGHT)
                    dir = LEFT;
                else if (ext == 77 && dir != LEFT)
                    dir = RIGHT;
            }
            else
            {
                char ch = (char)c;
                if ((ch == 'w' || ch == 'W') && dir != DOWN)
                    dir = UP;
                else if ((ch == 's' || ch == 'S') && dir != UP)
                    dir = DOWN;
                else if ((ch == 'a' || ch == 'A') && dir != RIGHT)
                    dir = LEFT;
                else if ((ch == 'd' || ch == 'D') && dir != LEFT)
                    dir = RIGHT;
                else if (ch == 'q' || ch == 'Q')
                    running = false;
            }
        }
#else
        int c = getch_noblock();
        if (c != 0)
        {
            if (c == 27)
            { // arrow keys
                if (kbhit())
                {
                    int c2 = getchar();
                    if (c2 == '[' && kbhit())
                    {
                        int c3 = getchar();
                        if (c3 == 'A' && dir != DOWN)
                            dir = UP;
                        else if (c3 == 'B' && dir != UP)
                            dir = DOWN;
                        else if (c3 == 'C' && dir != LEFT)
                            dir = RIGHT;
                        else if (c3 == 'D' && dir != RIGHT)
                            dir = LEFT;
                    }
                }
            }
            else
            {
                char ch = (char)c;
                if ((ch == 'w' || ch == 'W') && dir != DOWN)
                    dir = UP;
                else if ((ch == 's' || ch == 'S') && dir != UP)
                    dir = DOWN;
                else if ((ch == 'a' || ch == 'A') && dir != RIGHT)
                    dir = LEFT;
                else if ((ch == 'd' || ch == 'D') && dir != LEFT)
                    dir = RIGHT;
                else if (ch == 'q' || ch == 'Q')
                    running = false;
            }
        }
#endif

       
        bool grow = false; 

        moveSnake(snake, dir, width, height, grow);
        draw(snake, width, height, score); 
        Point head = snake.front();
        for (size_t i = 1; i < snake.size(); ++i) {
            if (head.x == snake[i].x && head.y == snake[i].y) {
                running = false;
                break;
            }
        }

        this_thread::sleep_for(chrono::milliseconds(150));
    }

    cout << "Game Over! Your score: " << score << endl;
    return 0;
}