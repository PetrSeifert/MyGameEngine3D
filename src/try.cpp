#include <try.hpp>

void SnakeGameAI::reset() {
    score = 0;
    snake_size = 4;
    isRestarted = true;
    direction = Direction::RIGHT;
    game_over = false;
    snake = {Point(6, 5), Point(5, 5), Point(4, 5), Point(3, 5)};
    grid = std::vector<std::vector<int>>(width, std::vector<int>(height, 0));
    grid[6][5] = 1;
    grid[5][5] = 1;
    grid[4][5] = 1;
    grid[3][5] = 1;
    static std::random_device rd;  
    static std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distrib(0, width - 1);
    food_x = distrib(gen);
    food_y = distrib(gen);
    grid[food_x][food_y] = -1;
}

bool SnakeGameAI::is_collision(Point p) {
    if ( p.x < 0 || p.y < 0 || p.x > width - 1 || p.y > height - 1)
        return true;
    return grid[p.x][p.y] == 1;
}

void SnakeGameAI::move_snake() {
    cycles++;
    Point head = snake[0];
    Point new_head = Point(0, 0);
    if (direction == Direction::UP) {
        new_head = Point(head.x, head.y - 1);
    } else if (direction == Direction::DOWN) {
        new_head = Point(head.x, head.y + 1);
    } else if (direction == Direction::LEFT) {
        new_head = Point(head.x - 1, head.y);
    } else if (direction == Direction::RIGHT) {
        new_head = Point(head.x + 1, head.y);
    }
    reward = {-1.f / (width * height)};
    if (new_head.x < 0 || new_head.x >= width || new_head.y < 0 || new_head.y >= height) {
        game_over = true;
        reward = {-1.f};
        cycles = 0;
        return;
    }

    if (grid[new_head.x][new_head.y] == 1) {
        game_over = true;
        reward = {-1.f};
        cycles = 0;
        return;
    }

    if (cycles == width * height)
    {
        game_over = true;
        reward = {-1.f};
        cycles = 0;
        return;
    }

    if (new_head.x == food_x && new_head.y == food_y) {
        score += 1;
        reward = {1.f};
        snake_size += 1;
        static std::random_device rd;  
        static std::mt19937 gen(rd()); 
        std::uniform_int_distribution<> distrib(0, width - 1);
        food_x = distrib(gen);
        food_y = distrib(gen);
        while (grid[food_x][food_y] != 0) {
            food_x = distrib(gen);
            food_y = distrib(gen);
        }
        grid[food_x][food_y] = -1;
        cycles = 0;
    } else {
        Point tail = snake.back();
        grid[tail.x][tail.y] = 0;
        snake.pop_back();
    }

    grid[new_head.x][new_head.y] = 1;
    snake.insert(snake.begin(), new_head);
}

std::tuple<std::vector<float>, std::vector<float>, float> SnakeGameAI::play_step(std::vector<float> move) {
    if (move[0] == 1) {
        direction -= 1;
    } else if (move[2] == 1) {
        direction += 1;
    }

    if (direction < 0) direction = 3;
    else if (direction > 3) direction = 0;

    move_snake();
    return make_tuple(reward, std::vector<float>{(float)game_over}, score);
}

torch::Tensor Linear_QNetImpl::forward(torch::Tensor x) {
    x = torch::relu(fc1->forward(x));
    x = torch::relu(fc2->forward(x));
    x = torch::relu(fc3->forward(x));
    x = fc4->forward(x);
    return x;
}

void Linear_QNetImpl::reset() {
    torch::nn::init::xavier_uniform_(fc1->weight);
    torch::nn::init::xavier_uniform_(fc2->weight);
    torch::nn::init::xavier_uniform_(fc3->weight);
    torch::nn::init::xavier_uniform_(fc4->weight);
}

void Linear_QNetImpl::save() {
    torch::save(fc1, "fc1.pt");
    torch::save(fc2, "fc2.pt");
    torch::save(fc3, "fc3.pt");
    torch::save(fc4, "fc4.pt");
}

void Linear_QNetImpl::load(){
    torch::load(fc1, "fc1.pt");
    torch::load(fc2, "fc2.pt");
    torch::load(fc3, "fc3.pt");
    torch::load(fc4, "fc4.pt");
    fc1->eval();
    fc2->eval();
    fc3->eval();
    fc4->eval();
    eval();
}

