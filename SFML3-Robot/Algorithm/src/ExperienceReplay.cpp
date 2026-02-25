#include "ExperienceReplay.h"
#include <fstream>
#include <iostream>

ExperienceReplay::ExperienceReplay(size_t maxSize, size_t batchSize)
    : maxSize(maxSize), batchSize(batchSize), rng(std::random_device{}()) {
}

void ExperienceReplay::addExperience(const Experience& experience) {
    if (buffer.size() >= maxSize) {
        buffer.pop_front();
    }
    buffer.push_back(experience);
}

void ExperienceReplay::addExperience(const std::vector<double>& state, int action,
    double reward, const std::vector<double>& nextState,
    bool terminal) {
    addExperience(Experience(state, action, reward, nextState, terminal));
}

std::vector<Experience> ExperienceReplay::sampleBatch() {
    std::vector<Experience> batch;

    if (buffer.size() < batchSize) {
        return batch; // Return empty batch if not enough experiences
    }

    std::uniform_int_distribution<size_t> dist(0, buffer.size() - 1);

    for (size_t i = 0; i < batchSize; ++i) {
        size_t idx = dist(rng);
        batch.push_back(buffer[idx]);
    }

    return batch;
}

std::vector<Experience> ExperienceReplay::samplePrioritizedBatch() {
    // Simple implementation - uniform sampling
    // For true prioritized replay, you'd need to maintain priority scores
    return sampleBatch();
}

void ExperienceReplay::updatePriority(const Experience& experience, double priority) {
    // For prioritized replay - find and update priority
    // This is a simplified version
}

bool ExperienceReplay::saveToFile(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Save buffer size
    size_t bufferSize = buffer.size();
    file.write(reinterpret_cast<const char*>(&bufferSize), sizeof(bufferSize));

    // Save each experience
    for (const auto& exp : buffer) {
        // Save state
        size_t stateSize = exp.state.size();
        file.write(reinterpret_cast<const char*>(&stateSize), sizeof(stateSize));
        for (double val : exp.state) {
            file.write(reinterpret_cast<const char*>(&val), sizeof(val));
        }

        // Save action
        file.write(reinterpret_cast<const char*>(&exp.action), sizeof(exp.action));

        // Save reward
        file.write(reinterpret_cast<const char*>(&exp.reward), sizeof(exp.reward));

        // Save next state
        size_t nextStateSize = exp.nextState.size();
        file.write(reinterpret_cast<const char*>(&nextStateSize), sizeof(nextStateSize));
        for (double val : exp.nextState) {
            file.write(reinterpret_cast<const char*>(&val), sizeof(val));
        }

        // Save terminal flag
        file.write(reinterpret_cast<const char*>(&exp.terminal), sizeof(exp.terminal));
    }

    file.close();
    return true;
}

bool ExperienceReplay::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    buffer.clear();

    // Load buffer size
    size_t bufferSize;
    file.read(reinterpret_cast<char*>(&bufferSize), sizeof(bufferSize));

    // Load each experience
    for (size_t i = 0; i < bufferSize; ++i) {
        // Load state
        size_t stateSize;
        file.read(reinterpret_cast<char*>(&stateSize), sizeof(stateSize));
        std::vector<double> state(stateSize);
        for (size_t j = 0; j < stateSize; ++j) {
            file.read(reinterpret_cast<char*>(&state[j]), sizeof(double));
        }

        // Load action
        int action;
        file.read(reinterpret_cast<char*>(&action), sizeof(action));

        // Load reward
        double reward;
        file.read(reinterpret_cast<char*>(&reward), sizeof(reward));

        // Load next state
        size_t nextStateSize;
        file.read(reinterpret_cast<char*>(&nextStateSize), sizeof(nextStateSize));
        std::vector<double> nextState(nextStateSize);
        for (size_t j = 0; j < nextStateSize; ++j) {
            file.read(reinterpret_cast<char*>(&nextState[j]), sizeof(double));
        }

        // Load terminal flag
        bool terminal;
        file.read(reinterpret_cast<char*>(&terminal), sizeof(terminal));

        buffer.emplace_back(state, action, reward, nextState, terminal);
    }

    file.close();
    return true;
}