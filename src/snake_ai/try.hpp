#include <iostream>
#include <vector>
#include <deque>
#include <random>
#include <cmath>
#include <algorithm>
#include <torch/torch.h>

const int MAX_MEMORY = 100000;
const int BATCH_SIZE = 1000;
const float LR = 0.001;


struct Point {
    int x;
    int y;
    Point(int x, int y) : x(x), y(y) {}
};

enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

class SnakeGameAI {
public:
    int width;
    int height;
    int score;
    int food_x;
    int food_y;
    int snake_size;
    int direction;
    int cycles = 0;
    bool isRestarted = false;
    std::vector<Point> snake;
    std::vector<std::vector<int>> grid;
    bool game_over;
    std::vector<float> reward = {0};

    SnakeGameAI() {
        width = 40;
        height = 40;
        score = 0;
        snake_size = 4;
        direction = Direction::RIGHT;
        game_over = false;
        snake = {Point(6, 5), Point(5, 5), Point(4, 5), Point(3, 5)};
        grid = std::vector<std::vector<int>>(width, std::vector<int>(height, 0));
        grid[6][5] = 1;
        grid[5][5] = 1;
        grid[4][5] = 1;
        grid[3][5] = 1;
        food_x = rand() % width;
        food_y = rand() % height;
        grid[food_x][food_y] = -1;
    }

    void reset();
    bool is_collision(Point p);
    void move_snake();
    std::tuple<std::vector<float>, std::vector<float>, float> play_step(std::vector<float> move);
};

struct Linear_QNetImpl : torch::nn::Module {
    torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr}, fc4{nullptr};

    Linear_QNetImpl(int64_t n_inputs, int64_t n_hidden, int64_t n_outputs)
            : fc1(register_module("fc1", torch::nn::Linear(n_inputs, n_hidden))),
              fc2(register_module("fc2", torch::nn::Linear(n_hidden, n_hidden))),
              fc3(register_module("fc3", torch::nn::Linear(n_hidden, n_hidden))),
              fc4(register_module("fc4", torch::nn::Linear(n_hidden, n_outputs))){
        reset();
    }

    torch::Tensor forward(torch::Tensor x);
    void reset();
    void save();
    void load();
};

struct QTrainer {
    Linear_QNetImpl model;
    torch::optim::Adam optimizer;
    float gamma;

    QTrainer(Linear_QNetImpl model, float lr, float gamma)
            : model{model}, optimizer(torch::optim::Adam(model.parameters(), torch::optim::AdamOptions(lr))), gamma{gamma} {}
    
    void train_step(std::vector<float> state, std::vector<float> action, std::vector<float> reward, std::vector<float> next_state, std::vector<float> done);
};

class Agent {
public:
    int n_games = 0;
    float epsilon = 0;
    float gamma = 0.9f;
    Linear_QNetImpl model = Linear_QNetImpl(11, 512, 3);
    QTrainer trainer = QTrainer(model, LR, gamma);
    std::deque<std::vector<std::vector<float>>> memory;

    std::vector<float> get_state(SnakeGameAI game);
    float getStraightDangerDistance(SnakeGameAI game, Point head, bool dir_l, bool dir_r, bool dir_u, bool dir_d);
    float getLeftDangerDistance(SnakeGameAI game, Point head, bool dir_l, bool dir_r, bool dir_u, bool dir_d);
    float getRightDangerDistance(SnakeGameAI game, Point head, bool dir_l, bool dir_r, bool dir_u, bool dir_d);
    float getLeftFoodDistance(SnakeGameAI game, Point head, Point food);
    float getRightFoodDistance(SnakeGameAI game, Point head, Point food);
    float getUpFoodDistance(SnakeGameAI game, Point head, Point food);
    float getDownFoodDistance(SnakeGameAI game, Point head, Point food);
    void remember(std::vector<float> state, std::vector<float> action, std::vector<float> reward, std::vector<float> next_state, std::vector<float> done);
    void train_long_memory();
    void train_short_memory(std::vector<float> state, std::vector<float> action, std::vector<float> reward, std::vector<float> next_state, std::vector<float> done);
    std::vector<float> get_action(std::vector<float> state);
};