void QTrainer::train_step(std::vector<float> state, std::vector<float> action, std::vector<float> reward, std::vector<float> next_state, std::vector<float> done) {
    torch::Tensor state_tensor = torch::from_blob(state.data(), {(int64_t)state.size()}, torch::kFloat32);
    torch::Tensor action_tensor = torch::from_blob(action.data(), {(int64_t)action.size()}, torch::kFloat32);
    torch::Tensor reward_tensor = torch::from_blob(reward.data(), {(int64_t)reward.size()}, torch::kFloat32);
    torch::Tensor next_state_tensor = torch::from_blob(next_state.data(), {(int64_t)next_state.size()}, torch::kFloat32);

    torch::Tensor q_value = model.forward(state_tensor);

    torch::Tensor next_q_value = model.forward(next_state_tensor);
    next_q_value = torch::max(next_q_value);

    torch::Tensor target = q_value.clone();
    torch::Tensor expected_q_value = reward_tensor + (gamma * next_q_value * (1 - done[0]));
    
    target[torch::argmax(action_tensor).item()] = expected_q_value[0];

    optimizer.zero_grad();

    torch::Tensor loss = torch::nn::functional::mse_loss(q_value, target);


    loss.backward();
    optimizer.step();
}

std::vector<float> Agent::get_state(SnakeGameAI game) {
    Point head = game.snake[0];
    Point point_l = Point(head.x - 1, head.y);
    Point point_r = Point(head.x + 1, head.y);
    Point point_u = Point(head.x, head.y - 1);
    Point point_d = Point(head.x, head.y + 1);

    bool dir_l = game.direction == Direction::LEFT;
    bool dir_r = game.direction == Direction::RIGHT;
    bool dir_u = game.direction == Direction::UP;
    bool dir_d = game.direction == Direction::DOWN;

    std::vector<float> state = {
        
        // Danger straight
        getStraightDangerDistance(game, head, dir_l, dir_r, dir_u, dir_d),

        // Danger right
        getRightDangerDistance(game, head, dir_l, dir_r, dir_u, dir_d),

        // Danger left
        getLeftDangerDistance(game, head, dir_l, dir_r, dir_u, dir_d),

        // Move direction
        (float)dir_l,
        (float)dir_r,
        (float)dir_u,
        (float)dir_d,

        // Food distance
        getLeftFoodDistance(game, head, Point(game.food_x, game.food_y)),
        getRightFoodDistance(game, head, Point(game.food_x, game.food_y)),
        getUpFoodDistance(game, head, Point(game.food_x, game.food_y)),
        getDownFoodDistance(game, head, Point(game.food_x, game.food_y))
    };

    return state;
}


float Agent::getStraightDangerDistance(SnakeGameAI game, Point head, bool dir_l, bool dir_r, bool dir_u, bool dir_d){

    if (dir_u) 
        for (int i = head.y - 1; i >= -1; i--) 
            if (game.is_collision(Point(head.x, i)))
                return (float)(head.y - i) / game.height;

    if (dir_d) 
        for (int i = head.y + 1; i <= game.height; i++) 
            if (game.is_collision(Point(head.x, i)))
                return (float)(i - head.y) / game.height;

    if (dir_l) 
        for (int i = head.x - 1; i >= -1; i--) 
            if (game.is_collision(Point(i, head.y)))
                return (float)(head.x - i) / game.width;

    for (int i = head.x + 1; i <= game.width; i++) 
        if (game.is_collision(Point(i, head.y)))
            return (float)(i - head.x) / game.width;
}

float Agent::getLeftDangerDistance(SnakeGameAI game, Point head, bool dir_l, bool dir_r, bool dir_u, bool dir_d){

    if (dir_u) 
        for (int i = head.x - 1; i >= -1; i--) 
            if (game.is_collision(Point(i, head.y)))
                return (float)(head.x - i) / game.width;

    if (dir_d) 
        for (int i = head.x + 1; i <= game.width; i++) 
            if (game.is_collision(Point(i, head.y)))
                return (float)(i - head.x) / game.width;

    if (dir_l) 
        for (int i = head.y + 1; i <= game.height; i++) 
            if (game.is_collision(Point(head.x, i)))
                return (float)(i - head.y) / game.height;

    for (int i = head.y - 1; i >= -1; i--) 
        if (game.is_collision(Point(head.x, i)))
            return (float)(head.y - i) / game.height;
}

