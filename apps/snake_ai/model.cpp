// #include <model.hpp>

// LinearQNet::LinearQNet(int inputSize, int hiddenSize, int outputSize){
//     linear1 = torch::nn::Linear(inputSize, hiddenSize);
//     linear2 = torch::nn::Linear(hiddenSize, outputSize);
// }

// LinearQNet::~LinearQNet(){}

// torch::Tensor LinearQNet::forward(torch::Tensor x){
//     x = torch::nn::functional::relu(linear1->forward(x));
//     x = linear2->forward(x);
//     return x;
// }

// void LinearQNet::save(std::string fileName){
//     std::string modelFolderPath = "./model";
//     if (!std::filesystem::exists(modelFolderPath))
//         std::filesystem::create_directories(modelFolderPath);

//     fileName = modelFolderPath + "/" + fileName;
//     torch::save(linear2, fileName);
// }

// QTrainer::QTrainer(LinearQNet model, double lr, float gamma) :
//     model{model}, lr{lr}, gamma{gamma}, optimizer{torch::optim::Adam(model.parameters(), lr)}, criterion{torch::nn::MSELoss()}{}

// QTrainer::~QTrainer(){}

// void QTrainer::trainStep(std::vector<int> state, std::vector<int> action, std::vector<int> reward, std::vector<int> nextState, std::vector<int> done){ 
//     torch::Tensor stateTensor = torch::tensor(state);
//     torch::Tensor actionTensor = torch::tensor(action);
//     torch::Tensor rewardTensor = torch::tensor(reward);
//     torch::Tensor nextStateTensor = torch::tensor(nextState);

//     stateTensor = torch::unsqueeze(stateTensor, 0);
//     actionTensor = torch::unsqueeze(actionTensor, 0);
//     rewardTensor = torch::unsqueeze(rewardTensor, 0);
//     nextStateTensor = torch::unsqueeze(nextStateTensor, 0);
//     std::vector<int> doneVector{done,};

//     torch::Tensor pred = model.forward(stateTensor);
//     torch::Tensor target = pred.clone();
//     for (int i = 0; i < doneVector.size(); i++)
//     {
//         torch::Tensor QNew = rewardTensor[i];
//         if (!doneVector[i])
//             QNew = rewardTensor[i] + gamma * torch::max(model.forward(nextStateTensor));

//         target[i, torch::argmax(actionTensor[i]).item()] = QNew;
//     }
//     optimizer.zero_grad();
//     torch::Tensor loss = criterion(target, pred);
//     loss.backward();
//     optimizer.step();
// }