float Agent::getRightDangerDistance(SnakeGameAI game, Point head, bool dir_l, bool dir_r, bool dir_u, bool dir_d){

    if (dir_u) 
        for (int i = head.x + 1; i <= game.width; i++) 
            if (game.is_collision(Point(i, head.y)))
                return (float)(i - head.x) / game.width;

    if (dir_d) 
        for (int i = head.x - 1; i >= -1; i--) 
            if (game.is_collision(Point(i, head.y)))
                return (float)(head.x - i) / game.width;

    if (dir_l) 
        for (int i = head.y - 1; i >= -1; i--) 
            if (game.is_collision(Point(head.x, i)))
                return (float)(head.y - i) / game.height;

    for (int i = head.y + 1; i <= game.height; i++) 
        if (game.is_collision(Point(head.x, i)))
            return (float)(i - head.y) / game.height;
}

float Agent::getLeftFoodDistance(SnakeGameAI game, Point head, Point food){
    float foodDistance = (float)(head.x - food.x) / (game.width - 1);
    if  (foodDistance > 0)
        return foodDistance;
    return 0;
}

float Agent::getRightFoodDistance(SnakeGameAI game, Point head, Point food){
    float foodDistance = (float)(food.x - head.x) / (game.width - 1);
    if  (foodDistance > 0)
        return foodDistance;
    return 0;
}

float Agent::getUpFoodDistance(SnakeGameAI game, Point head, Point food){
    float foodDistance = (float)(head.y - food.y) / (game.height - 1);
    if  (foodDistance > 0)
        return foodDistance;
    return 0;
}

float Agent::getDownFoodDistance(SnakeGameAI game, Point head, Point food){
    float foodDistance = (float)(food.y - head.y) / (game.height - 1);
    if  (foodDistance > 0)
        return foodDistance;
    return 0;
}


void Agent::remember(std::vector<float> state, std::vector<float> action, std::vector<float> reward, std::vector<float> next_state, std::vector<float> done) {
    memory.push_back(std::vector<std::vector<float>>{state, action, reward, next_state, done});
    if (memory.size() > MAX_MEMORY) {
        memory.pop_front();
    }
}

void Agent::train_long_memory() {
    std::deque<std::vector<std::vector<float>>> mini_sample;
    if (memory.size() > BATCH_SIZE) {
        std::sample(memory.begin(), memory.end(), std::back_inserter(mini_sample),
            BATCH_SIZE, std::mt19937{std::random_device{}()});
    } else {
        std::copy(memory.begin(), memory.end(), back_inserter(mini_sample));
    }

    std::vector<std::vector<float>> states;
    std::vector<std::vector<float>> actions;
    std::vector<std::vector<float>> rewards;
    std::vector<std::vector<float>> next_states;
    std::vector<std::vector<float>> dones;
    for (auto data : mini_sample){
        states.push_back(data[0]);
        actions.push_back(data[1]);
        rewards.push_back(data[2]);
        next_states.push_back(data[3]);
        dones.push_back(data[4]);
    }
    // for (int i = 0; i < mini_sample.size(); i++){
    //     states.push_back(mini_sample[i][0]);
    //     actions.push_back(mini_sample[i][1]);
    //     rewards.push_back(mini_sample[i][2]);
    //     next_states.push_back(mini_sample[i][3]);
    //     dones.push_back(mini_sample[i][4]);
    // }
    for (int i = 0; i < states.size(); i++)
        trainer.train_step(states[i], actions[i], rewards[i], next_states[i], dones[i]);
}

void Agent::train_short_memory(std::vector<float> state, std::vector<float> action, std::vector<float> reward, std::vector<float> next_state, std::vector<float> done) {
    trainer.train_step({state}, {action}, {reward}, {next_state}, {done});
}

std::vector<float> Agent::get_action(std::vector<float> state) {
    static std::random_device rd;  
    static std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distrib1(0, 200);
    // random moves: tradeoff exploration / exploitation
    epsilon = 0 - n_games;
    std::vector<float> final_move = {0, 0, 0};
    if (distrib1(gen) < epsilon) {
        std::uniform_int_distribution<> distrib2(0, 2);
        int move = distrib2(gen);
        final_move[move] = 1;
    } else {
        torch::Tensor state_tensor = torch::from_blob(state.data(), {(int64_t)state.size()}, torch::kFloat32);
        torch::Tensor prediction = model.forward(state_tensor);
        int move = torch::argmax(prediction).item().toInt();
        final_move[move] = 1;
    }

    return final_move;
}